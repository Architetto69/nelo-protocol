#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_system.h"
#include "esp_log.h"

#define PACKET_SIZE          96
#define BUFFER_SLOTS         32
#define MEM_OBLIO_TTL_MS     120

#define PIN_NUM_MISO         13
#define PIN_NUM_MOSI         12
#define PIN_NUM_CLK          11
#define PIN_NUM_CS           10
#define PIN_NUM_DIO1         21
#define PIN_NUM_TAMPER       4   // Associato al micro-interruttore anti-manomissione

// Struttura dello slot della SHADOW_QUEUE allineata alla memoria
typedef struct {
    uint8_t payload[PACKET_SIZE];
    uint64_t timestamp;
    bool volatile_flag;
} __attribute__((aligned(4))) shadow_slot_t;

// Allocazione forzata in RAM interna (assenza di persistenza)
static shadow_slot_t shadow_queue[BUFFER_SLOTS];
static volatile uint8_t head_ptr = 0;

// Routine di bonifica immediata (SPEC-010): XOR distruttivo tramite TRNG hardware
static void nelo_memclear(size_t slot_idx) {
    uint32_t rand_mask;
    
    for (size_t i = 0; i < PACKET_SIZE; i += 4) {
        // Campionamento del generatore di entropia hardware dell'ESP32-S3
        rand_mask = esp_random(); 
        
        // Applicazione dello XOR distruttivo sulla RAM volatile
        *(uint32_t*)(&shadow_queue[slot_idx].payload[i]) ^= rand_mask;
    }
    
    shadow_queue[slot_idx].timestamp = 0;
    shadow_queue[slot_idx].volatile_flag = false;
}

// ISR a priorità assoluta per il rilevamento Tamper fisico
static void IRAM_ATTR tamper_isr_handler(void* arg) {
    // 1. Arresto immediato dei registri SPI periferici (Radio Lock)
    REG_WRITE(SPI_CMD_REG(1), 0);
    
    // 2. Bonifica istantanea di emergenza di tutta la RAM della coda
    for (size_t i = 0; i < BUFFER_SLOTS; i++) {
        for (size_t j = 0; j < PACKET_SIZE; j++) {
            shadow_queue[i].payload[j] = 0x00;
        }
    }
    
    // 3. Reset hardware forzato immediato (il bootloader verificherà la Flash in sola lettura)
    esp_restart();
}

// Task di monitoraggio dei tempi di permanenza (TTL Check)
void volatile_lifespan_task(void *pvParameters) {
    while(1) {
        uint64_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        for (size_t i = 0; i < BUFFER_SLOTS; i++) {
            if (shadow_queue[i].volatile_flag && 
               (current_time - shadow_queue[i].timestamp >= MEM_OBLIO_TTL_MS)) {
                
                nelo_memclear(i);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    // CONFIGURAZIONE SICUREZZA INTERNA
    // Inizializzazione GPIO Anti-Tamper (SPEC-010 / Clausola di Autodistruzione Logica)
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE, // Scatta quando la scocca viene aperta (circuito interrotto)
        .pin_bit_mask = (1ULL << PIN_NUM_TAMPER),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_NUM_TAMPER, tamper_isr_handler, NULL);

    // Boot Check 1: Test di isolamento Flash simulato a runtime
    // (I driver di scrittura devono essere assenti o disabilitati)
    
    // Boot Check 2: Controllo dell'indice di Entropia minima del TRNG
    uint32_t test_entropy = esp_random();
    if (test_entropy == 0 || test_entropy == 0xFFFFFFFF) {
        // Blocco del boot se il TRNG hardware non risponde correttamente
        while(1) { gpio_set_level(GPIO_NUM_2, 1); vTaskDelay(100); gpio_set_level(GPIO_NUM_2, 0); vTaskDelay(100); }
    }

    // Inizializzazione della coda circolare in uno stato pulito (Entropia iniziale)
    for (size_t i = 0; i < BUFFER_SLOTS; i++) {
        nelo_memclear(i);
    }

    // Creazione del task di controllo dell'oblio a 120ms
    xTaskCreate(volatile_lifespan_task, "nelo_oblio", 2048, NULL, configMAX_PRIORITIES - 1, NULL);

    // Il transceiver SX1262 viene configurato in Continuous RX tramite bus SPI...
}
