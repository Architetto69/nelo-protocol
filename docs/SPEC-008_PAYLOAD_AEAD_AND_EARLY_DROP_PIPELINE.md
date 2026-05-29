# SPEC-008: Struttura del Payload Binario AEAD e Pipeline di Early Drop

**Stato:** SPEC / ALPHA  
**Versione:** 3.1-HARDENED  
**Target:** Sviluppatori Firmware, Crittografi, Ingegneri di Rete  

---

## 1. Vincoli di Canale e Obiettivi di Efficienza

In condizioni di Stato di Emergenza ($D > 0.7$), il Cripto-Braccialetto Edge deve iniettare informazioni nello spazio aereo Sub-GHz minimizzando l'Airtime (tempo di trasmissione radio). Questo riduce drasticamente l'esposizione del sensore a tentativi di triangolazione goniometrica nemica e massimizza l'indice di penetrazione radio in ambienti ad alta frizione o saturazione elettromagnetica.

* **Dimensione Fissa (固定):** Il pacchetto ha una lunghezza immutabile di **96 Byte (768 bit)**. Non è ammesso alcun tipo di padding dinamico o overhead testuale (No JSON, No Type-Length-Value espanso). L'invarianza della dimensione impedisce l'analisi del traffico basata sulla lunghezza dei frame.
* **Allineamento di Memoria:** La dimensione si adatta nativamente alle code EasyDMA del SoC nRF52840 (sensore) e dell'ESP32-S3 (router), scongiurando la frammentazione della RAM e consentendo allocazioni statiche $O(1)$ a zero allocazione dinamica.

---

## 2. Mappatura Bit-a-Bit del Pacchetto

Il payload si compone di una sezione di controllo e dati associati in chiaro (Autenticati ma non cifrati) e di un blocco protetto da Cifratura Autenticata (AEAD) contenente la firma crittografica del nodo.

<pre>
+------------------------------------+------------+-----------------------------------+
|     AUTHENTICATED DATA (AAD)       |  AUTH TAG  |          ENCRYPTED BODY           |
|            (20 Byte)               | (16 Byte)  |             (64 Byte)             |
+------------------------------------+------------+-----------------------------------+
</pre>

---

## 3. Mappa Binaria del Pacchetto (96 Byte)


<pre>
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Magic Byte  | Protocol Vers |          Sequence No          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Timestamp                           |
|                     (Unix Epoch - 64 bit)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Cryptographic Nonce                      |
|                           (32 bit)                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       Fattore D (Q1.15)       |           Reserved            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         AEAD MAC Tag                          |
|                           (128 bit)                           |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                                                               |
|                     Firma Ed25519 (Payload)                   |
|                           (512 bit)                           |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
</pre>

### 3.1 Dizionario dei Campi (Data Dictionary)

| Offset (Byte) | Dimensione (Byte) | Campo | Tipo | Descrizione |
| --- | --- | --- | --- | --- |
| **0** | 1 | `MAGIC_BYTE` | `uint8_t` | Identificativo fisso del protocollo: `0x4E` (ASCII 'N'). Filtro rapido L2. |
| **1** | 1 | `VERSION` | `uint8_t` | Versione del protocollo corrente. Valore hardware compilato: `0x31`. |
| **2 - 3** | 2 | `SEQ_NO` | `uint16_t` | Numero di sequenza locale del sensore. Incrementato a ogni trasmissione. |
| **4 - 11** | 8 | `TIMESTAMP` | `uint64_t` | Tempo Unix corrente in millisecondi. Validazione della freschezza. |
| **12 - 15** | 4 | `NONCE` | `uint32_t` | Numero casuale monouso estratto dal TRNG hardware. Barriera anti-replay. |
| **16 - 17** | 2 | `D_INDEX` | `uint16_t` | **Fattore D in virgola fissa Q1.15**. `0x0000` = 0.0, `0x7FFF` = 1.0. |
| **18 - 19** | 2 | `RESERVED` | `uint8_t[2]` | Spazio di allineamento, iniettato con entropia TRNG anti-compressione. |
| **20 - 35** | 16 | `AEAD_TAG` | `uint8_t[16]` | Tag Poly1305 generato dalla cifratura simmetrica dei dati associati. |
| **36 - 95** | 64 | `ED25519_SIG` | `uint8_t[64]` | Firma asimmetrica a 512 bit generata dalla chiave $SK_{sensor}$ nell'enclave. |

---

## 4. Algoritmo di Serializzazione ed Enclaving Edge (C-Implementation)

```c
#define PACKET_SIZE 96
#define AAD_SIZE 12
#define PLAINTEXT_SIZE 4

typedef struct __attribute__((packed)) {
    // --- DATI IN CHIARO AUTENTICATI (AAD) ---
    uint8_t  magic_byte;     // Offset 0
    uint8_t  version;        // Offset 1
    uint16_t seq_no;         // Offset 2
    uint32_t timestamp_sec;  // Offset 4 (Finestra temporale ridotta a secondi)
    uint32_t nonce;          // Offset 8
    
    // --- CORPO CIFRATO (Sostituito in-place dopo la cifratura) ---
    uint16_t d_index;        // Offset 12 (Protetto)
    uint8_t  reserved[2];    // Offset 14 (Protetto)
    
    // --- CONTROLLO E FIRMA ---
    uint8_t  aead_tag[16];   // Offset 16
    uint8_t  signature[64];  // Offset 32
} NeloPacket_t;

void build_and_sign_nelo_packet(uint16_t calculated_d, NeloPacket_t *out_packet) {
    static uint16_t local_seq = 0;
    
    out_packet->magic_byte    = 0x4E;
    out_packet->version       = 0x31;
    out_packet->seq_no        = local_seq++;
    out_packet->timestamp_sec = (uint32_t)(hardware_rtc_get_ms() / 1000);
    out_packet->nonce         = hardware_trng_get_u32();
    
    // Dati sensibili posizionati nel blocco plaintext prima della cifratura
    out_packet->d_index       = calculated_d;
    hardware_trng_get_bytes(out_packet->reserved, 2);

    // Cifratura in-place del blocco payload (Offset 12-15) e generazione del Tag
    cryptocell_chacha20_poly1305_encrypt_inplace(
        (uint8_t*)&out_packet->d_index, PLAINTEXT_SIZE,
        (uint8_t*)out_packet, AAD_SIZE,
        out_packet->aead_tag,
        &SK_session_ephemeral
    );

    // Firma finale dell'intera struttura generata (32 byte di dati + tag)
    cryptocell_ed25519_sign(
        (uint8_t*)out_packet, 32, 
        out_packet->signature,                      
        &SK_sensor                             
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
2. **Validazione della Finestra di Tolleranza Temporale:** Il timestamp a 64 bit viene confrontato con l'orologio Real-Time Clock (RTC) interno del nodo. Se la deviazione temporale supera i $\pm 5$ secondi ($\Delta T > 5000\text{ms}$), il pacchetto viene trattato come tentativo di iniezione tardiva o attacco replay da cattura e immediatamente scartato.
3. **Controllo Anti-Replay via Bloom Filter Volatile:** Il `nonce` a 32 bit viene estratto e interrogato in un filtro di Bloom locale strutturato in memoria fissa. Se l'algoritmo rileva che lo stesso identico nonce è già transitato nell'area, l'operazione collide nel filtro in tempo costante $O(1)$. Il pacchetto duplicato viene cancellato senza eseguire alcuna operazione crittografica asimmetrica, azzerando l'impatto computazionale del DoS.

---

## 6. Parametri di Trasmissione e Airtime Logico

Dati i 96 byte fisici, i parametri di modulazione consigliati per il ricetrasmettitore LoRa SX1262 in modalità emergenza sono:

$$\text{Spreading Factor (SF)} = 7, \quad \text{Bandwidth (BW)} = 125\text{ kHz}, \quad \text{Coding Rate (CR)} = 4/5$$

Con questa configurazione, il tempo di permanenza in aria del pacchetto è inferiore a **150 ms**, garantendo il perfetto bilanciamento tra raggio d'azione urbano (fino a 2-3 km in linea d'aria metropolitana) e invisibilità temporale.
