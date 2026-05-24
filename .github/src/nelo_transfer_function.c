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

// Risoluzione TMP117: 1.5°C equivalgono a 1.5 * 128 = 192 LSB nativi
#define TMP117_CRITICAL_DRIFT 192

// Stato interno per la baseline termica
typedef struct {
    int16_t history[EXP_SAMPLES]; // int16_t per supportare formati nativi TMP117 in due a complemento
    int32_t sum;
    uint8_t index;
    bool primed;
} temp_baseline_t;

static temp_baseline_t temp_base = {0};

/**
 * @brief Approssimazione sigmoide stabile in Q16.16
 * @details Usa 1/(1+e^-x) con espansione di Taylor fino a x^2/2
 * @param x Input Q16.16
 * @return Output Q16.16 in [0, F_ONE]
 */
static int32_t nelo_fast_sigmoid(int32_t x) {
    if (x <= -5 * F_ONE) return 0;
    if (x >= 5 * F_ONE) return F_ONE;

    int64_t x64 = x;
    int32_t e_x;

    if (x >= 0) {
        // e^x ≈ 1 + x + x^2/2
        e_x = F_ONE + x + (int32_t)((x64 * x64) >> 17);
        int64_t num = (int64_t)F_ONE * F_ONE;
        return F_ONE - (int32_t)(num / (F_ONE + e_x));
    } else {
        int32_t abs_x = -x;
        int64_t abs_x64 = abs_x;
        e_x = F_ONE + abs_x + (int32_t)((abs_x64 * abs_x64) >> 17);
        int64_t num = (int64_t)F_ONE * F_ONE;
        return (int32_t)(num / (F_ONE + e_x));
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

    // Aggiornamento FIFO
    temp_base.sum -= temp_base.history[temp_base.index];
    temp_base.history[temp_base.index] = t_skin_raw;
    temp_base.sum += t_skin_raw;
    temp_base.index = (temp_base.index + 1) % EXP_SAMPLES;

    if (drift <= 0) return 0;

    // Satura a 1.0 per drift >= 1.5°C (basato su costante LSB del sensore)
    int32_t s_delta_t = (drift * F_ONE) / TMP117_CRITICAL_DRIFT;
    if (s_delta_t > F_ONE) s_delta_t = F_ONE;

    return s_delta_t;
}

/**
 * @brief Calcolo deterministico dell'Indice di Danno D per SPEC-004 §3.1
 * @param s_g Punteggio conduttanza cutanea Q16.16 [0, F_ONE]
 * @param s_v Punteggio HRV invertito Q16.16 [0, F_ONE]. 1.0 = calma, 0.0 = shock
 * @param t_skin Temperatura cutanea attuale dal registro del sensore (TMP117 nativo)
 * @return D in Q16.16 [0, F_ONE]. Soglia di innesco a 45875 = 0.7
 */
uint32_t nelo_calculate_d_index(int32_t s_g, int32_t s_v, uint16_t t_skin) {
    // Clamping preventivo input
    if (s_g < 0) s_g = 0; else if (s_g > F_ONE) s_g = F_ONE;
    if (s_v < 0) s_v = 0; else if (s_v > F_ONE) s_v = F_ONE;

    // 1. Inversione semantica dell'HRV per isolare la componente di stress biologico
    // Se s_v è 1.0 (calma), lo stress_v è 0.0. Se s_v è 0.0 (shock), lo stress_v è 1.0.
    int32_t s_v_stress = F_ONE - s_v;

    // 2. Anomalia termica differenziale passata come intero con segno
    int32_t s_delta_t = process_thermal_anomaly((int16_t)t_skin);

    // 3. Somma lineare pesata con HRV corretto
    int64_t linear_sum = ((int64_t)WEIGHT_ALPHA * s_g) +
                         ((int64_t)WEIGHT_BETA * s_v_stress) +
                         ((int64_t)WEIGHT_GAMMA * s_delta_t);
    linear_sum >>= 16;

    // 4. Termine di rinforzo cross-channel interconnesso
    int64_t cross_product = ((int64_t)s_g * s_v_stress) >> 16;
    cross_product = (cross_product * s_delta_t) >> 16;
    int64_t reinforcement_term = (cross_product * WEIGHT_OMEGA) >> 16;

    // 5. Sintesi argomento logistico Z con barriera anti-overflow
    int64_t z64 = linear_sum + reinforcement_term - BIAS_DELTA;
    if (z64 > INT32_MAX) z64 = INT32_MAX;
    if (z64 < INT32_MIN) z64 = INT32_MIN;
    int32_t z = (int32_t)z64;

    // 6. Attivazione sigmoide hardened
    return (uint32_t)nelo_fast_sigmoid(z);
}
