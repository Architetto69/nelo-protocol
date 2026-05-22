## SPEC-003: Meccanica dell’Oblio (The "120ms Volatility" v1.1)

## 1. Obiettivo
Eliminare radicalmente alla radice la possibilità di tracciamento individuale, profilazione comportamentale e sorveglianza retroattiva. Il sistema deve operare in uno stato di presente continuo, rendendo impossibile la persistenza della memoria storica dell'identità.

## 2. Il Limite Temporale: 120ms
È stato scelto il limite di 120ms perché è biologicamente inferiore alla soglia di percezione conscia umana e al tempo medio di reazione neuronale. 
*   **Input-to-Void:** Ogni dato identificativo che entra nel buffer del firmware ha una scadenza temporale non negoziabile imposta via hardware.
*   **Stateless Processing:** Il protocollo non mantiene alcuno stato dell'utente; decodifica temporaneamente i dati di telemetria biometrica per trasformarli in vettori di forza anonimi e poi dimentica l'origine.

## 3. Architettura della Memoria Volatile (Double-Buffering)
Il firmware isola i dati sensibili in una partizione di RAM protetta chiamata `SHADOW_BUFFER`, strutturata a doppia banca logica (Ping-Pong).
*   **Hardware-Enforced Switch:** Ogni 120ms un timer hardware dedicato innesca un interrupt asincrono che scambia la banca di memoria attiva.
*   **Anti-Remanence Oblivion:** La banca disattivata viene immediatamente saturata con rumore casuale generato dal TRNG (True Random Number Generator) hardware per eliminare la rimanenza elettrica/magnetica nei transistor, e successivamente azzerata.
*   **No Persistence:** Qualsiasi tentativo di mappare indirizzi della `SHADOW_BUFFER` su memorie Flash o EEPROM non volatili è fisicamente interdetto dalle linee di indirizzamento della scheda.

## 4. Differenziazione dei Dati
Il protocollo attua una separazione netta e irreversibile tra Segnale e Identità:
*   **Il Segnale (Persistente):** L'intensità normalizzata del danno ($D \in [0, 1]$) e i metadati dei pattern di rischio aggregati. Questi dati statistici puri vengono estratti e salvati per alimentare le funzioni di retroazione della SPEC-001.
*   **L'Identità (Effimera):** ID univoci di schede o chip, coordinate geografiche grezze ad alta precisione, pacchetti di autenticazione e impronte biometriche. Questo materiale decade e viene distrutto entro la finestra dei 120ms.

## 5. Implementazione del "Wipe" (Hardware Interrupt Level)
Eseguito nel Secure Enclave tramite Interrupt Service Routine (ISR) agganciata al Timer Hardware di sistema.

```c
#define SHADOW_BUFFER_SIZE  4096

typedef struct {
    volatile uint8_t bank_A[SHADOW_BUFFER_SIZE];
    volatile uint8_t bank_B[SHADOW_BUFFER_SIZE];
    volatile uint8_t *active_write_ptr;
    volatile uint32_t active_bank; 
} shadow_memory_t;

static shadow_memory_t shadow_ram;

void __attribute__((interrupt)) HW_TIMER_120MS_ISR(void) {
    volatile uint8_t *buffer_to_wipe;

    // 1. Scambio atomico delle banche di memoria (Previene Race Conditions)
    if (shadow_ram.active_bank == 0) {
        shadow_ram.active_write_ptr = shadow_ram.bank_B;
        shadow_ram.active_bank = 1;
        buffer_to_wipe = shadow_ram.bank_A;
    } else {
        shadow_ram.active_write_ptr = shadow_ram.bank_A;
        shadow_ram.active_bank = 0;
        buffer_to_wipe = shadow_ram.bank_B;
    }

    // Barriera di sincronizzazione hardware della CPU
    __sync_synchronize();

    // 2. Distruzione della rimanenza tramite TRNG Hardware
    uint32_t *noise_ptr = (uint32_t *)buffer_to_wipe;
    size_t words = SHADOW_BUFFER_SIZE / sizeof(uint32_t);
    for (size_t i = 0; i < words; i++) {
        noise_ptr[i] = generate_hardware_random_u32(); 
    }

    __sync_synchronize();

    // 3. Sanificazione finale (Scrittura protetta da ottimizzazioni del compilatore)
    for (size_t i = 0; i < SHADOW_BUFFER_SIZE; i++) {
        buffer_to_wipe[i] = 0x00;
    }

    // Svuotamento fisico della Cache della CPU (L1/L2)
    flush_cpu_cache_range((void *)buffer_to_wipe, SHADOW_BUFFER_SIZE);
}
```

## 6. Sicurezza: Il "Memory Jail"
​Se il firmware tenta di allocare o copiare dati sensibili contrassegnati al di fuori dello spazio di indirizzamento della SHADOW_BUFFER (es. tentativi occulti di logging o buffer overflow indotti), la MPU (Memory Protection Unit) solleva immediatamente un'eccezione hardware di violazione di accesso. Il sistema entra in Kernel Panic, taglia l'alimentazione ai banchi di RAM e resetta il dispositivo.
### Perché questa specifica è "Hardened"?
- ​Inutilizzabilità Giudiziaria/Inquisitiva: Non esiste alcun registro storico da sequestrare o analizzare. Il sistema è legalmente e tecnicamente incapace di testimoniare o essere violato ex-post, poiché la sua memoria non si estende oltre l'intervallo operativo minimo.
- ​Anonimato dei 127: I nodi estratti per il quorum di validazione (SPEC-002) sono tutelati dall'oblio. Nel momento in cui la firma viene registrata e validata dal collettivo, il collegamento crittografico locale tra l'identità del firmatario e l'azione decade nella SHADOW_BUFFER, lasciando solo l'evidenza numerica del voto superato.

​[SPEC_003: v1.1-HARDENED] [LOG: TRIANGOLO_DI_SICUREZZA_CHIUSO] [STATUS: COMPLETE_INFRASTRUCTURE_SPEC]
