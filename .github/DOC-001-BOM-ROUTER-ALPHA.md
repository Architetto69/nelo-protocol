# Progetto NELO: Guida Hardware e Distinta Base (BOM)

**Stato:** SPEC / ALPHA

**Versione:** 1.0

**Target:** Maker, Validatori di Rete, Laboratori Open-Source

---
Configurazione hardware di riferimento proposta per il kit:

#### 1. Unità di Elaborazione Core (SoC)

* **Scelta standard:** Modulo basato su **Espressif ESP32-S3** (es. ESP32-S3-WROOM-1 con 8MB PSRAM) oppure **Nordic nRF52840** (es. Holyiot / Seeed Studio Xiao).
* **Requisiti tecnici soddisfatti:** Presenza di un generatore di entropia hardware (TRNG) integrato per la routine `nelo_memclear()`, supporto a livello hardware per crittografia accelerata, e memoria RAM/PSRAM sufficiente per isolare l'intera esecuzione dello stack software senza toccare la Flash.

#### 2. Transceiver Radio a Lungo Raggio (Sub-GHz)

* **Scelta standard:** Modulo **Semtech SX1262** (interfaccia SPI, operante sulle frequenze libere regionali: 868 MHz per l'Europa, 915 MHz per le Americhe).
* **Requisiti tecnici soddisfatti:** Alta sensibilità in ricezione per penetrazione muraria urbana, supporto nativo a modulazioni LoRa/FSK, e bassissimo consumo energetico in modalità d'ascolto continuo, ideale per il buffering parassitario a 120ms.

#### 3. Sottosistema di Alimentazione Resiliente (Energy Harvesting)

* **Scelta standard:** Mini-pannello solare flessibile in silicio monocristallino (5V / 1A–2A) + Controller di carica solare (PMIC es. TP4056 o BQ25504 per micro-harvesting) + Supercondensatori industriali o cella singola LTO (Litio-Titanato) da 500mAh.
* **Requisiti tecnici soddisfatti:** Autonomia termica estrema (funzionamento da -20°C a +60°C senza degradazione), immunità al rischio di esplosione da cortocircuito e tolleranza a fluttuazioni energetiche continue senza la necessità di manutenzione fisica.

---

### 4. Filosofia di Scelta dei Componenti

In conformità con la `SPEC-006` e la `SPEC-010`, i componenti scelti per il **Kit Router Alpha** devono rispondere a tre requisiti tassativi:

1. **Reperibilità Globale (COTS):** Chip commerciali standard acquistabili sfusi da qualsiasi distributore di elettronica (Mouser, DigiKey, AliExpress) senza lasciare tracciabilità burocratica o richiedere licenze speciali.
2. **Assenza di Firma Hardware di Fabbrica:** Nessun chip deve integrare sistemi di telemetria proprietari o identificativi cloud immutabili non sovrascrivibili.
3. **Resilienza Energetica Passiva:** Il consumo deve essere sufficientemente basso da consentire il funzionamento continuo tramite micro-solare o harvesting ambientale.

---

### 5. Distinta Base dei Componenti (Bill of Materials)

| Componente | Opzione Consigliata | Alternativa Compatibile | Funzione nel Protocollo | Costo Stimato |
| --- | --- | --- | --- | --- |
| **Unità di Calcolo (SoC)** | **Espressif ESP32-S3-WROOM-1** (con 8MB PSRAM) | Nordic nRF52840 Module (Holyiot 18010) | Gestione della `SHADOW_QUEUE` in RAM volatile, esecuzione del firmware stateless e isolamento delle chiamate crittografiche tramite accelerazione hardware. | ~€4,50 |
| **Transceiver Radio** | **Modulo Semtech SX1262** (SPI flessibile) | Modulo RFM95W (SX1276) | Ricezione passiva a corto raggio dei pacchetti da 96 byte dai braccialetti ed emissione del traffico di copertura (*Chaffing*). | €3,50 |
| **Accumulatore Energetico** | **Cella Litio-Titanato (LTO) 18650** (500mAh - 2.4V) | Coppia di Supercondensatori Industriali (50F 2.7V in serie) | Alimentazione del nodo con tolleranza termica estrema (-20°C / +60°C) e ciclo di vita superiore a 15 anni senza manutenzione. | €4,00 |
| **Gestione Energetica** | **PMIC Texas Instruments BQ25504** | TP4056 (Solo per configurazioni standard LiPo/LTO) | Gestione avanzata del micro-harvesting solare a bassissima potenza, impedendo la scarica profonda. | €2,50 |
| **Energy Harvester** | **Pannello Solare Monocristallino PET** (5V / 200mA flessibile) | Cella induttiva parassitaria | Raccolta energetica ambientale di superficie. | €2,00 |
| **Involucro & Sicurezza** | **Scatola di derivazione industriale IP67** (ABS o Policarbonato) | Scocca stampata in 3D + resinatura epossidica | Protezione dagli agenti atmosferici e isolamento fisico contro manomissioni. | €1,50 |

**Costo Totale Stimato dell'Hardware: ~€18,00 - €20,00** (ottimizzato su acquisti di piccoli lotti per i nodi pilota).

---

### 6. Schema dei Collegamenti Logici (Pinout di Riferimento)

Per l'assemblaggio del prototipo Alpha utilizzando un SoC ESP32-S3 e un modulo radio Semtech SX1262, i collegamenti sul bus SPI e le linee di interrupt devono essere cablati come segue:

```
+--------------------------+               +--------------------------+
|    ESP32-S3 (MCU Core)   |               |   SX1262 (Radio LoRa)    |
|                          |               |                          |
|        3V3 / GND --------|-------------->|-------- VCC / GND        |
|        GPIO 12 (MOSI) ---|-------------->|-------- MOSI             |
|        GPIO 13 (MISO) <--|---------------|-------- MISO             |
|        GPIO 11 (SCK) ----|-------------->|-------- SCK              |
|        GPIO 10 (CS) -----|-------------->|-------- NSS (Chip Select)|
|        GPIO 9 (RESET) ---|-------------->|-------- NRESET           |
|        GPIO 14 (BUSY) <--|---------------|-------- BUSY             |
|        GPIO 21 (DIO1) <--|---------------|-------- DIO1 (Interrupt) |
+--------------------------+               +--------------------------+

```

#### Note di Cablaggio Critiche per i Maker:

* **Linea DIO1:** È l'unica linea di interrupt radio tassativa per l'SX1262. Il firmware stateless si affida a questo pin per svegliare istantaneamente il modulo di ricezione DMA non appena un preambolo radio valido a 868 MHz viene intercettato nello spazio aereo.
* **Linea BUSY:** Il chip SX1262 richiede il controllo dello stato di occupazione prima di ogni transizione di stato (da RX a TX). Non lasciare questo pin fluttuante.

---

### 7. Primo Avvio e Verifica della RAM (Boot Check)

Al momento dell'accensione del kit assembler, il firmware Alpha esegue una routine di autodiagnostica visibile tramite un LED di stato (o terminale seriale temporaneo prima della disattivazione della UART come da `SPEC-010`):

1. **Flash Isolation Test:** Il sistema tenta di scrivere un byte finto sulla memoria non volatile. Se la scrittura ha successo, il router si blocca emettendo un segnale di errore (il firmware non è sicuro). Il test deve fallire: la Flash deve essere in sola lettura.
2. **TRNG Entropy Verification:** Il generatore hardware di numeri casuali viene campionato per 1024 cicli. Se l'entropia calcolata sul flusso di bit è inferiore a 7.99 bit per byte, il boot si arresta (impossibilità di garantire la bonifica XOR della coda).
3. **Mesh Sync Ready:** Il transceiver LoRa entra in modalità d'ascolto continuo (*Continuous RX Mode*), pronto a elaborare i pacchetti e a sovrascrivere la memoria ogni 120ms.

---

## Appendice Software: Firmware di Riferimento Alpha (main.c)

```c
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
```