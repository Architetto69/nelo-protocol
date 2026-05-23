/**
 * @file nelo_transfer_function.c
 * @brief Gestione delle matrici di rischio locali e campionamento bus I2C privato schermato.
 * @note Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
 * @version 3.1-HARDENED
 */

#include "nrf.h"
#include <stdint.h>
#include <stdbool.h>

// Parametri di calcolo in virgola fissa (Q16.16: 65536 = 1.0)
#define F_ONE          65536
#define EXP_SAMPLES    5         // Finestra media mobile (~5 secondi a 1Hz)

// Pesi hardware della funzione di trasferimento (Q16.16)
#define WEIGHT_ALPHA   (131072)  // 2.0 in Q16.16: Conduttanza Cutanea (G)
#define WEIGHT_BETA    (163840)  // 2.5 in Q16.16: Variabilità Cardiaca (V_hrv)
#define WEIGHT_GAMMA   (196608)  // 3.0 in Q16.16: Velocità Crollo Termico (dT/dt)
#define BIAS_DELTA     (196608)  // 3.0 in Q16.16: Offset difensivo di sbarramento

/**
 * @brief Approssimazione polinomiale sicura della sigmoide: 1 / (1 + e^-x)
 * @details Previene l'overflow a 32 bit espandendo i quadrati intermedi a 64 bit.
 *          Garantisce l'assenza di inversioni di segno sotto carichi limite.
 * @param x Valore di input in formato fixed-point Q16.16
 * @return Output Q16.16 limitato rigorosamente nell'intervallo [0, F_ONE]
 */
static int32_t nelo_fast_sigmoid(int32_t x) {
    if (x < -5 * F_ONE) return 0;
    if (x > 5 * F_ONE) return F_ONE;
    
    int64_t x_64 = x;
    int32_t e_x;
    
    if (x >= 0) {
        // e^x ~= 1 + x + (x^2 / 2)
        // Spostiamo lo shift per allineare il quadrato da Q32.32 a Q16.16: >> 16, più un extra >> 1 per la divisione per 2 (totale >> 17)
        e_x = F_ONE + x + (int32_t)((x_64 * x_64) >> 17);
        
        // 1 - 1/(1+e_x) per l'ala positiva
        int64_t num = (int64_t)F_ONE * F_ONE;
        return F_ONE - (int32_t)(num / (F_ONE + e_x));
    } else {
        int32_t abs_x = -x;
        int64_t abs_x_64 = abs_x;
        
        e_x = F_ONE + abs_x + (int32_t)((abs_x_64 * abs_x_64) >> 17);
        int64_t num = (int64_t)F_ONE * F_ONE;
        return (int32_t)(num / (F_ONE + e_x));
    }
}

/**
 * @brief Calcolo deterministico dell'Indice di Danno D (SPEC-004 Sezione 3.1)
 * @param raw_g Conduttanza cutanea normalizzata [0, F_ONE]
 * @param raw_v_hrv HRV normalizzato [0, F_ONE] (1.0 = massima calma, 0.0 = shock)
 * @param raw_dt Gradiente termico normalizzato [0, F_ONE]
 * @return Intero a 16 bit a virgola fissa [0, 65535] pronto per il Payload AEAD
 */
uint16_t nelo_compute_damage_index(int32_t raw_g, int32_t raw_v_hrv, int32_t raw_dt) {
    // Calcolo dei singoli contributi pesati (espansione temporanea a 64 bit per evitare overflow da prodotto)
    int64_t g_contrib = ((int64_t)raw_g * WEIGHT_ALPHA) >> 16;
    
    // Inversione della variabilità cardiaca: lo stress aumenta al diminuire di V_hrv
    int32_t v_inv = F_ONE - raw_v_hrv;
    int64_t v_contrib = ((int64_t)v_inv * WEIGHT_BETA) >> 16;
    
    int64_t dt_contrib = ((int64_t)raw_dt * WEIGHT_GAMMA) >> 16;
    
    // Combinazione lineare dell'argomento della sigmoide: z = a*G + b*(1-V) + g*dT - delta
    int32_t z = (int32_t)(g_contrib + v_contrib + dt_contrib - BIAS_DELTA);
    
    // Passaggio attraverso la funzione di attivazione sigmoidale hardened
    int32_t d_q16 = nelo_fast_sigmoid(z);
    
    // Mappatura finale da Q16.16 a Uint16 a virgola fissa [0, 65535] per il pacchetto radio
    if (d_q16 >= F_ONE) return 0xFFFF;
    if (d_q16 <= 0)      return 0x0000;
    
    return (uint16_t)((d_q16 * 65535) >> 16);
}
/**
 * @brief Calcola la velocità di crollo termico isolandola dalla temperatura ambiente
 * @param t_skin_raw Temperatura cutanea attuale (es: 3350 = 33.5°C)
 * @return Punteggio di anomalia S_deltaT in formato Q16.16
 */
static int32_t process_thermal_anomaly(uint16_t t_skin_raw) {
    if (!temp_base.primed) {
        for (int i = 0; i < EXP_SAMPLES; i++) temp_base.history[i] = t_skin_raw;
        temp_base.sum = t_skin_raw * EXP_SAMPLES;
        temp_base.index = 0;
        temp_base.primed = true;
        return 0;
    }

    uint32_t current_baseline = temp_base.sum / EXP_SAMPLES;
    int32_t drift = (int32_t)current_baseline - (int32_t)t_skin_raw;
    
    temp_base.sum -= temp_base.history[temp_base.index];
    temp_base.history[temp_base.index] = t_skin_raw;
    temp_base.sum += t_skin_raw;
    temp_base.index = (temp_base.index + 1) % EXP_SAMPLES;

    if (drift <= 0) return 0;

    int32_t s_delta_t = (drift * F_ONE) / 150;
    if (s_delta_t > F_ONE) s_delta_t = F_ONE;

    return s_delta_t;
}

/**
 * @brief FUNZIONE DI TRASFERIMENTO CRITTOGRAFICA - INDICE D
 */
uint32_t nelo_calculate_d_index(int32_t s_g, int32_t s_v, uint16_t t_skin) {
    int32_t s_delta_t = process_thermal_anomaly(t_skin);

    int64_t linear_sum = ((int64_t)WEIGHT_ALPHA * s_g) + 
                         ((int64_t)WEIGHT_BETA * s_v) + 
                         ((int64_t)WEIGHT_GAMMA * s_delta_t);
    linear_sum >>= 16; 

    int64_t cross_product = ((int64_t)s_g * s_v) >> 16;
    cross_product = (cross_product * s_delta_t) >> 16;
    
    int64_t reinforcement_term = (cross_product * WEIGHT_OMEGA) >> 16;
    int32_t z = (int32_t)(linear_sum + reinforcement_term) - BIAS_DELTA;

    return (uint32_t)nelo_fast_sigmoid(z);
}


/**
 * @brief Calcola la velocità di crollo termico isolandola dalla temperatura ambiente
 * @param t_skin_raw Temperatura cutanea attuale (moltiplicata per 100, es: 3350 = 33.5°C)
 * @return Punteggio di anomalia S_deltaT in formato Q16.16
 */
static int32_t process_thermal_anomaly(uint16_t t_skin_raw) {
    if (!temp_base.primed) {
        // Inizializzazione della baseline dinamica locale
        for (int i = 0; i < EXP_SAMPLES; i++) temp_base.history[i] = t_skin_raw;
        temp_base.sum = t_skin_raw * EXP_SAMPLES;
        temp_base.index = 0;
        temp_base.primed = true;
        return 0;
    }

    uint32_t current_baseline = temp_base.sum / EXP_SAMPLES;
    
    // Calcolo della derivata discreta (Crollo repentino)
    int32_t drift = (int32_t)current_baseline - (int32_t)t_skin_raw;
    
    // Aggiornamento della media mobile (FIFO buffer)
    temp_base.sum -= temp_base.history[temp_base.index];
    temp_base.history[temp_base.index] = t_skin_raw;
    temp_base.sum += t_skin_raw;
    temp_base.index = (temp_base.index + 1) % EXP_SAMPLES;

    // Se la temperatura sale o oscilla normalmente, nessuna anomalia (S = 0)
    if (drift <= 0) return 0;

    // Se il crollo supera la soglia critica di 1.5°C (150 punti grezzi), satura a 1.0
    // Risolve il paradosso dell'innalzamento della temperatura esterna estiva
    int32_t s_delta_t = (drift * F_ONE) / 150;
    if (s_delta_t > F_ONE) s_delta_t = F_ONE;

    return s_delta_t;
}

/**
 * @brief FUNZIONE DI TRASFERIMENTO CRITTOGRAFICA - INDICE D
 * @param s_g Punteggio derivativo conduttanza cutanea [0, F_ONE] (Q16.16)
 * @param s_v Punteggio invertito HF/LF variabilità cardiaca [0, F_ONE] (Q16.16)
 * @param t_skin Temperatura cutanea attuale dal sensore hardware
 * @return uint32_t Indice D in formato Q16.16 (Soglia di innesco se >= 45875, ossia 0.7)
 */
uint32_t nelo_calculate_d_index(int32_t s_g, int32_t s_v, uint16_t t_skin) {
    // 1. Estrazione dinamica dell'anomalia termica differenziale
    int32_t s_delta_t = process_thermal_anomaly(t_skin);

    // 2. Calcolo dei componenti lineari pesati
    int64_t linear_sum = ((int64_t)WEIGHT_ALPHA * s_g) + 
                         ((int64_t)WEIGHT_BETA * s_v) + 
                         ((int64_t)WEIGHT_GAMMA * s_delta_t);
    linear_sum >>= 16; // Riconversione a Q16.16 dopo la moltiplicazione

    // 3. Calcolo del Moltiplicatore Geometrico Cross-Channel (Fattore Omega)
    // S_g * S_v * S_delta_t
    int64_t cross_product = ((int64_t)s_g * s_v) >> 16;
    cross_product = (cross_product * s_delta_t) >> 16;
    
    int64_t reinforcement_term = (cross_product * WEIGHT_OMEGA) >> 16;

    // 4. Sintesi dell'Argomento Logistico Centrale (Z)
    int32_t z = (int32_t)(linear_sum + reinforcement_term) - BIAS_DELTA;

    // 5. Attivazione ed estrazione dell'Indice asettico D
    return (uint32_t)nelo_fast_sigmoid(z);
}
