/**
 * @file nelo_damage_synthesis.c
 * @brief Motore di sintesi deterministico in virgola fissa per l'Indice di Danno D.
 * @note Ottimizzato per nRF52840 (Cortex-M4). Implementazione SPEC-004 Sezione 3.
 * @version 3.1-HARDENED
 */

#include "nrf.h"
#include <stdint.h>
#include <stdbool.h>

// Parametri di calcolo in virgola fissa (Q16.16: 65536 = 1.0)
#define F_ONE          65536
#define F_HALF         32768

// Pesi hardware della funzione di trasferimento (Q16.16)
#define WEIGHT_ALPHA   (131072)  // 2.0 in Q16.16: Conduttanza Cutanea (G)
#define WEIGHT_BETA    (163840)  // 2.5 in Q16.16: Variabilità Cardiaca (V_hrv)
#define WEIGHT_GAMMA   (196608)  // 3.0 in Q16.16: Velocità Crollo Termico (dT/dt)
#define BIAS_DELTA     (196608)  // 3.0 in Q16.16: Offset difensivo di sbarramento

/**
 * @brief Approssimazione simmetrica e stabile della sigmoide tramite funzione razionale.
 * @details Sostituisce la serie di Taylor con una sigmoide algebrica normalizzata in Q16.16.
 *          Garantisce l'assenza di asimmetrie e previene divergenze polinomiali.
 * @param z Valore di input combinato in formato fixed-point Q16.16
 * @return Output Q16.16 limitato rigorosamente nell'intervallo [0, F_ONE]
 */
static int32_t nelo_fixed_sigmoid_hardened(int32_t z) {
    // Clamping preventivo per evitare overflow computazionali estremi
    if (z <= -12 * F_ONE) return 0;
    if (z >=  12 * F_ONE) return F_ONE;

    int32_t abs_z = (z < 0) ? -z : z;

    // Calcolo del denominatore: (1.0 + |z|) in Q16.16
    // Dividiamo z per il denominatore per ottenere la retta di transizione normalizzata
    int64_t num = ((int64_t)z) << 16;
    int32_t den = F_ONE + abs_z;
    int32_t intermediate = (int32_t)(num / den); // Risultato in Q16.16 compreso tra -1.0 e +1.0

    // Traslazione e riscalamento: D = 0.5 * (1.0 + intermedio)
    int32_t result = F_HALF + (intermediate >> 1);

    // Clamping finale di sicurezza sul confine biologico
    if (result < 0) return 0;
    if (result > F_ONE) return F_ONE;

    return result;
}

/**
 * @brief Calcolo deterministico dell'Indice di Danno D (SPEC-004 Sezione 3.1)
 * @param raw_g Conduttanza cutanea normalizzata [0, F_ONE]
 * @param raw_v_hrv HRV normalizzato [0, F_ONE] (1.0 = massima calma, 0.0 = shock)
 * @param raw_dt Gradiente termico normalizzato [0, F_ONE]
 * @return Intero a 16 bit a virgola fissa [0, 65535] pronto per il Payload AEAD
 */
uint16_t nelo_compute_damage_index(int32_t raw_g, int32_t raw_v_hrv, int32_t raw_dt) {
    // Protezione da input fuori scala (Clamping preventivo dei canali analogici)
    if (raw_g < 0) raw_g = 0; else if (raw_g > F_ONE) raw_g = F_ONE;
    if (raw_v_hrv < 0) raw_v_hrv = 0; else if (raw_v_hrv > F_ONE) raw_v_hrv = F_ONE;
    if (raw_dt < 0) raw_dt = 0; else if (raw_dt > F_ONE) raw_dt = F_ONE;

    // 1. Calcolo dei singoli contributi pesati (Spostamento del prodotto a 64 bit)
    int64_t g_contrib = ((int64_t)raw_g * WEIGHT_ALPHA) >> 16;
    
    // Inversione logica HRV: la reazione di stress aumenta al decrescere della variabilità
    int32_t v_inv = F_ONE - raw_v_hrv;
    int64_t v_contrib = ((int64_t)v_inv * WEIGHT_BETA) >> 16;
    
    int64_t dt_contrib = ((int64_t)raw_dt * WEIGHT_GAMMA) >> 16;
    
    // 2. Combinazione lineare dei vettori: z = α·G + β·(1-V) + γ·ΔT - δ
    int32_t z = (int32_t)(g_contrib + v_contrib + dt_contrib - BIAS_DELTA);
    
    // 3. Elaborazione tramite funzione di attivazione sigmoidea antiriflesso
    int32_t d_q16 = nelo_fixed_sigmoid_hardened(z);
    
    // 4. Mappatura formale da Q16.16 a Uint16 fixed-point [0, 65535]
    // Poiché 1.0 in Q16.16 corrisponde a 65536, per mappare su 65535 eseguiamo un adattamento lineare sicuro
    uint32_t d_final = (uint32_t)((((int64_t)d_q16) * 65535) >> 16);
    
    if (d_final > 65535) {
        d_final = 65535;
    }

    return (uint16_t)d_final;
}
