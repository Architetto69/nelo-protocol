/**
 * @file nelo_crypto_sensor_hardened.c
 * @brief Implementazione dei vincoli di sicurezza hardware SPEC-004 su nRF52840.
 * @note Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
 * @version 3.1-HARDENED
 */

#include "nrf.h"
#include "nrf_delay.h"
#include <stdint.h>
#include <stddef.h>

// --- INDIRIZZI I2C PERIFERICHE (SPEC-004) ---
#define MAX30102_ADDR             0x57  
#define MAX30009_ADDR             0x51  
#define TMP117_SKIN_ADDR          0x48  
#define TMP117_ENV_ADDR           0x49  

// Definizioni per l'interfaccia crittografica CryptoCell-310
#define CRYS_BASE_ADDR            (0x5002A000UL)
#define CRYS_REG_AO_SW_RESET      (*(volatile uint32_t *)(CRYS_BASE_ADDR + 0x004UL))
#define CRYS_REG_HOST_CRYPTOCELL_EN (*(volatile uint32_t *)(CRYS_BASE_ADDR + 0x008UL))

#define BIOMETRIC_BUFFER_SIZE     64
#define TIMEOUT_MAX_LOOPS         10000

// Buffer effimeri forzati in RAM (allineati a 32-bit per EasyDMA)
static volatile uint8_t biometric_raw_buffer[BIOMETRIC_BUFFER_SIZE] __attribute__((aligned(4)));
static uint8_t entropy_pool[BIOMETRIC_BUFFER_SIZE] __attribute__((aligned(4)));
static volatile uint8_t entropy_pool_index = 0;

// Buffer RAM obbligatorio per il comando EasyDMA (evita allocazione in Flash .rodata)
static uint8_t i2c_wipe_cmd[2] __attribute__((aligned(4))) = {0x04, 0x40};

/**
 * @brief INTERRUPT HANDLER TIMER1: Obliterazione totale (120ms) con protezione da stallo
 */
void TIMER1_IRQHandler(void) {
    // Doppia verifica per mitigare il salto dell'istruzione condizionale via glitching
    if (NRF_TIMER1->EVENTS_COMPARE[0] == 1 && NRF_TIMER1->EVENTS_COMPARE[0] != 0) {
        NRF_TIMER1->EVENTS_COMPARE[0] = 0;

        // Isola IMMEDIATAMENTE l'interfaccia TWIM spegnendo il modulo
        // Questo interrompe i bus master AHB legati a EasyDMA a prescindere dallo stato dei task
        NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
        NRF_TWIM0->TASKS_STOP = 1;

        // 2. Wipe distruttivo della RAM locale (Inversione dei bit di sfoltimento)
        uint32_t *raw_ptr32 = (uint32_t *)biometric_raw_buffer;
        if (entropy_pool_index >= BIOMETRIC_BUFFER_SIZE) {
            for (size_t i = 0; i < (BIOMETRIC_BUFFER_SIZE / 4); i++) {
                raw_ptr32[i] = ((uint32_t *)entropy_pool)[i];
            }
        } else {
            // Pattern alternato ad alta transizione di stato (0x55AA55AA) per scaricare i nodi di memoria
            for (size_t i = 0; i < (BIOMETRIC_BUFFER_SIZE / 4); i++) {
                raw_ptr32[i] = 0x55AA55AA; 
            }
        }
        entropy_pool_index = 0;

        // Ripristina il modulo per il comando esterno di Wipe
        NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);

        // 3. Reset FIFO esterno
        NRF_TWIM0->ADDRESS = MAX30102_ADDR;
        NRF_TWIM0->TXD.PTR = (uint32_t)i2c_wipe_cmd; 
        NRF_TWIM0->TXD.MAXCNT = 2;
        NRF_TWIM0->TASKS_STARTTX = 1;
        
        uint32_t timeout = TIMEOUT_MAX_LOOPS;
        while ((NRF_TWIM0->EVENTS_LASTTX == 0) && (--timeout > 0));
        NRF_TWIM0->EVENTS_LASTTX = 0;

        NRF_TWIM0->TASKS_STOP = 1;
        timeout = TIMEOUT_MAX_LOOPS;
        while ((NRF_TWIM0->EVENTS_STOPPED == 0) && (--timeout > 0));
        NRF_TWIM0->EVENTS_STOPPED = 0;

        // Forza la sincronizzazione della pipeline d'esecuzione ed esclude ottimizzazioni out-of-order della CPU
        __DSB();
        __ISB();
    }
}


/**
 * @brief 1. ATTIVAZIONE HARDENED DI APPROTECT (Blocco hardware del Debugger SWD)
 * @note Implementa la logica a doppio livello richiesta dalle Rev 3 del silicio nRF52840.
 */
void nelo_hw_enforce_approtect(void) {
    // Configurazione del registro UICR hardware permanente (Flash NVMC)
    if (NRF_UICR->APPROTECT != 0x00000000) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        NRF_UICR->APPROTECT = 0x00000000;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        // Forza un reset di sistema per applicare le modifiche dell'UICR
        NVIC_SystemReset();
    }

    // Blindatura a livello di runtime del core per bloccare attacchi di fault injection sul bus di boot
    // Se APPROTECT nell'UICR è attivo (0x00), il firmware DEVE forzare l'abilitazione sul blocco logico.
    #ifdef NRF_APPROTECT_HAS_LATCH
        if (NRF_APPROTECT->FORCEDISABLE == APPROTECT_FORCEDISABLE_FORCEDISABLE_ForceDisable) {
            NRF_APPROTECT->FORCEDISABLE = APPROTECT_FORCEDISABLE_FORCEDISABLE_Disabled;
        }
    #else
        // Mapping standard per i registri di controllo runtime di APPROTECT su core Cortex
        NRF_APPROTECT->DISABLE = 0x00000000; 
    #endif
}

/**
 * @brief 2. INIZIALIZZAZIONE DEL TIMER CRITICO (Finestra a 120ms)
 */
void nelo_hw_timer_oblivion_init(void) {
    NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER1->PRESCALER = 4;                      // 1MHz (1 us per tick)
    NRF_TIMER1->CC[0] = 120000;                     // 120 ms
    NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

    NVIC_SetPriority(TIMER1_IRQn, 0);               // Massima priorità hardware
    NVIC_EnableIRQ(TIMER1_IRQn);
    NRF_TIMER1->TASKS_START = 1;
}

/**
 * @brief 3. INIZIALIZZAZIONE DEL TRNG PER RUMORE BIANCO / ENTROPIA
 */
void nelo_hw_trng_init(void) {
    NRF_RNG->CONFIG = RNG_CONFIG_DERC_Enabled << RNG_CONFIG_DERC_Pos; // Correzione deriva termica
    NRF_RNG->INTENSET = RNG_INTENSET_VALRDY_Msk;
    NRF_RNG->TASKS_START = 1;
}

/**
 * @brief 4. ATTIVAZIONE COPROCESORE CRITTOGRAFICO CRYPTOCELL-310
 */
void nelo_hw_cryptocell_enable(void) {
    CRYS_REG_AO_SW_RESET = 0x1UL;
    nrf_delay_us(10);
    CRYS_REG_AO_SW_RESET = 0x0UL;

    CRYS_REG_HOST_CRYPTOCELL_EN = 0x1UL;
    NRF_CRYPTOCELL->ENABLE = 1;
}

/**
 * @brief EXECUTION PIPELINE (Lockdown di Sicurezza del Nodo)
 */
void nelo_sensor_security_lockdown(void) {
    nelo_hw_enforce_approtect();     // Fase 1: Blocco SWD Hardened
    nelo_hw_trng_init();             // Fase 2: Entropia
    nelo_hw_timer_oblivion_init();   // Fase 3: Finestra temporale 120ms
    nelo_hw_cryptocell_enable();     // Fase 4: Enclave CryptoCell
}
