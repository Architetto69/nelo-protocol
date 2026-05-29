# SPEC-008: Struttura del Payload Binario AEAD e Pipeline di Early Drop

**Stato:** SPEC / ALPHA  
**Versione:** 3.1-HARDENED  
**Target:** Sviluppatori Firmware, Crittografi, Ingegneri di Rete  

---

## 1. Vincoli di Canale e Obiettivi di Efficienza

In condizioni di Stato di Emergenza ($D > 0.7$), il Cripto-Braccialetto Edge deve iniettare informazioni nello spazio aereo Sub-GHz minimizzando l'Airtime (tempo di trasmissione radio). Questo riduce l'esposizione del sensore a tentativi di triangolazione goniometrica nemica e massimizza l'indice di penetrazione radio in ambienti saturi.

* **Dimensione Fissa (固定):** Il pacchetto ha una lunghezza immutabile di **96 Byte (768 bit)**. Non è ammesso alcun tipo di padding dinamico o overhead testuale (No JSON, No TLV espanso). L'invarianza della dimensione impedisce l'analisi del traffico basata sulla lunghezza dei frame.
* **Allineamento di Memoria:** La dimensione si adatta alle code EasyDMA del SoC nRF52840 (sensore) e dell'ESP32-S3 (router), consentendo allocazioni statiche e deterministiche a complessità $O(1)$.

---

## 2. Mappatura dei Macro-Blocchi

Il payload applica lo schema **Encrypt-then-Sign** per garantire la riservatezza dei dati biologici ed evitare che nodi intermedi debbano decifrare il pacchetto per verificarne l'autenticità.

<pre>
+------------------------------------+-----------------------------------+-----------------------------------+
|     AUTHENTICATED DATA (AAD)       |        ENCRYPTED CIPHERTEXT       |             AEAD TAG              |
|        In Chiaro (12 Byte)         |         Protetto (4 Byte)         |             (16 Byte)             |
+------------------------------------+-----------------------------------+-----------------------------------+
|                                                                                                            |
|◄────────────────────────────────────────────── 32 BYTE FIRMATI ───────────────────────────────────────────►|
|                                                                                                            |
+------------------------------------------------------------------------------------------------------------+
|                                           FIRMA DETERMINISTICA Ed25519                                     |
|                                                    (64 Byte)                                               |
+------------------------------------------------------------------------------------------------------------+
</pre>

---

## 3. Mappa Binaria del Pacchetto (96 Byte)


```

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Magic Byte  | Protocol Vers |          Sequence No          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Timestamp Sec                         |
|                     (Unix Epoch - 32 bit)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Cryptographic Nonce                      |
|                           (32 bit)                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       Fattore D (Q1.15)       |           Reserved            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         AEAD MAC Tag                          |
!                                                               |
|                           (128 bit)                           |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Firma Ed25519 (Payload)                   |
|                                                               |
|                           (512 bit)                           |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

### 3.1 Dizionario dei Campi (Data Dictionary)

| Offset (Byte) | Dimensione (Byte) | Campo | Tipo | Descrizione |
| --- | --- | --- | --- | --- |
| **0** | 1 | `MAGIC_BYTE` | `uint8_t` | Identificativo fisso del protocollo: `0x4E` (ASCII 'N'). Filtro rapido L2. |
| **1** | 1 | `VERSION` | `uint8_t` | Versione del protocollo corrente. Valore hardware compilato: `0x31`. |
| **2 - 3** | 2 | `SEQ_NO` | `uint16_t` | Numero di sequenza locale del sensore. Incrementato a ogni trasmissione. |
| **4 - 7** | 4 | `TIMESTAMP_SEC` | `uint32_t` | Tempo Unix espresso in secondi. Validazione della freschezza temporale. |
| **8 - 11** | 4 | `NONCE` | `uint32_t` | Numero casuale monouso estratto dal TRNG hardware. Barriera anti-replay. |
| **12 - 13** | 2 | `D_INDEX` | `uint16_t` | **Fattore D Cifrante in virgola fissa Q1.15**. `0x0000` = 0.0, `0x7FFF` = 1.0. |
| **14 - 15** | 2 | `RESERVED` | `uint8_t[2]` | Spazio cifrato, iniettato con entropia TRNG per spezzare dizionari euristici. |
| **16 - 31** | 16 | `AEAD_TAG` | `uint8_t[16]` | Tag Poly1305 generato dalla cifratura simmetrica in-place del blocco payload. |
| **32 - 95** | 64 | `ED25519_SIG` | `uint8_t[64]` | Firma asimmetrica a 512 bit calcolata sui primi 32 byte complessivi del frame. |

---

## 4. Algoritmo di Serializzazione ed Enclaving Edge (C-Implementation)

```c
#include <stdint.h>
#include <string.h>

#define PACKET_SIZE 96
#define AAD_SIZE 12         // Offset 0-11: Metadati in chiaro autenticati
#define PLAINTEXT_SIZE 4    // Offset 12-15: Area sensibile da cifrare (d_index + reserved)

typedef struct __attribute__((packed)) {
    // --- DATI IN CHIARO AUTENTICATI (AAD) ---
    uint8_t  magic_byte;     // Offset 0
    uint8_t  version;        // Offset 1
    uint16_t seq_no;         // Offset 2
    uint32_t timestamp_sec;  // Offset 4
    uint32_t nonce;          // Offset 8
    
    // --- CORPO CIFRATO (Sostituito in-place dall'unita crittografica) ---
    uint16_t d_index;        // Offset 12
    uint8_t  reserved[2];    // Offset 14
    
    // --- CONTROLLO E MUTUA AUTENTICAZIONE ---
    uint8_t  aead_tag[16];   // Offset 16
    uint8_t  signature[64];  // Offset 32
} NeloPacket_t;

void build_and_sign_nelo_packet(uint16_t calculated_d, NeloPacket_t *out_packet) {
    static uint16_t local_seq = 0;
    
    // 1. Iniezione metadati strutturali in chiaro (Data Block)
    out_packet->magic_byte    = 0x4E;
    out_packet->version       = 0x31;
    out_packet->seq_no        = local_seq++;
    out_packet->timestamp_sec = (uint32_t)(hardware_rtc_get_ms() / 1000);
    out_packet->nonce         = hardware_trng_get_u32();
    
    // 2. Allineamento dei dati sensibili nel blocco Plaintext prima della cifratura
    out_packet->d_index       = calculated_d;
    hardware_trng_get_bytes(out_packet->reserved, 2);

    // 3. Cifratura Autenticata In-Place (AEAD) via Hardware Crypto Engine
    // Cifra d_index e reserved, generando il Tag Poly1305 memorizzato in aead_tag
    cryptocell_chacha20_poly1305_encrypt_inplace(
        (uint8_t*)&out_packet->d_index, PLAINTEXT_SIZE,
        (uint8_t*)out_packet, AAD_SIZE,
        out_packet->aead_tag,
        &SK_session_ephemeral
    );

    // 4. Firma Asimmetrica Deterministica (Ed25519)
    // Firma l'intero blocco validato pre-esistente (AAD + Ciphertext + TAG = 32 byte)
    cryptocell_ed25519_sign(
        (uint8_t*)out_packet, 32, 
        out_packet->signature,                      
        &SK_sensor                             // Chiave protetta da APPROTECT
    );
}

```

---

## 5. Pipeline di Early Drop a complessità costante $O(1)$ sui Nodi Mesh

I nodi di transito mesh (COTS) operano in condizioni di alimentazione critica (`SPEC-005`). Per impedire che attacchi da saturazione radio o iniezione di pacchetti fake (DDoS Computazionale) svuotino le batterie dei router, ogni nodo implementa una pipeline di scarto istantaneo a complessità costante $O(1)$.

```
     [ Ricezione Pacchetto 96 Byte via Radio ]
                       │
                       ▼
     [ Verifica Sintattica: Magic Byte & Version == 0x31 ] ──NO──► [ DROP ISTANTANEO ]
                       │ SI
                       ▼
     [ Verifica Temporale: Delta-T Freshness <= 5s ]       ──NO──► [ DROP ISTANTANEO ]
                       │ SI
                       ▼
     [ Interrogazione Filtro di Bloom: Nonce Esistente? ]   ──SI──► [ DROP ISTANTANEO ]
                       │ NO
                       ▼
             [ Iniezione in Coda di Transito Mesh ]
          (Pronto per la verifica asincrona del Quorum)

```

1. **Scarto Sintattico Balistico (Livello 2):** Il router esamina i primi 2 byte del frame prima di allocare memoria interna. Se `magic_byte != 0x4E` o `version != 0x31`, il pacchetto viene eliminato entro 2 cicli di clock della CPU, interrompendo la catena di elaborazione.
2. **Validazione della Finestra di Tolleranza Temporale:** Il timestamp a 32 bit viene confrontato con l'orologio Real-Time Clock (RTC) interno del nodo. Se la deviazione temporale supera i $\pm 5$ secondi ($\Delta T > 5\text{s}$), il pacchetto viene trattato come tentativo di iniezione tardiva o attacco replay da cattura e immediatamente scartato.
3. **Controllo Anti-Replay via Bloom Filter Volatile:** Il `nonce` a 32 bit viene estratto e interrogato in un filtro di Bloom locale strutturato in memoria fissa. Se l'algoritmo rileva che lo stesso identico nonce è già transitato nell'area, l'operazione collide nel filtro in tempo costante $O(1)$. Il pacchetto duplicato viene cancellato senza eseguire alcuna operazione crittografica asimmetrica, azzerando l'impatto computazionale del DoS.

---

## 6. Parametri di Trasmissione e Airtime Logico

Dati i 96 byte fisici, i parametri di modulazione consigliati per il ricetrasmettitore LoRa SX1262 in modalità emergenza sono:

$$\text{Spreading Factor (SF)} = 7, \quad \text{Bandwidth (BW)} = 125\text{ kHz}, \quad \text{Coding Rate (CR)} = 4/5$$

Con questa configurazione, il tempo di permanenza in aria del pacchetto è inferiore a **150 ms**, garantendo il perfetto bilanciamento tra raggio d'azione urbano (fino a 2-3 km in linea d'aria metropolitana) e invisibilità temporale.
