/**
 * @file nelo_transfer_function.c
 * @brief Gestione delle matrici di rischio locali e campionamento bus I2C privato schermato.
 * @note Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
 * @version 3.1-HARDENED
 */

#include "nrf.h"
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

// Parametri di calcolo in virgola fissa Q16.16: 65536 = 1.0
#define F_ONE 65536
#define EXP_SAMPLES 5 // Finestra media mobile ~5s a 1Hz

// Pesi hardware della funzione di trasferimento Q16.16
#define WEIGHT_ALPHA (131072) // 2.0: Conduttanza Cutanea G
#define WEIGHT_BETA (163840)  // 2.5: Variabilità Cardiaca V_hrv (Stress)
#define WEIGHT_GAMMA (196608) // 3.0: Gradiente Termico dT/dt
#define WEIGHT_OMEGA (65536)  // 1.0: Moltiplicatore Cross-Channel
#define BIAS_DELTA (196608)   // 3.0: Offset difensivo

// Soglia critica basata su centesimi di grado Celsius: 1.5°C = 150 unità grezze
#define CELSIUS_CENT_CRITICAL_DRIFT 150

// Stato interno per la baseline termica
typedef struct {
    int16_t history[EXP_SAMPLES]; // Supporta temperature in centesimi con segno
    int32_t sum;
    uint8_t index;
    bool primed;
} temp_baseline_t;

static temp_baseline_t temp_base = {0};

/**
 * @brief Approssimazione sigmoide hardened ad alta fedeltà in Q16.16
 * @details Sfrutta lo sviluppo simmetrico ed estende l'approssimazione al terzo ordine 
 * per eliminare il rumore di fondo parassita (D > 0.05) quando z è fortemente negativo.
 * @param x Input Q16.16 (argomento logistico Z)
 * @return Output Q16.16 rigorosamente confinato in [0, F_ONE]
 */
static int32_t nelo_fast_sigmoid(int32_t x) {
    if (x <= -5 * F_ONE) return 0;
    if (x >= 5 * F_ONE) return F_ONE;

    // Sfruttiamo la simmetria speculare: sigmoide(-x) = 1 - sigmoide(x)
    bool negative = false;
    if (x < 0) {
        x = -x;
        negative = true;
    }

    int64_t x64 = x;
    int32_t res;

    // Estensione al terzo ordine interpolato: e^x ≈ 1 + x + x^2/2 + x^3/6
    int64_t x2 = (x64 * x64) >> 16;
    int64_t x3 = (x2 * x64) >> 16;
    
    int32_t e_x = F_ONE + x + (int32_t)(x2 >> 1) + (int32_t)((x3 * 2731) >> 14); // 1/6 ≈ 2731/16384
    int64_t num = (int64_t)F_ONE * F_ONE;
    res = (int32_t)(num / (F_ONE + e_x));

    if (negative) {
        return res; 
    } else {
        return F_ONE - res;
    }
}

/**
 * @brief Calcola l'anomalia termica differenziale rispetto alla baseline mobile
 * @param t_skin_raw Temperatura cutanea dal TMP117 in centesimi di °C. Es: 3350 = 33.50°C
 * @return S_deltaT Q16.16 in [0, F_ONE]
 */
static int32_t process_thermal_anomaly(int16_t t_skin_raw) {
    if (!temp_base.primed) {
        for (int i = 0; i < EXP_SAMPLES; i++) {
            temp_base.history[i] = t_skin_raw;
        }
        temp_base.sum = (int32_t)t_skin_raw * EXP_SAMPLES;
        temp_base.index = 0;
        temp_base.primed = true;
        return 0;
    }

    int32_t current_baseline = temp_base.sum / EXP_SAMPLES;
    int32_t drift = current_baseline - (int32_t)t_skin_raw;

    // Aggiornamento FIFO del buffer circolare
    temp_base.sum -= temp_base.history[temp_base.index];
    temp_base.history[temp_base.index] = t_skin_raw;
    temp_base.sum += t_skin_raw;
    temp_base.index = (temp_base.index + 1) % EXP_SAMPLES;

    if (drift <= 0) return 0;

    // Normalizzazione basata sulla risoluzione in centesimi di grado
    int32_t s_delta_t = (drift * F_ONE) / CELSIUS_CENT_CRITICAL_DRIFT;
    if (s_delta_t > F_ONE) s_delta_t = F_ONE;

    return s_delta_t;
}

/**
 * @brief Calcolo deterministico dell'Indice di Danno D per SPEC-004 §3.1
 * @param s_g Punteggio conduttanza cutanea Q16.16 [0, F_ONE]
 * @param s_v Punteggio HRV calma Q16.16 [0, F_ONE]. 1.0 = calma, 0.0 = shock
 * @param t_skin Temperatura cutanea in centesimi di grado Celsius (es: 3350)
 * @return D in Q16.16 [0, F_ONE]. Soglia di innesco a 45875 = 0.7
 */
uint32_t nelo_calculate_d_index(int32_t s_g, int32_t s_v, int16_t t_skin) {
    // Clamping preventivo input
    if (s_g < 0) s_g = 0; else if (s_g > F_ONE) s_g = F_ONE;
    if (s_v < 0) s_v = 0; else if (s_v > F_ONE) s_v = F_ONE;

    // 1. Inversione semantica dell'HRV (1.0 calma -> 0.0 stress; 0.0 shock -> 1.0 stress)
    int32_t s_v_stress = F_ONE - s_v;

    // 2. Anomalia termica differenziale (Corretto errore di sintassi di battitura)
    int32_t s_delta_t = process_thermal_anomaly(t_skin);

    // 3. Somma lineare pesata
    int64_t linear_sum = ((int64_t)WEIGHT_ALPHA * s_g) +
                         ((int64_t)WEIGHT_BETA * s_v_stress) +
                         ((int64_t)WEIGHT_GAMMA * s_delta_t);
    linear_sum >>= 16;

    // 4. Termine di rinforzo cross-channel geometrico
    int64_t cross_product = ((int64_t)s_g * s_v_stress) >> 16;
    cross_product = (cross_product * s_delta_t) >> 16;
    int64_t reinforcement_term = (cross_product * WEIGHT_OMEGA) >> 16;

    // 5. Sintesi argomento logistico Z con clipping anti-overflow
    int64_t z64 = linear_sum + reinforcement_term - BIAS_DELTA;
    if (z64 > INT32_MAX) z64 = INT32_MAX;
    if (z64 < INT32_MIN) z64 = INT32_MIN;
    int32_t z = (int32_t)z64;

    // 6. Attivazione sigmoide hardened stabile alle code asintotiche
    return (uint32_t)nelo_fast_sigmoid(z);
}

/**
 * @brief Wrapper pubblico: calcola D e lo mappa a uint16_t per trasmissione payload AEAD
 * @param s_g Punteggio conduttanza Q16.16 [0, F_ONE]
 * @param s_v Punteggio HRV calma Q16.16 [0, F_ONE]
 * @param t_skin Temperatura cutanea nativa in centesimi di grado Celsius
 * @param[out] triggered Set a true se D >= 0.7, altrimenti false. Può essere NULL
 * @return D riscalato linearmente in formato Uint16 a piena dinamica [0, 65535]
 */
uint16_t nelo_get_damage_index_u16(int32_t s_g, int32_t s_v, int16_t t_skin, bool *triggered)
{
    uint32_t d_q16 = nelo_calculate_d_index(s_g, s_v, t_skin);

    // Mappatura a piena dinamica [0, 65535]: (D_q16 * 65535) / 65536
    // Sfrutta l'espansione temporanea a 64 bit per evitare overflow nel prodotto
    uint64_t d_scaled = ((uint64_t)d_q16 * 65535) >> 16;
    if (d_scaled > 65535) d_scaled = 65535;

    const uint32_t THRESHOLD_Q16 = (uint32_t)(45875); // 0.7 * F_ONE
    bool is_triggered = (d_q16 >= THRESHOLD_Q16);

    if (triggered) {
        *triggered = is_triggered;
    }

    // RIMOSSO IL RESET AUTOMATICO DESTRUTTIVO DI TEMP_BASE
    // La baseline deve persistere per monitorare la durata complessiva dello shock.

    return (uint16_t)d_scaled;
}

/**
 * @brief Reset esplicito della baseline termica. Usalo unicamente al boot o dopo wipe hardware.
 */
void nelo_reset_thermal_baseline(void)
{
    temp_base.primed = false;
    temp_base.sum = 0;
    temp_base.index = 0;
    for (int i = 0; i < EXP_SAMPLES; i++) {
        temp_base.history[i] = 0;
    }
}
