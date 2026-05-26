
# NELO Protocol Specification
## SPEC-010: Architettura del Firmware dei Router Mesh e Gestione dello Stato Volatile
**Stato:** DRAFT / HARDENED  
**Versione:** 3.1  
**Autore:** Architetto69 Autenticato
**Data:** 26 Maggio 2026  

---

### 1. Obiettivo e Modello di Minaccia

La presente specifica definisce l'architettura del firmware e i vincoli di gestione della memoria per i **Router Mesh di Superficie (Nodi di Transito)** all'interno dell'ecosistema NELO. 

Il modello di minaccia assume che qualsiasi nodo sul territorio possa subire:
1. **Sequestro fisico** e successiva analisi forense della memoria persistente (NAND/NOR Flash).
2. **Iniezione di firmware malevolo** tramite tentativi di exploit OTA (Over-The-Air).
3. **Analisi dei canali laterali (Side-Channel)** per l'estrazione di chiavi di cifratura di transito.

Per neutralizzare queste minacce, la `SPEC-010` impone il paradigma dell'**Assenza Totale di Stato Persistente (Stateless Architecture)**. Ciò che non esiste nel mondo fisico non può essere sequestrato.

---

### 2. Layout della Memoria e Ciclo di Vita del Boot

I router basati su hardware COTS (es. architetture ESP32-S3 o Nordic nRF52840 accoppiate a transceiver LoRa SX1262) devono implementare un partizionamento rigido della memoria che esclude qualsiasi scrittura a runtime sui supporti non volatili.

```c
+-----------------------------------------------------------------------+
|                       NOR/NAND FLASH (Sola Lettura)                   |
+-----------------------------------------------------------------------+
|  [Bootloader Protetto] -> [Slot A: FW Immutabile] -> [Slot B: Backup] |
+-----------------------------------------------------------------------+
|
                       (Esecuzione e Decompressione)
                                   |
                                   v
+-----------------------------------------------------------------------+
|                            RAM VOLATILE                               |
+-----------------------------------------------------------------------+
|  [Kernel/RTOS Space] | [Buffer Circolare Cifrato] | [Heap Temporaneo] |
+-----------------------------------------------------------------------+

```

#### 2.1 Fasi del Boot Strapping
1. **Verifica della Firma (Secure Boot):** Il bootloader, protetto da scrittura tramite fusione dei fusibili hardware (*eFuses*), verifica l'integrità del firmware nello *Slot A* tramite firma asimmetrica Ed25519.
2. **Caricamento in RAM:** L'immagine del firmware viene decompressa ed eseguita interamente nello spazio di indirizzamento della memoria RAM volatile.
3. **Isolamento dei Driver Flash:** Subito dopo il boot, i driver di scrittura sulla memoria Flash non volatile vengono disabilitati a livello di kernel. Qualsiasi chiamata di sistema verso partizioni non volatili genera un `Kernel Panic` e l'arresto immediato del dispositivo.

---

### 3. Gestione dei Dati di Transito: Il Buffer Circolare Volatile

I router mesh agiscono esclusivamente come relè. È tassativamente vietata la persistenza dei pacchetti AEAD da 96 byte provenienti dai cripto-braccialetti edge.

#### 3.1 Struttura della Coda in Memoria (`SHADOW_QUEUE`)
I pacchetti in transito vengono allocati in un buffer circolare statico pre-allocato in RAM all'avvio del sistema, per prevenire la frammentazione della memoria e attacchi di tipo *Heap Spraying*.

* Ogno slot del buffer ha una dimensione fissa di 128 byte (struttura dati ottimizzata per l'allineamento dei blocchi di memoria).
* Il tempo massimo di permanenza (TTL - Time to Live) di un pacchetto nella `SHADOW_QUEUE` è limitato a **120 millisecondi**, indipendentemente dall'esito della ritrasmissione.

#### 3.2 Routine di Bonifica (*Oblio Istantaneo*)
Non appena il puntatore della coda avanza (perché il pacchetto è stato inoltrato o il TTL è scaduto), il firmware esegue la funzione `nelo_memclear()`. 

La bonifica non si limita a liberare il puntatore, ma sovrascrive attivamente l'area di memoria utilizzando una maschera XOR generata dal generatore di entropia hardware (TRNG) del SoC:

$$\text{MemoriaOblio} = \text{DatiVecchi} \oplus \text{TrngNoise}$$

Questo processo distrugge qualsiasi traccia residua di carica capacitiva nelle celle della RAM, rendendo inefficaci gli attacchi di tipo *Cold-Boot* (congelamento della RAM per l'estrazione dei dati post-spegnimento).

---

### 4. Zero Logging Policy

Il firmware deve essere compilato escludendo qualsiasi modulo di tracciamento o diagnostica persistente.

* **Nessun File di Registro:** Le macro di debug (`printk`, `printf`, `LOG_INF`) devono essere rimosse in fase di compilazione tramite flag del preprocessore (`-DNELO_RELEASE`).
* **Interfaccia UART Disabilitata:** I pin fisici della porta seriale di debug sul PCB devono essere disattivati via software dopo la fase di inizializzazione per impedire l'aggancio di analizzatori logici esterni.
* **Anonimato dei Nodi Vicini:** Le tabelle di routing dinamico memorizzano solo l'identificativo effimero a corto raggio (HMAC mobile) dei router adiacenti. Nessun indirizzo MAC statico o coordinata geografica permanente può essere archiviata nella memoria volatile del nodo.

---

### 5. Clausola di Autodistruzione Logica (Anti-Tamper)

Ogni router mesh implementa una routine di protezione in retroazione per reagire ai tentativi di manomissione fisica della scocca o di analisi termica.

* **Rilevamento Caduta di Tensione / Riconfigurazione:** Se i pin di monitoraggio hardware (GPIO dedicati accoppiati a un micro-interruttore normalmente chiuso nella scocca del router) rilevano l'apertura del contenitore, viene generato un interrupt a priorità assoluta (NMI).
* **Procedura di Emergenza:**
  1. Arresto immediato di tutte le trasmissioni radio Sub-GHz per evitare l'esfiltrazione di pacchetti corrotti.
  2. Esecuzione di un ciclo di sovrascrittura distruttiva di tutta la RAM tramite loop di rumore TRNG.
  3. Reset forzato del SoC con blocco del bootloader fino al ciclo di alimentazione successivo.

---

### 6. Tabella di Conformità Hardware per i Maker

Per i validatori che assemblano i kit in autonomia nella **Fase 1 (Seed Alpha)**, il software applicherà i seguenti vincoli di configurazione per garantire la validità della specifica:

| Parametro Firmware | Valore di Specifica | Vincolo di Sicurezza |
| :--- | :--- | :--- |
| **Storage Allocations** | 0 KB su Flash a Runtime | Immutabilità del sistema |
| **RAM Retention Time** | Max 120ms per pacchetto | Prevenzione indagini forensi |
| **Encryption Mode** | Pass-through AEAD Poly1305 | Nessuna decifrazione locale |
| **Debug Interfaces** | JTAG/UART disabilitate | Blocco degli attacchi hardware |

```
