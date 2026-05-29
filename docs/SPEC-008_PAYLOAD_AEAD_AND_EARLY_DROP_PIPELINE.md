# SPEC-008: Struttura del Payload Binario AEAD (Sub-GHz)

**Stato:** SPEC / ALPHA  
**Versione:** 3.1-HARDENED  
**Target:** Sviluppatori Firmware, Crittografi, Ingegneri di Rete  

---

## 1. Vincoli di Canale e Obiettivi di Efficienza

In condizioni di Stato di Emergenza ($D > 0.7$), il Cripto-Braccialetto Edge deve iniettare informazioni nello spazio aereo Sub-GHz minimizzando l'Airtime (tempo di trasmissione radio). Questo riduce drasticamente l'esposizione del sensore a tentativi di triangolazione goniometrica nemica e massimizza l'indice di penetrazione radio in ambienti ad alta frizione o saturazione elettromagnetica.

* **Dimensione Fissa:** Il pacchetto ha una lunghezza immutabile di **96 Byte (768 bit)**. Non è ammesso alcun tipo di padding dinamico o overhead testuale (No JSON, No Type-Length-Value espanso).
* **Allineamento di Memoria:** La dimensione è un multiplo esatto dei blocchi di cifratura hardware (16 byte) e si adatta nativamente alle code EasyDMA del SoC nRF52840 (sensore) e dell'ESP32-S3 (router), scongiurando la frammentazione della RAM.

---

## 2. Mappatura Bit-a-Bit del Pacchetto

Il payload si compone di una sezione di controllo in chiaro (Header e Nonce) e di un blocco protetto da Cifratura Autenticata con Dati Associati (AEAD - *Authenticated Encryption with Associated Data*).

* In chiaro: I 20 byte iniziali comprendono i metadati, il contatore di sequenza locale, il timestamp balistico e il nonce stocastico.

* Protetti (AEAD + Sig): I restanti 76 byte racchiudono il fattore D in virgola fissa Q1.15, l'entropia di riserva, il tag simmetrico Poly1305 e la firma Ed25519.

<pre>
+-----------+------------+-----------------------------------+------------+
|  HEADER   |   NONCE    |         CIPHERTEXT BODY           |  AUTH TAG  |
|  (4 Byte) | (12 Byte)  |            (64 Byte)              | (16 Byte)  |
+-----------+------------+-----------------------------------+------------+
</pre>

### 2.1. Unencrypted Header [4 Byte] (Byte 0 - 3)
Dati visibili necessari ai Router Alpha di superficie per l'instradamento e il filtraggio rapido dei pacchetti, senza gravare sulla CPU con operazioni di decifratura asimmetrica nei nodi di puro transito.

* **Byte 0: Protocol Version & Flags [8 bit]**
  * `bit 7-4`: Versione del protocollo NELO (corrente: `0x3` per v3.1-HARDENED).
  * `bit 3-0`: Flag operativi di instradamento e priorità di coda.
* **Byte 1-3: Ephemeral Epoch Timestamp [24 bit]**
  * Contatore incrementale basato su finestre temporali stocastiche generate dalla rete. Non esprime l'orario UTC reale (anti-tracciamento), ma serve a determinare la freschezza del pacchetto e a neutralizzare attacchi di replica (*Replay Attacks*).

### 2.2. Cryptographic Nonce / IV [12 Byte] (Byte 4 - 15)
* **Byte 4-15: Vettore di Inizializzazione [96 bit]**
  * Stringa stocastica estratta dal TRNG hardware del bracciale ad ogni singola trasmissione. Garantisce che la cifratura dello stesso stato biologico produca stringhe binarie aeree completamente differenti, impedendo la correlazione statistica da parte di analizzatori di spettro avversari.

### 2.3. Encrypted Payload Body [64 Byte] (Byte 16 - 79)
Il nucleo informativo protetto. I dati interni sono estratti e decifrati esclusivamente dai nodi che possiedono la chiave di sessione o l'autorità di cella.

* **Byte 16-19: Telemetria Biologica Compressa [32 bit]**
  * `bit 31-20`: Indice di Danno puntuale $D$ (risoluzione a 12 bit, scala millesimale $0.000 - 1.000$).
  * `bit 19-10`: Valore HRV (*Heart Rate Variability*) normalizzato su 10 bit.
  * `bit 9-0`: Livello di attivazione GSR (*Galvanic Skin Response*) su 10 bit.
* **Byte 20-31: Georeferenziazione Stocastica d'Area [96 bit]**
  * Coordinate geografiche approssimate caricate dal modulo GPS dello smartphone prima del Kill-Switch radio, mascherate con rumore gaussiano dinamico. Identificano il quadrante urbano di sofferenza escludendo la localizzazione balistica millimetrica del cittadino.
* **Byte 32-79: Firma Asimmetrica della Cella [48 Byte / 384 bit]**
  * Firma crittografica generata tramite chiavi effimere basate su curve ellittiche rapide (Ed25519/Curve25519). Attesta l'integrità del sensore all'interno dell'architettura di voto tramite *Ring Signatures* (firme di gruppo), tutelando l'anonimato dell'hardware mittente.

### 2.4. AEAD Authentication Tag [16 Byte] (Byte 80 - 95)
* **Byte 80-95: Message Authentication Code (MAC) [128 bit]**
  * Il sigillo generato dall'algoritmo di cifratura (AES-128-CCM o ChaCha20-Poly1305). Protegge sia il testo cifrato che l'header in chiaro. Se un solo bit viene alterato da interferenze o tentativi di iniezione malevola, il Router Alpha scarta il pacchetto a livello fisico, azzerando l'avvelenamento delle code.

---

## 3. Parametri di Trasmissione e Airtime Logico

Dati i 96 byte fisici, i parametri di modulazione consigliati per il ricetrasmettitore LoRa SX1262 in modalità emergenza sono:

$$\text{Spreading Factor (SF)} = 7, \quad \text{Bandwidth (BW)} = 125\text{ kHz}, \quad \text{Coding Rate (CR)} = 4/5$$

Con questa configurazione, il tempo di permanenza in aria del pacchetto è inferiore a **150 ms**, garantendo il perfetto bilanciamento tra raggio d'azione urbano (fino a 2-3 km in linea d'aria metropolitana) e invisibilità temporale.

---

## 4. Mappa Binaria del Pacchetto (96 Byte / 固定)

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

### 4.1 Dizionario dei Campi (Data Dictionary)

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

## 5. Algoritmo di Serializzazione ed Enclaving Edge (C-Implementation)

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

## 6. Pipeline di Early Drop a complessità costante $O(1)$ sui Nodi Mesh

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

## 7. Note di Audit Ingegneristico

* **Invarianza Strutturale:** Qualsiasi tentativo software o compilatore di ottimizzare il codice alterando la dimensione della struttura `NeloPacket_t` compromette la compatibilità hardware a livello di ricevitore RF e solleva un'eccezione MPU.
* **Isolamento dei Registri delle Cache:** Al superamento della pipeline di validazione, i registri temporanei della CPU coinvolti nella lettura dei byte in chiaro devono essere lavati con istruzioni di barriera di sincronizzazione dei dati (`__DSB()`, `__ISB()`), estendendo l'oblio hardware definito nella `SPEC-004`.
