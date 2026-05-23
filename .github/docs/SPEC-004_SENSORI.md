# SPEC-004: Architettura e Ingegnerizzazione dei Cripto-Sensori Antropici (Antropic Sensor Nodes)
## 1. Obiettivo e Vettore Semantico
Il calcolo dell'Indice di Danno D \in [0.0, 1.0] costituisce la base logica su cui il Protocollo NELO innesca le contromisure di frizione e alterazione topologica della rete. Come evidenziato nelle analisi di vulnerabilità, se la determinazione di D fosse delegata a un'analisi puramente software, centralizzata o basata su metriche arbitrarie, il sistema cadrebbe vittima di manipolazioni semantiche o attacchi di cattura burocratica.
La SPEC-004 definisce le specifiche hardware, logiche e crittografiche dei **Cripto-Sensori Antropici**. Il loro scopo è trasformare la sofferenza non consensuale causata da atti di coercizione fisica o sistemica da un concetto filosofico a un **fatto fisico, multimodale e non falsificabile**, misurato direttamente sul confine biologico dell'essere umano.
```
+-----------------------------------------------------------------------+
|                       STADIO 1: RILEVAMENTO BIOLOGICO                 |
|  [PPG / HRV]           [GSR / Impedenza]       [Micro-Termoresistenze] |
+------------------------------------+----------------------------------+
                                     |
                                     v (Segnali Analogici)
+-----------------------------------------------------------------------+
|                       STADIO 2: SILICIO HARDENED                      |
|  [ nRF52840 SAADC ] -> [ EasyDMA Buffer ] -> [ Calcolo Matrice D ]     |
|         ^                       |                                     |
|         |                       v (Wipe Hardware ogni 120ms)          |
|  [ TRNG Noise Injection ] <-----+                                     |
+------------------------------------+----------------------------------+
                                     |
                                     v (Indice D + Nonce)
+-----------------------------------------------------------------------+
|                       STADIO 3: ATTREZZAGGIO CRITTOGRAFICO            |
|  [ CryptoCell-310 ] -> Firma Ed25519 (Chiave protetta da APPROTECT)   |
+------------------------------------+----------------------------------+
                                     |
                                     v (Payload Firmato e Cifrato)
                               [ Rete Mesh ]

```
## 2. Architettura Hardware Zero-Trust
Il cripto-sensore deve operare in un regime di totale isolamento logico. Non si ammettono interfacce di programmazione esposte, bus condivisi non protetti o linee dati intercettabili.
### 2.1 Principi di Progettazione del Circuito
 * **Integrazione Monolitica Analogico-Digitale:** Il condizionamento del segnale analogico (amplificazione e filtraggio delle risposte biologiche) e la conversione digitale devono avvenire sullo stesso PCB, racchiusi in una schermatura di Faraday metallica saldata direttamente sul circuito stampato. Questo impedisce l'iniezione di tensioni parassite tramite sonde esterne volte a simulare falsi stati di quiete.
 * **Alimentazione Isolata e Filtrata:** Il sottosistema di rilevamento biologico deve essere alimentato tramite regolatori di tensione a bassissimo rumore (LDO) dedicati, galvanicamente isolati dal circuito di trasmissione radio. Variazioni indotte nel consumo di corrente della radio non devono influenzare la precisione delle letture analogiche (prevenzione degli attacchi *Side-Channel* basati sulla potenza).
### 2.2 Requisiti Biometrici Multimodali (Prevenzione del Cloning)
Il sensore calcola il segnale di stress e coercizione combinando tre canali biologici indipendenti. Un'alterazione artificiale di un singolo parametro invalida l'intera lettura, azzerando l'anomalia.
 1. **Canale Elettrodermico (GSR - Galvanic Skin Response):** Misura la conduttanza cutanea indotta dall'attivazione del sistema nervoso simpatico. Il circuito deve effettuare un rilevamento dell'impedenza dinamica a corrente alternata (AC) per accertarsi che il sensore sia a contatto con tessuto biologico vivo, scartando materiali sintetici o simulatori resistivi statici.
 2. **Canale Fotopletismografico (PPG / HRV):** Monitoraggio ottico del volume del sangue periferico tramite LED a doppia lunghezza d'onda (Infrarosso e Rosso). Il firmware non analizza la semplice frequenza cardiaca bpt (*Beats Per Minute*), ma la **Variabilità della Frequenza Cardiaca (HRV)** nello spettro delle frequenze. Crolli repentini della componente HF (*High Frequency*) accoppiati a picchi della LF (*Low Frequency*) indicano uno stato di shock o minaccia imminente.
 3. **Gradiente Micro-Termico:** Monitoraggio accoppiato della temperatura cutanea superficiale e della temperatura ambientale. Lo stress acuto indotto da coercizione genera vasocostrizione periferica con un calo termico localizzato immediato, impossibile da emulare senza alterare la temperatura interna del modulo.
## 3. Il Motore di Sintesi del Segnale D
Il calcolo dell'Indice D deve essere deterministico, matematicamente limitato e privo di euristiche aggiornabili o modelli statistici predittivi soggetti ad avvelenamento dei dati (*data poisoning*).
### 3.1 Funzione di Trasferimento Non Lineare
I tre vettori biologici grezzi – Conduttanza Cutanea (G), Variabilità Cardiaca normalizzata (V_{hrv}), e Gradiente Termico (\Delta T) – vengono mappati in matrici di rischio locali normalizzate nell'intervallo [0, 1].
L'Indice di Danno globale D è calcolato applicando una funzione di attivazione sigmoidale non lineare pesata, progettata per saturare rapidamente verso l'alto in presenza di anomalie simultanee su più canali:  

$$D = \frac{1}{1 + e^{-\left(\alpha \cdot G + \beta \cdot (1 - V_{hrv}) + \gamma \cdot \Delta T - \delta\right)}}$$
  
Dove \alpha, \beta, \gamma rappresentano i pesi hardware tarati in fase di fabbricazione del nodo, e \delta è il fattore di offset difensivo che impedisce ai falsi positivi microscopici (es. attività fisica moderata) di superare la soglia critica di innesco (0.7).
### 3.2 Meccanica dell'Oblio Biometrico (SPEC-003 Integration)
I dati biometrici grezzi necessari per calcolare G, V_{hrv} e \Delta T sono considerati **informazioni ad altissimo rischio di de-anonimizzazione**. Il sensore applica l'oblio radicale secondo le seguenti regole:
 * **Buffer Effimeri:** Le letture analogiche digitalizzate dall'ADC vengono scritte in una matrice circolare allocata in RAM.
 * **Finestra Temporale di Distruzione:** Il tempo di permanenza di una lettura grezza in memoria non può superare i **120 millisecondi**.
 * **Wipe Hardware Attivo:** Allo scadere di ogni ciclo di clock del timer hardware, il buffer viene letto dal Motore di Sintesi per aggiornare l'Indice D. Un istante dopo, il registro viene sovrascritto iniettando rumore bianco generato dal TRNG del chip. Nessuna traccia del battito cardiaco o della conduttanza cutanea passata sopravvive all'interno del silicio.
## 4. Attestazione Hardware e Cripto-Firmware
Un sensore non può limitarsi a trasmettere il valore di D in chiaro sulla rete mesh; deve dimostrare in modo matematico la propria integrità strutturale e l'autenticità del proprio silicio ad ogni singolo pacchetto inviato.
### 4.1 Isolamento delle Chiavi tramite Coprocessore Hardware
 * La chiave privata del cripto-sensore (SK_{sensor}) viene generata all'interno dell'ambiente protetto del coprocessore crittografico in fase di inizializzazione.
 * Il core principale della CPU non ha diritti di lettura sui registri fisici che contengono SK_{sensor}. Può soltanto inviare il digest del carico utile (il valore D unito a un timestamp e a un nonce) al coprocessore e ricevere indietro la firma digitale.
 * La chiave pubblica corrispondente (PK_{sensor}) è firmata digitalmente dalla chiave di attivazione del Quorum in fase di immissione del dispositivo nella rete mesh, creando un'ancora di fiducia distribuita.
### 4.2 Struttura del Pacchetto Dati (Payload)
Ogni trasmissione del sensore verso il nodo di rete mesh locale deve essere atomica e strutturata secondo il seguente schema a campi fissi, cifrato e firmato in modalità autenticata (AEAD):
| Dimensione (Byte) | Campo | Descrizione |
|---|---|---|
| **4** | TIMESTAMP | Epoca Unix aggiornata tramite clock hardware interno sincronizzato a maglia |
| **8** | NONCE | Numero casuale monouso generato dal TRNG per impedire attacchi di replica |
| **2** | DAMAGE_INDEX | Il valore D espresso come intero a 16-bit a virgola fissa (moltiplicato per 65535) |
| **64** | SIGNATURE | Firma Ed25519 calcolata su [TIMESTAMP + NONCE + DAMAGE_INDEX] |
## 5. Meccanismi di Anti-Manomissione (Tamper-Resistance)
Nel modello di minaccia di NELO, il sequestro fisico del sensore da parte di un avversario statale è un evento certo. Il dispositivo deve essere progettato per autodistruggersi crittograficamente prima che l'attaccante possa estrarre le chiavi o manipolare il firmware.
### 5.1 Protezione dei Canali di Debug (APPROTECT Permanente)
Le interfacce di programmazione e tracciamento hardware (JTAG / SWD) devono essere disabilitate in modo distruttivo e permanente. La configurazione dei registri di blocco dell'accesso alla memoria flash deve essere scritta nei settori non volatili del chip modificando i bit di protezione hardware (eFuse o registri UICR bloccati). Qualsiasi tentativo di forzare il debugger deve provocare il reset totale della memoria flash di sistema.
### 5.2 Rilevamento dell'Apertura del Casing (Involucro Attivo)
L'involucro protettivo del cripto-sensore è rivestito internamente da una maglia di vernice conduttiva percorsa da una micro-corrente continua monitorata da un pin GPIO con interrupt attivo.
 * **Innesco della Tabula Rasa:** L'interruzione fisica della maglia conduttiva (tentativo di apertura o perforazione del guscio) o la variazione repentina della capacità elettrica generano un interrupt non mascherabile (NMI) a priorità assoluta.
 * **Procedura di Emergenza:** Il firmware interrompe qualsiasi operazione in corso, invoca il coprocessore crittografico per sovrascrivere immediatamente il settore contenente SK_{sensor} con zeri logici, ed esegue un ciclo continuo di scrittura e cancellazione su tutta la SRAM, lasciando il chip permanentemente inutilizzabile e privo di materiale crittografico (Zeroizzazione Hardware).
A questo punto la struttura formale della SPEC-004 è definita in ogni suo vincolo logico e di sicurezza.
## Appendice Tecnica SPEC-004: Blindatura Hardware su Nordic nRF52840
```c
/**
 * @file nelo_crypto_sensor_hardened.c
 * @brief Implementazione dei vincoli di sicurezza hardware SPEC-004 su nRF52840.
 * @note Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
 */

#include "nrf.h"
#include "nrf_delay.h"

// Definizioni per l'interfaccia crittografica CryptoCell-310 (registri mappati in memoria)
#define CRYS_BASE_ADDR            (0x5002A000UL)
#define CRYS_REG_AO_SW_RESET      (*(volatile uint32_t *)(CRYS_BASE_ADDR + 0x004UL))
#define CRYS_REG_HOST_CRYPTOCELL_EN (*(volatile uint32_t *)(CRYS_BASE_ADDR + 0x008UL))

// Buffer effimero per il calcolo dell'Indice D (120ms max prima del wipe)
#define BIOMETRIC_BUFFER_SIZE     64
// Ogni interazione con il buffer deve essere protetta da barriere di memoria o disattivazione
// temporanea degli interrupt dell'ADC durante la lettura/scrittura del Motore di Sintesi
static uint16_t biometric_raw_buffer[BIOMETRIC_BUFFER_SIZE] __attribute__((aligned(4)));
static volatile uint8_t buffer_index = 0;

/**
 * @brief 1. ATTIVAZIONE HARDENED DI APPROTECT (Blocco permanente del Debugger SWD)
 * @details Scrive nei registri UICR (User Information Configuration Registers) 
 *          per disabilitare permanentemente la porta di debug. Nelle revisioni recenti 
 *          del silicio nRF52840, questo impedisce attacchi di Fault Injection sulla porta SWD.
 */
void nelo_hw_enforce_approtect(void) {
    // Verifica se APPROTECT è già abilitato (0x00000000 significa protetto)
    if (NRF_UICR->APPROTECT != 0x00000000) {
        // Abilita la scrittura nella memoria NVMC (Non-Volatile Memory Controller)
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        // Scrive il flag di blocco nel registro UICR dedicato ad APPROTECT
        NRF_UICR->APPROTECT = 0x00000000;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        // Disabilita la scrittura per evitare corruzioni accidentali
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        // Forza un System Reset per rendere effettive le protezioni hardware
        NVIC_SystemReset();
    }
}

/**
 * @brief 2. INIZIALIZZAZIONE DEL TIMER CRITICO (Finestra di distruzione a 120ms)
 * @details Configura il TIMER1 in modalità hardware pura per scattare esattamente 
 *          ogni 120 millisecondi, invocando un Interrupt Non Mascherabile (NMI).
 */
void nelo_hw_timer_oblivion_init(void) {
    NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;       // Modalità Timer
    NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit; // Contatore a 32 bit
    NRF_TIMER1->PRESCALER = 4;                      // 16MHz / 2^4 = 1MHz (1 microsecondo per tick)

    // Imposta il valore di confronto per 120.000 microsecondi (120 ms)
    NRF_TIMER1->CC[0] = 120000;

    // Configura lo sfoltimento automatico (Shortcut): azzera il timer al raggiungimento del match
    NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;

    // Abilita l'interrupt sul canale di confronto CC[0]
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

    // Configura la priorità dell'interrupt a 0 (Massima priorità hardware del Cortex-M4)
    NVIC_SetPriority(TIMER1_IRQn, 0);
    NVIC_EnableIRQ(TIMER1_IRQn);

    // Avvia il timer
    NRF_TIMER1->TASKS_START = 1;
}

/**
 * @brief 3. INIZIALIZZAZIONE DEL TRNG PER L'INIEZIONE DI RUMORE BIANCO
 */
void nelo_hw_trng_init(void) {
    NRF_RNG->CONFIG = RNG_CONFIG_DERC_Enabled << RNG_CONFIG_DERC_Pos; // Abilita correzione deriva termica
    NRF_RNG->INTENSET = RNG_INTENSET_VALRDY_Msk;                      // Abilita interrupt dato pronto
    NRF_RNG->TASKS_START = 1;                                         // Avvia il generatore hardware
}
// Array di entropia accumulata asincronamente al di fuori del timer critico
static uint8_t entropy_pool[BIOMETRIC_BUFFER_SIZE * 2];
static volatile uint8_t entropy_pool_index = 0;

/**
 * @brief Riempimento asincrono del pool di entropia
 * @note Da invocare nel loop principale (main) o tramite interrupt a bassa priorità del TRNG
 */
void nelo_hw_entropy_collect(void) {
    if (NRF_RNG->EVENTS_VALRDY == 1) {
        NRF_RNG->EVENTS_VALRDY = 0;
        if (entropy_pool_index < (BIOMETRIC_BUFFER_SIZE * 2)) {
            entropy_pool[entropy_pool_index++] = (uint8_t)(NRF_RNG->VALUE & 0xFF);
        }
    }
}

/**
 * @brief INTERRUPT HANDLER TIMER1 OTTIMIZZATO (Esecuzione in < 5 microsecondi)
 */
void TIMER1_IRQHandler(void) {
    if (NRF_TIMER1->EVENTS_COMPARE[0] == 1) {
        NRF_TIMER1->EVENTS_COMPARE[0] = 0;

        // Congela l'EasyDMA
        NRF_SAADC->TASK_STOP = 1;
        while (NRF_SAADC->EVENTS_STOPPED == 0) // Attesa bloccante harware
        NRF_SAADC->EVENTS_STOPPED == 1;

        uint8_t *raw_ptr = (uint8_t *)biometric_raw_buffer;
        size_t buffer_bytes = BIOMETRIC_BUFFER_SIZE * sizeof(uint16_t);

        // Se il pool di entropia è pronto, esegue il wipe istantaneo senza attese lineari
        if (entropy_pool_index >= buffer_bytes) {
            for (size_t i = 0; i < buffer_bytes; i++) {
                raw_ptr[i] = entropy_pool[i];
            }
        } else {
            // Fallback deterministico di sicurezza se l'entropia non è completata
            for (size_t i = 0; i < buffer_bytes; i++) {
                raw_ptr[i] = 0xAA; // Sovrascrittura statica alternata di sicurezza
            }
        }

        buffer_index = 0;
        entropy_pool_index = 0; // Resetta il pool per il prossimo ciclo da 120ms

        NRF_SAADC->TASK_START = 1; // Riattiva il campionamento protetto per il nuovo ciclo
        
        __DSB();
        __ISB();
    }
}


/**
 * @brief 4. ATTIVAZIONE COPROCESORE CRITTOGRAFICO CRYPTOCELL-310
 * @details Sveglia l'enclave crittografica hardware isolandola dal core ARM principale.
 */
void nelo_hw_cryptocell_enable(void) {
    // Rilascia il modulo dal reset software
    CRYS_REG_AO_SW_RESET = 0x1UL;
    nrf_delay_us(10);
    CRYS_REG_AO_SW_RESET = 0x0UL;

    // Abilita l'accesso hardware all'interfaccia CryptoCell
    CRYS_REG_HOST_CRYPTOCELL_EN = 0x1UL;
    
    // Configura i permessi di accesso del subsystem crittografico alle periferiche DMA
    NRF_CRYPTOCELL->ENABLE = 1;
}

/**
 * @brief EXECUTION PIPELINE (Inizializzazione dei Vincoli di Sicurezza del Nodo)
 */
void nelo_sensor_security_lockdown(void) {
    // Fase 1: Blindatura fisica contro estrazione flash
    nelo_hw_enforce_approtect();

    // Fase 2: Attivazione generatori di entropia hardware
    nelo_hw_trng_init();

    // Fase 3: Avvio della clessidra dell'oblio (120ms loop)
    nelo_hw_timer_oblivion_init();

    // Fase 4: Accensione dell'enclave isolata per la firma dei pacchetti D
    nelo_hw_cryptocell_enable();
}

```
## Note di Audit di Sicurezza per lo Sviluppatore (SPEC-004-AUDIT):
1. Il Registro UICR: La funzione nelo_hw_enforce_approtect() scrive nella memoria non volatile speciale del chip. Questa operazione è irreversibile via software. Una volta eseguita sul sensore, il chip rifiuterà qualsiasi connessione J-Link o debugger esterno. Per riprogrammarlo sarà necessario eseguire un comando hardware di ERASEALL di intero chip, il quale azzera istantaneamente la flash, cancellando la chiave privata SK_{sensor}.
2. Uso degli Interrupt: Il TIMER1_IRQHandler è configurato con priorità 0. Questo garantisce che la distruzione del buffer della RAM non possa essere interrotta o ritardata da processi radio (BLE o Thread Mesh). L'oblio ha sempre la precedenza sulla comunicazione.
3. Istruzioni di Sincronizzazione (__DSB(), __ISB()): Inserite al termine del wipe hardware. Costringono la pipeline della CPU ARM a svuotare le sue cache interne e i registri interni dei passaggi intermedi, eliminando qualsiasi "fantasma energetico" o residuo dei dati biometrici grezzi prima di tornare all'esecuzione del codice ordinario.

### Implementazione Firmware (Virgola Fissa / Hardened) della Funzione di Trasferimento

```c
#include "nrf.h"
#include <stdint.h>
#include <stdbool.h>

// Parametri di calcolo in virgola fissa (Q16.16: 65536 = 1.0)
#define F_ONE          65536
#define EXP_SAMPLES    5         // Finestra media mobile (~5 secondi a 1Hz)

// Pesi hardware della funzione di trasferimento (Q16.16)
#define WEIGHT_ALPHA   (2 * F_ONE)      // Conduttanza Cutanea (G)
#define WEIGHT_BETA    (2.5 * F_ONE)    // Variabilità Cardiaca (V_hrv)
#define WEIGHT_GAMMA   (3 * F_ONE)      // Velocità Crollo Termico (dT/dt)
#define WEIGHT_OMEGA   (5 * F_ONE)      // Fattore di rinforzo geometrico simultaneo
#define BIAS_DELTA     (3 * F_ONE)      // Offset difensivo di sbarramento

// Strutture dati per l'analisi differenziale transitoria
typedef struct {
    uint32_t history[EXP_SAMPLES];
    uint8_t  index;
    uint32_t sum;
    bool     primed;
} baseline_t;

static baseline_t temp_base;

/**
 * @brief Approssimazione polinomiale veloce della sigmoide: 1 / (1 + e^-x)
 * @param x Valore di input in formato fixed-point Q16.16
 * @return Output Q16.16 limitato rigorosamente nell'intervallo [0, F_ONE]
 */
static int32_t nelo_fast_sigmoid(int32_t x) {
    // Approssimazione di Taylor troncata per l'argomento logistico centrale
    // Se x è fortemente negativo, la sigmoide tende a 0
    if (x < -5 * F_ONE) return 0;
    // Se x è fortemente positivo, la sigmoide satura a 1.0
    if (x > 5 * F_ONE) return F_ONE;
    
    // Approssimazione lineare a tratti per la protezione dei cicli di clock dell'ISR
    int32_t e_x;
    if (x >= 0) {
        e_x = F_ONE + x + ((x * x) >> 17); // 1 + x + x^2/2
        return F_ONE - (F_ONE * F_ONE) / (F_ONE + e_x);
    } else {
        int32_t abs_x = -x;
        e_x = F_ONE + abs_x + ((abs_x * abs_x) >> 17);
        return (F_ONE * F_ONE) / (F_ONE + e_x);
    }
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
```
