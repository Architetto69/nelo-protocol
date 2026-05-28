# SPEC-008: Specifica Formale del Payload AEAD e Pipeline di Early Drop

**Versione:** 3.1-HARDENED

**Stato:** APPROVATO / INGEGNERIZZAZIONE DI BASSO LIVELLO

**Riferimenti di Sistema:** `SPEC-003_OBLIO`, `SPEC-004_SENSORI`, `SPEC-005_NODI`, `SPEC-006_THREAT_MODELING`

---

## 1. Obiettivo Sperimentale e Sicurezza Topologica

Nelle reti mesh operanti in bande Sub-GHz (LoRa/802.15.4), la Maximum Transmission Unit (**MTU**) è strutturalmente limitata. Per garantire la massima penetrazione radio in ambienti urbani ostili, saturazione dello spettro o scenari di jamming attivo, il payload utile deve essere ridotto al minimo e avere una dimensione rigorosamente fissa.

Questa specifica definisce l'architettura binaria del pacchetto radio a **96 Byte (768 Bit)** del Protocollo NELO. La fissità dimensionale assoluta è una contromisura crittografica nativa: impedisce agli analizzatori di spettro avversari di eseguire attacchi di tipo *Traffic Size Analysis* (dedurre le variazioni del fattore di danno $D$ in base alla lunghezza del frame).

---

## 2. Mappa Binaria del Pacchetto (96 Byte / 固定)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Magic Byte  | Protocol Vers |          Sequence No          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Timestamp                           |
|                       (Unix Epoch - 64 bit)                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Cryptographic Nonce                     |
|                            (32 bit)                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       Fattore D (Q1.15)       |          Reserved             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                         AEAD MAC Tag                          +
|                           (128 bit)                           |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                     Firma Ed25519 (Sig)                       +
|                           (512 bit)                           |
+                                                               +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

### 2.1 Dizionario dei Campi (Data Dictionary)

| Offset (Byte) | Dimensione (Byte) | Campo | Tipo | Descrizione |
| --- | --- | --- | --- | --- |
| **0** | 1 | `MAGIC_BYTE` | `uint8_t` | Identificativo fisso del protocollo: `0x4E` (carattere ASCII 'N'). Filtro rapido di livello 2. |
| **1** | 1 | `VERSION` | `uint8_t` | Versione del protocollo corrente. Valore hardware compilato: `0x31`. |
| **2 - 3** | 2 | `SEQ_NO` | `uint16_t` | Numero di sequenza locale del sensore. Incrementato rigidamente a ogni trasmissione. |
| **4 - 11** | 8 | `TIMESTAMP` | `uint64_t` | Tempo Unix corrente espresso in millisecondi. Validazione della freschezza temporale. |
| **12 - 15** | 4 | `NONCE` | `uint32_t` | Numero casuale monouso estratto dal TRNG hardware. Barriera anti-replay. |
| **16 - 17** | 2 | `D_INDEX` | `uint16_t` | **Fattore D normalizzato in virgola fissa Q1.15**. `0x0000` = 0.0, `0x7FFF` = 1.0. |
| **18 - 19** | 2 | `RESERVED` | `uint8_t[2]` | Spazio di allineamento a 32 bit, riempito con entropia casuale (TRNG) per prevenire attacchi di compressione. |
| **20 - 35** | 16 | `AEAD_TAG` | `uint8_t[16]` | Tag di autenticazione Poly1305 generato dalla cifratura simmetrica del payload. |
| **36 - 95** | 64 | `ED25519_SIG` | `uint8_t[64]` | Firma asimmetrica a 512 bit generata dalla chiave $SK_{sensor}$ blindata nell'enclave. |

---

## 3. Algoritmo di Serializzazione ed Enclaving Edge (C-Implementation)

All'attivazione dell'interrupt ciclico di campionamento (120ms, vedi `SPEC-003`), la routine estrae i dati biometrici immagazzinati nella `SHADOW_BUFFER`, calcola l'indice logico in virgola fissa e genera il pacchetto all'interno dell'area di memoria isolata protetta da MPU.

```c
#include <stdint.h>
#include <string.h>

#define PACKET_SIZE 96
#define AUTH_DATA_SIZE 20

typedef struct __attribute__((packed)) {
    uint8_t  magic_byte;     // Offset 0
    uint8_t  version;        // Offset 1
    uint16_t seq_no;         // Offset 2
    uint64_t timestamp;      // Offset 4
    uint32_t nonce;          // Offset 12
    uint16_t d_index;        // Offset 16
    uint8_t  reserved[2];    // Offset 18
    uint8_t  aead_tag[16];   // Offset 20
    uint8_t  signature[64];  // Offset 36
} NeloPacket_t;

void build_and_sign_nelo_packet(uint16_t calculated_d, NeloPacket_t *out_packet) {
    static uint16_t local_seq = 0;
    
    // 1. Iniezione metadati strutturali in chiaro
    out_packet->magic_byte = 0x4E;
    out_packet->version    = 0x31;
    out_packet->seq_no     = local_seq++;
    out_packet->timestamp  = hardware_rtc_get_ms();
    out_packet->nonce      = hardware_trng_get_u32();
    out_packet->d_index    = calculated_d; 
    
    // Iniezione di rumore nello spazio riservato per spezzare l'euristica dei dizionari di compressione
    hardware_trng_get_bytes(out_packet->reserved, 2);

    // 2. Cifratura Autenticata (AEAD) via Hardware Crypto Engine
    // Generazione del Tag Poly1305 per i primi 20 byte (Data Block)
    cryptocell_chacha20_poly1305_encrypt(
        (uint8_t*)out_packet, AUTH_DATA_SIZE,   
        out_packet->aead_tag,                  
        &SK_session_ephemeral                  // Chiave di sessione effimera rimescolata
    );

    // 3. Firma Asimmetrica Deterministica (Ed25519)
    // Firma dell'intero blocco protetto fino al tag incluso (36 byte complessivi)
    cryptocell_ed25519_sign(
        (uint8_t*)out_packet, AUTH_DATA_SIZE + 16, 
        out_packet->signature,                     
        &SK_sensor                                 // Chiave privata di nodo protetta da APPROTECT
    );
}

```

---

## 4. Pipeline di Early Drop a complessità costante $O(1)$ sui Nodi Mesh

I nodi di transito mesh (COTS) sono considerati elementi sacrificabili e operano in condizioni di alimentazione critica (`SPEC-005`). Per impedire che attacchi da saturazione radio o iniezione di pacchetti fake (DDoS Computazionale) svuotino le batterie dei router obbligandoli a calcolare firme crittografiche massive, ogni nodo implementa una pipeline di scarto istantaneo a complessità costante.

```
             [ Ricezione Pacchetto 96 Byte via Radio ]
                                │
                                ▼
         [ Verifica Sintattica: Magic Byte & Version == 0x31 ] ──NO──► [ DROP ISTANTANEO ]
                                │ SI
                                ▼
         [ Verifica Temporale: Delta-T Freshness <= 5000ms ]   ──NO──► [ DROP ISTANTANEO ]
                                │ SI
                                ▼
         [ Interrogazione Filtro di Bloom: Nonce Esistente? ]   ──SI──► [ DROP ISTANTANEO ]
                                │ NO
                                ▼
             [ Iniezione in Coda di Transito Mesh ]
          (Pronto per la verifica asincrona del Quorum)

```

1. **Scarto Sintattico Balistico (Livello 2):** Il router esamina i primi 2 byte del frame prima di allocare memoria interna. Se `magic_byte != 0x4E` o `version != 0x31`, il pacchetto viene eliminato entro 2 cicli di clock della CPU, interrompendo la catena di elaborazione.
2. **Validazione della Finestra di Tolleranza Temporale:** Il timestamp a 64 bit viene confrontato con l'orologio Real-Time Clock (RTC) interno del nodo, parzialmente sincronizzato in modo pseudo-stocastico dai transiti di rete validi. Se la deviazione temporale supera i $\pm 5$ secondi ($\Delta T > 5000\text{ms}$), il pacchetto viene trattato come tentativo di iniezione tardiva o attacco replay da cattura e immediatamente scartato.
3. **Controllo Anti-Replay via Bloom Filter Volatile:** Il `nonce` a 32 bit viene estratto e iniettato in un filtro di Bloom locale strutturato in memoria fissa. Se l'algoritmo rileva che lo stesso identico nonce è già transitato nell'area, l'operazione collide nel filtro in tempo costante $O(1)$. Il pacchetto duplicato viene cancellato senza eseguire alcuna operazione crittografica asimmetrica di decodifica della firma Ed25519, azzerando l'impatto computazionale del DoS.

---

## 5. Note di Audit Ingegneristico

* **Invarianza Strutturale:** Qualsiasi tentativo software o compilatore di ottimizzare il codice alterando la dimensione della struttura `NeloPacket_t` compromette la compatibilità hardware a livello di ricevitore RF e solleva un'eccezione MPU.
* **Isolamento dei Registri delle Cache:** Al superamento della pipeline di validazione, i registri temporanei della CPU coinvolti nella lettura dei byte in chiaro devono essere lavati con istruzioni di barriera di sincronizzazione dei dati (`__DSB()`, `__ISB()`), estendendo l'oblio hardware definito nella `SPEC-004`.
