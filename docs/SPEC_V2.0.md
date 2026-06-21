# **ARCHITETTURA DEL PROTOCOLLO NELO \- SPEC V2.0**

*Versione: 2.0 (Evolution) | Status: Operational Friction | Mode: Systemic Inertia*

## **1\. Evoluzione Architetturale**

Il protocollo NELO evolve dalla versione 1.0 (Mesh pura) alla **versione 2.0 (Hybrid Gateway-Stateless)**. Il sensore (L1) non agisce più come nodo terminale autonomo di iniezione, ma come sorgente di dati firmati che necessitano del contesto mobile (Gateway) per completare la catena di autenticazione.

### Architettura del Protocollo NELO - Schema Concettuale aggiornato
```mermaid
graph TB
    %% Livello 1: Cripto-Sensori Antropici (Calcolo Locale e Blindato)
    subgraph L1 [Livello 1 - Cripto-Sensori Antropici Hardened]
        subgraph S1 [Sensore Antropico 1]
            B1[Bio-Readings: PPG / GSR / TMP117] -->|120ms Volatility| MB1[(SHADOW_BUFFER RAM)]
            MB1 -->|Q16.16 Math| RL1[Risk Engine: Calcolo D]
            RL1 -->|Payload AEAD + SK_sensor| TX1[Modulo Radio Push]
        end
        subgraph S2 [Sensore Antropico N...]
            B2[Bio-Readings] -->|120ms Volatility| MB2[(SHADOW_BUFFER RAM)]
            MB2 -->|Q16.16 Math| RL2[Risk Engine: Calcolo D]
            RL2 -->|Payload AEAD + SK_sensor| TX2[Modulo Radio Push]
        end
    end

    %% Inserimento App Gateway
    subgraph G1 [Livello 1.5 - Smartphone Gateway]
        APP[App NELO - Gateway Stateless]
    end

    %% Flusso aggiornato
      TX1 -->|Segmento S: Dati Firmati| APP
      TX2 -->|Segmento S: Dati Firmati| APP
      
      %% Iniezione nella Mesh (ora mediata dall'App)
      APP ==>|Segmento S+A: Contesto Firmato| N1
      APP ==>|Segmento S+A: Contesto Firmato| N2

    %% Livello 3: Strati Consensuali e di Interdizione Distribuiti
    subgraph L3 [Livello 3 - Strati di Rete Distribuiti]
        CL[Consensus Layer<br/>Quorum Bizantino 85/127]
        IL[Interdiction Layer<br/>Backoff Logaritmico Log D]
        AL[Audit Layer<br/>Registro Immutabile e-Fuse]
    end

    %% Flusso Unidirezionale dei Dati
    TX1 ==>|Iniezione Asincrona D| N1
    TX2 ==>|Iniezione Asincrona D| N2

    %% Elaborazione e Consenso di Rete
    N1 --> CL
    N2 --> CL
    N3 --> CL
    
    CL -->|Consenso Raggiunto| IL
    IL -->|Tracciamento Anonimo| AL

    %% Applicazione della Frizione Meccanica (Feedback sulla Rete)
    IL -.->|Iniezione Latenza Adattiva L_MAX| N1
    IL -.->|Iniezione Latenza Adattiva L_MAX| N2
    IL -.->|Iniezione Latenza Adattiva L_MAX| N3

    %% Stili Rigorosi
    classDef sensor fill:#e1f5fe,stroke:#01579b,stroke-width:2px,color:#000
    classDef buffer fill:#ffe0b2,stroke:#e65100,stroke-width:1px,color:#000
    classDef node fill:#f3e5f5,stroke:#4a148c,stroke-width:2px,color:#000
    classDef layer fill:#e8f5e8,stroke:#1b5e20,stroke-width:2px,color:#000

    class S1,S2 sensor
    class MB1,MB2 buffer
    class N1,N2,N3 node
    class CL,IL,AL layer
```
## Spiegazione dello Schema

### 1. Rilevamento e Calcolo Locale (Livello 1)
 * *Isolamento Radicale:* I dati biologici grezzi (conduttanza cutanea, HRV, temperatura in centesimi di grado) entrano nella SHADOW_BUFFER volatile.
 * *Esecuzione del Risk Engine:* La funzione nelo_calculate_d_index elabora le metriche internamente al chip. L'Indice di Danno D viene generato prima che il pacchetto tocchi l'antenna.
 * *Oblio Immediato:* Scaduto il timer hardware di 120ms, la memoria RAM viene sovrascritta con rumore casuale TRNG. La rete esterna non conoscerà mai i dati medici dell'individuo, ma solo il fattore di danno risultante D.
### 2. Instradamento COTS e Gateway Mobile (Livello 2)
* **Gateway Mobile Stateless:** Il Gateway (Smartphone/App) agisce come punto di aggregazione del contesto. Riceve il *Segmento S* (Dati Sensore) e vi appone il *Segmento A* (Contesto Operativo), firmando crittograficamente l'intero blocco prima dell'inoltro alla rete Mesh.
* **Involucri Anonimi:** I nodi mesh ricevono un payload "a doppia firma". Il primo livello (Sensore) garantisce l'integrità del dato biometrico, il secondo livello (Gateway) garantisce l'autenticità del contesto di transito.
* **Impossibilità di Manipolazione:** I nodi mesh operano esclusivamente come router *stateless* di transito. Non disponendo delle chiavi di firma né del sensore (SK_sensor) né del Gateway, qualsiasi tentativo di alterazione del valore *D* o del contesto comporterebbe l'immediata invalidazione del pacchetto, che verrebbe scartato dal *Consensus Layer*. La persistenza è nulla (Read-Only RootFS), eliminando ogni traccia del transito.
### 3. Strati di Rete e Applicazione della Frizione (Livello 3)
 * *Consensus Layer:* Quando i pacchetti viaggiano nella mesh, il protocollo estrae in modo probabilistico un pool di 127 nodi per validare l'integrità dell'evento tramite quorum bizantino (\ge 85 firme oneste).
 * *Interdiction Layer (Il vero bersaglio della Frizione):* Una volta validato un valore di D > 0.05, l'algoritmo di Backoff Logaritmico Adattivo calcola la latenza L(D). Questa latenza viene iniettata *all'interno delle code di instradamento dei Nodi Mesh (Livello 2)*. La rete rallenta se stessa, espandendo i tempi di trasmissione fino a L_MAX.
 * *Audit Layer:* Le transizioni critiche approvate dal quorum sbloccano la scrittura fisica su e-Fuse o registri non volatili immutabili distribuiti, creando una traccia crittografica dell'interdizione che esclude qualsiasi dato tracciabile o identità persistente.

## **2\. Payload di Comunicazione (Payload AEAD V2)**

Il pacchetto è ora suddiviso in due segmenti interdipendenti, garantendo una firma a due livelli:

### **2.1 Segmento S (Sensore \- Source)**

Prodotto dal firmware (nRF52840) e firmato tramite *CryptoCell-310* (Ed25519).

* **Dimensione:** 83 byte (fisso).  
* **Struttura:** \[timestamp (4b)\]\[damage\_index (2b)\]\[temperature (2b)\]\[bpm (1b)\]\[spo2 (1b)\]\[battery (1b)\]\[eda\_data (8b)\]\[signature\_hw (64b)\].

### **2.2 Segmento A (App \- Contextual)**

Aggiunto dal Gateway Mobile (App) prima dell'inoltro alla rete Mesh.

* **gw\_node\_id:** UUID univoco del dispositivo mobile.  
* **context\_flags:** Bitmask (WiFi, Roaming, Cell, Location\_Precision).  
* **app\_signature:** Firma Ed25519 del blocco concatenato \[Segmento S \+ Segmento A\].

## **3\. Schema di Flusso Aggiornato (V2.0)**

```mermaid
graph TD  
    S1[Sensore nRF52840] -->|Segmento S: Dati Firmati| G[Smartphone Gateway App]
    G -->|Segmento S + A: Contesto Firmato| M[Rete Mesh COTS]
    M -->|Validazione| CL[Consensus Layer]
    CL -->|Latenza L D| M
```
### ** 3.1 Schema completo di flusso aggiornato
```mermaid
flowchart TD
    %% CONTENITORE GLOBALE
    NELO["<h3>🛡️ NETWORK FOR ETHICAL LIMITATION OF OUTCOMES</h3>
    <i>Attrito Cablato - Frição Sistémica</i>"]

    %% SEZIONE 1: SENSORI ANTROPICI (SOURCE)
    %% SEZIONE 1: SENSORI ANTROPICI (SOURCE)
    subgraph SRC ["📡 FONTE: CRIPTO-SENSORI ANTROPICI"]
        S1[["<h4>SENSORE #001</h4>
        nRF52840 + CryptoCell-310"]]
        
        S1 --> B1["<h4>ACQUISIZIONE BIOMETRICA</h4>
        • MAX30102: PPG/HRV<br>
        • AD5941: EDA<br>
        • TMP117: ΔT + Batteria"]
        
        B1 --> BUF1["<h4>SHADOW_BUFFER</h4>
        ⏱️ Cripto-firma Segmento S"]
        
        BUF1 --> PKT1["<h4>PAYLOAD S (Firmato)</h4>
        ▸ Dati Bio + Firma HW"]
    end
    
    %% SEZIONE 1.5: GATEWAY MOBILE (NEW V2.0)
    subgraph GAT ["📱 GATEWAY MOBILE (APP)"]
        APP["<h4>APP NELO - GATEWAY</h4>
        ▸ Ricezione BLE<br>
        ▸ Iniezione Context (GPS/Network)<br>
        ▸ App_Signature (Ed25519)"]
    end

    %% SEZIONE 2: RETE MESH SACRIFICABILE (TRANSITO)
    subgraph MESH ["🌐 TRASPORTO: RETE MESH COTS"]
        direction LR
        N1["<h4>NODO MESH #A</h4>
        Router OpenWRT/LoRa<br>
        ▸ Read-Only RootFS<br>
        ▸ No storage persistente"]
        
        N2["<h4>NODO MESH #B</h4>
        Router OpenWRT/LoRa<br>
        ▸ Stateless forwarder<br>
        ▸ Anonimo/sacrificabile"]
        
        N3["<h4>NODO MESH #N</h4>
        Router OpenWRT/LoRa<br>
        ▸ Auto-alimentato<br>
        ▸ Stealth enclosure"]
        
        N1 -- Sub-GHz/WiFi --> N2
        N2 -- Sub-GHz/WiFi --> N3
        N3 -- Sub-GHz/WiFi --> N1
    end

    %% SEZIONE 3: STRATI FUNZIONALI DISTRIBUITI
    subgraph LAYERS ["⚙️ STRATI FUNZIONALI DISTRIBUITI"]
        CONS["<h4>CONSENSUS LAYER</h4>
        ▸ Quorum Bizantino"]
        INTER["<h4>INTERDICTION LAYER</h4>
        ▸ L(D) Adattivo"]
        AUDIT["<h4>AUDIT LAYER</h4>
        ▸ Registro Immutabile"]
    end

    %% SEZIONE 4: STATI E FEEDBACK
    subgraph STATES ["🔄 CICLO DI STATO"]
        MONITOR["<h4>MONITORAGGIO CONTINUO</h4>
        ▸ Gradiente entropia H<br>
        ▸ Ancoraggio reale α<br>
        ▸ Divergenza consenso ΔC"]
        
        DECISION["<h4>DECISION MATRIX</h4>
        IF D ≤ 0.05: No action<br>
        IF D ∈ (0.05, 0.7): Latenza espon.<br>
        IF D ∈ [0.7, 1.0]: Latenza lineare"]
        
        FEEDBACK["<h4>FEEDBACK LOOP</h4>
        ▸ Memoria del dolore<br>
        ▸ Anti-burst detection<br>
        ▸ Cooldown adattivo"]
    end

    %% FLUSSO PRINCIPALE AGGIORNATO V2.0
    SRC -- "BLE (Segmento S)" --> APP
    APP -- "Mesh (Segmento S + A)" --> MESH
    MESH --> LAYERS
    
    %% FEEDBACK
    LAYERS --> STATES[("🔄 CICLO ADATTIVO")]
    STATES -.->|Feedback Latenza| MESH

    %% LEGENDA VISIVA
    LEG["<h4>🧩 LEGENDA ARCHITETTURALE</h4>
    <b>🟦 SORGENTE</b>: Rilevamento biometrico localizzato<br>
    <b>🟪 TRASPORTO</b>: Rete sacrificabile stateless<br>
    <b>🟩 ELABORAZIONE</b>: Strati distribuiti BFT<br>
    <b>🟨 STATO</b>: Ciclo adattivo con memoria"]

    style SRC fill:#e3f2fd,stroke:#1565c0
    style GAT fill:#fff3e0,stroke:#ef6c00,stroke-width:3px
    style MESH fill:#f3e5f5,stroke:#7b1fa2
    style LAYERS fill:#e8f5e8,stroke:#2e7d32
    style NELO fill:#1a237e,stroke:#fff,stroke-width:2px,color:#fff
    style STATES fill:#fffde7,stroke:#f57f17,stroke-width:2px
    style LEG fill:#f5f5f5,stroke:#616161,stroke-width:1px```
```
## **SPIEGAZIONE DETTAGLIATA DEL FLUSSO (V2.0)**

### **FASE 1: SORGENTE BIOMETRICA (L1)** 🟦

1. **Acquisizione hardware**:
* **PPG/HRV** (frequenza cardiaca e saturazione SpO2)
* **GSR/AD5941** (bioimpedenza cutanea)
* **TMP117** (temperatura di precisione)
* **ADC integrato** (livello carica batteria per pianificazione resilienza)


2. **Elaborazione locale**:
* I dati entrano in `SHADOW_BUFFER` (RAM volatile).
* Processamento tramite **Risk Engine** (Q16.16) per il calcolo dell'Indice di Danno $D_{v2}$.
* **Oblio hardware**: Ogni 120ms il buffer viene sovrascritto con rumore TRNG (Hardware True Random Number Generator).


3. **Firma parziale**:
* Il firmware genera il **Segmento S** (83 byte).
* Firma del Segmento S con la chiave privata `SK_sensor` (Hardware-backed CryptoCell-310).
* **Risultato**: Dato biometrico certificato alla fonte, immodificabile.



### **FASE 1.5: GATEWAY MOBILE (APP)** 🟧

1. **Aggregazione**: Lo smartphone riceve il Segmento S via BLE (MTU esteso).
2. **Arricchimento contestuale**: L'App inietta il **Segmento A**:
* `gw_node_id`: Identificativo del Gateway.
* `context_flags`: Bitmask su rete (WiFi, 4G, Roaming, posizione stimata).


3. **Firma a due livelli**:
* L'App appone una seconda firma (Ed25519) sull'intero blocco `[Segmento S + Segmento A]`.
* L'App agisce come **Gateway Stateless**: non memorizza le biometriche, ma garantisce che il pacchetto sia contestualizzato correttamente per la rete Mesh.



### **FASE 2: TRASPORTO MESH (L2)** 🟪

1. **Ricezione radio**: I nodi mesh ricevono il pacchetto a doppia firma.
2. **Validazione di rete**:
* Controllo crittografico (firma HW + firma App).
* Controllo validità nonce e timestamp contro tentativi di *replay attack*.
* **Forwarding stateless**: Il pacchetto viene instradato verso il *Consensus Layer*.


3. **Integrità del transito**:
* I nodi sono meri forwarder privi di chiavi di decodifica.
* Assenza totale di memoria persistente: se il nodo viene sequestrato, non contiene alcuna traccia dei pacchetti in transito.


4. **Hardware COTS**:
* Operatività in modalità *stealth* su router con `Read-Only RootFS`.
* Capacità di autodistruzione logica dei log in caso di rilevamento intrusione.

### **FASE 3: ELABORAZIONE DISTRIBUITA** 🟩 - Dettaglio Tecnico aggiornato

```mermaid
graph TD
    A[PACCHETTO S+A VALIDO IN ARRIVO] --> B[CONSENSUS LAYER: Verifica Doppia Firma - Firma HW 85/127 e Firma App]
    B -->|VALIDO| C[Calcolo L D con Backoff Logaritmico]
    B -->|INVALIDO| E[Scarto immediato e Log errore]
    C --> D[INIEZIONE LATENZA: Gestione code in Mesh]
    D --> F[AUDIT: Registra Hash Immutabile evento]
    F --> G[MONITORAGGIO: Aggiorna H, Alpha, Delta C]
    G --> H[FEEDBACK: Adjust cooldown adattivo]
    H -.->|Ricalibrazione| C
```

## **4\. Risk Engine V2 (Integrazione Biometrica)**

Il calcolo di *D* (Indice di Danno) è stato aggiornato per includere la variabilità dell'ossigenazione sanguigna (SpO2) misurata dal MAX30102:  
*Dv2 \= σ(α·G \+ β·(1 \- HRVnorm) \+ γ·ΔT \+ δ·(1 \- SpO2norm))*  
Il fattore δ (SpO2) normalizza l'indice di sofferenza, rendendo il quorum più sensibile a cali di saturazione ossigeno improvvisi.
