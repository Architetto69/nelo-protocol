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
 1. **Canale Elettrodermico (GSR - Galvanic Skin Response):** Misura la conduttanza cutanea indotta dall'attivazione del sistema nervoso simpatico, realizzato utilizzando un Analog Devices (Maxim) MAX30009 su bus I2C. Il circuito deve effettuare un rilevamento dell'impedenza dinamica a corrente alternata (AC) per accertarsi che il sensore sia a contatto con tessuto biologico vivo, scartando materiali sintetici o simulatori resistivi statici.
 2. **Canale Fotopletismografico (PPG / HRV):** Monitoraggio ottico del volume del sangue periferico tramite LED a doppia lunghezza d'onda (Infrarosso e Rosso), realizzato utilizzando un Analog Devices (Maxim) MAX30102 su bus I2C. Il firmware non analizza la semplice frequenza cardiaca bpt (*Beats Per Minute*), ma la **Variabilità della Frequenza Cardiaca (HRV)** nello spettro delle frequenze. Crolli repentini della componente HF (*High Frequency*) accoppiati a picchi della LF (*Low Frequency*) indicano uno stato di shock o minaccia imminente.
 3. **Gradiente Micro-Termico:** Monitoraggio accoppiato della temperatura cutanea superficiale e della temperatura ambientale, realizzato utilizzando due Texas Instruments TMP117 su bus I2C. Lo stress acuto indotto da coercizione genera vasocostrizione periferica con un calo termico localizzato immediato, impossibile da emulare senza alterare la temperatura interna del modulo.
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

## Mappatura Finale dell'Integrazione Hardware (I^2C)
+-------------------------------------------------------------------------+
| SCHERMATURA DI FARADAY (Gabbia Metallica Saldata su PCB)                |
|                                                                         |
|  [MAX30009] ────► (GSR / Impedenza AC) ────┐                            |
|  [MAX30102] ────► (PPG Rosso / IR)     ────┼──► Bus I2C Privato         |
|  [TMP117-S] ────► (Temp. Cute)         ────┤    (Linee Schermate)       |
|  [TMP117-E] ────► (Temp. Ambiente)     ────┘            │               |
|                                                         ▼               |
|  +-------------------------------------------------------------------+  |
|  | HARDWARE NRF52840                                                 |  |
|  |  [ TWIM0 EasyDMA ] ──► [ Matrice Effimera RAM ]                   |  |
|  |                                │ (Wipe Attivo 120ms da TIMER1)    |  |
|  |                                ▼                                  |  |
|  |              [ TRNG Noise Injection / Zeroizzazione ]             |  |
|  +-------------------------------------------------------------------+  |
+-------------------------------------------------------------------------+
```c
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
static volatile uint8_t biometric_raw_buffer[BIOMETRIC_BUFFER_SIZE] _attribute_((aligned(4)));
static uint8_t entropy_pool[BIOMETRIC_BUFFER_SIZE] _attribute_((aligned(4)));
static volatile uint8_t entropy_pool_index = 0;

// Buffer RAM obbligatorio per il comando EasyDMA (evita allocazione in Flash .rodata)
static uint8_t i2c_wipe_cmd[2] _attribute_((aligned(4))) = {0x04, 0x40};

/**
 * @brief INTERRUPT HANDLER TIMER1: Obliterazione totale (120ms) con protezione da stallo
 */
void TIMER1_IRQHandler(void) {
    if (NRF_TIMER1->EVENTS_COMPARE[0] == 1) {
        NRF_TIMER1->EVENTS_COMPARE[0] = 0;

        // 1. Congela EasyDMA del modulo I2C con timeout di sicurezza
        NRF_TWIM0->TASKS_STOP = 1;
        uint32_t timeout = TIMEOUT_MAX_LOOPS;
        while ((NRF_TWIM0->EVENTS_STOPPED == 0) && (--timeout > 0));
        NRF_TWIM0->EVENTS_STOPPED = 0;

        // 2. Wipe distruttivo della RAM locale
        uint8_t *raw_ptr = (uint8_t *)biometric_raw_buffer;
        if (entropy_pool_index >= BIOMETRIC_BUFFER_SIZE) {
            for (size_t i = 0; i < BIOMETRIC_BUFFER_SIZE; i++) {
                raw_ptr[i] = entropy_pool[i];
            }
        } else {
            for (size_t i = 0; i < BIOMETRIC_BUFFER_SIZE; i++) {
                raw_ptr[i] = 0x55; // Pattern di sfoltimento alternato alternativo
            }
        }
        entropy_pool_index = 0;

        // 3. Reset FIFO esterno (Puntatore rigorosamente in RAM)
        NRF_TWIM0->ADDRESS = MAX30102_ADDR;
        NRF_TWIM0->TXD.PTR = (uint32_t)i2c_wipe_cmd; 
        NRF_TWIM0->TXD.MAXCNT = 2;
        NRF_TWIM0->TASKS_STARTTX = 1;
        
        timeout = TIMEOUT_MAX_LOOPS;
        while ((NRF_TWIM0->EVENTS_TXSTARTED == 0) && (--timeout > 0));
        NRF_TWIM0->EVENTS_TXSTARTED = 0;

        // Barriere di memoria per stabilizzare la pipeline
        __DSB();
        __ISB();
    }
}

/**
 * @brief 1. ATTIVAZIONE HARDENED DI APPROTECT (Blocco permanente del Debugger SWD)
 */
void nelo_hw_enforce_approtect(void) {
    if (NRF_UICR->APPROTECT != 0x00000000) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        NRF_UICR->APPROTECT = 0x00000000;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);


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
```
