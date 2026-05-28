L'implementazione pratica della `SPEC-008` (il payload AEAD fisso a 96 byte) e della `SPEC-009` (il calcolo dell'entropia e della divergenza) su hardware IoT commerciale e nodi mesh pone sfide ingegneristiche notevoli. Le limitazioni principali dei dispositivi IoT e dei router COTS (*Commodity Off-The-Shelf*) riguardano la **banda passante ristretta**, il **consumo energetico** e la **potenza computazionale limitata**.

Per calare queste specifiche nella realtà, l'architettura deve essere ottimizzata sfruttando al massimo le caratteristiche hardware dei moderni microcontrollori edge e dei microcontroller usati nei router mesh (come i SoC della famiglia Nordic nRF52 o i chip Espressif ESP32).

Ecco come si traduce in pratica questa implementazione, divisa per i due stadi fondamentali:

---

### 1. Implementazione lato Cripto-Sensore (Livello 1 - Edge IoT)

I sensori antropici indossabili sono dispositivi alimentati a batteria (spesso a bottone o mini-LiPo), dove l'efficienza energetica e l'isolamento sono critici.

* **Sfruttamento dei Sottosistemi Hardware (DMA e MPU):**
Il campionamento analogico dei sensori (PPG, GSR) non deve impegnare la CPU principale. Si utilizza il controller **SAADC** (Successive Approximation ADC) accoppiato al **EasyDMA** (Direct Memory Access). I dati biologici passano dai pin analogici direttamente alla `SHADOW_BUFFER` in RAM senza svegliare la CPU. La CPU si attiva tramite interrupt solo ogni 120ms per calcolare l'indice $D$ e impacchettare il frame, riducendo drasticamente i consumi.
* **Accelerazione Crittografica Hardware:**
Eseguire algoritmi complessi come *Ed25519* e *ChaCha20-Poly1305* interamente via software prosciugherebbe una piccola batteria in poche ore. Chip come l'nRF52840 integrano un coprocessore crittografico hardware (es. **ARM TrustZone CryptoCell-310**).
* La cifratura AEAD e la firma asimmetrica vengono delegate interamente a questo modulo hardware dedicato.
* L'operazione richiede pochissimi millisecondi e una frazione minima di corrente, rendendo fattibile il ciclo di trasmissione continuo.


* **Blindatura della Chiave Privata:**
Per impedire che la chiave $SK_{sensor}$ venga estratta tramite la lettura fisica del chip, si configurano i registri di protezione della memoria (**APPROTECT** o i blocchi NVMC). Una volta flashata la chiave in produzione, l'accesso ai canali di debug (J-Link/SWD) viene disattivato permanentemente a livello hardware.

---

### 2. Implementazione lato Nodo di Transito (Livello 2 - Router Mesh)

I router mesh distribuiti sul territorio gestiscono il traffico della maglia radio. Spesso operano con microcontroller economici a basso consumo o piccoli SoC embedded che devono digerire migliaia di pacchetti al secondo senza saturarsi.

* **Esecuzione dello Scarto Balistico (Early Drop):**
Quando il modulo radio riceve il frame da 96 byte, i driver di basso livello analizzano immediatamente i primi 2 byte. Questa operazione viene eseguita direttamente nel buffer di ricezione del chip radio (es. FIFO del modulo LoRa o della periferica BLE/802.15.4), prima ancora di copiare il pacchetto nella memoria di sistema del router. Se il controllo fallisce, il buffer viene sovrascritto, azzerando l'overhead software.
* **Filtro di Bloom in RAM ad Accesso Rapido:**
Per il controllo anti-replay, il `nonce` a 32 bit viene processato attraverso un **Filtro di Bloom** allocato in RAM statica (SRAM). Il Filtro di Bloom utilizza una serie di funzioni di hashing estremamente leggere (come *MurmurHash3*) che richiedono solo operazioni di bit-shifting a livello di CPU. L'interrogazione richiede un tempo costante $O(1)$: il nodo sa istantaneamente se ha già visto quel pacchetto senza dover scorrere liste o database, proteggendo la CPU da attacchi di saturazione.

---

### 3. Implementazione del Consensus Layer (Livello 3 - Validatori del Quorum)

La parte computazionalmente più onerosa è il calcolo della `SPEC-009` (l'Entropia $H$ di Shannon che include calcoli logaritmici e la verifica delle firme Ed25519).

* **Pipelining Asincrono:**
I nodi ordinari *non calcolano* l'entropia. Raccolgono semplicemente i pacchetti e li inoltrano lungo la mesh. Solo i 127 nodi estratti per il Quorum deviano temporaneamente una parte della loro capacità di calcolo per elaborare la matrice di stato.
* **Ottimizzazione della Matematica in Virgola Mobile:**
Molti microcontrollori IoT economici non hanno una FPU (Floating Point Unit) hardware avanzata, e il calcolo di `log2f()` richiesto dall'entropia di Shannon sarebbe troppo lento. In pratica, l'algoritmo viene ottimizzato sostituendo la funzione logaritmica con una **tabella di lookup precalcolata (LUT)** memorizzata nella memoria Flash, oppure approssimando il calcolo tramite espansioni in serie di Taylor a interi o logica a virgola fissa. Questo trasforma un calcolo matematico pesante in una serie di rapide letture di memoria.

---

### Sintesi Pratica dell'Infrastruttura IoT

In conclusione, l'architettura NELO è perfettamente realizzabile con l'hardware IoT attuale se si rispettano queste tre regole operative:

1. **Delegare l'hardware:** Sfruttare i motori crittografici e i moduli DMA integrati nei chip.
2. **Dividere i compiti:** Chi fa viaggiare i dati (i nodi mesh) esegue solo filtri balistici leggeri; chi valida (il Quorum) fa i calcoli complessi, isolando lo stress computazionale.
3. **Semplificare la matematica:** Sostituire le formule floating-point continue con approssimazioni in virgola fissa e tabelle di lookup per non sovraccaricare le CPU edge.
