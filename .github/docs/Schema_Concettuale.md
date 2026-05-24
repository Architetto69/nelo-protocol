# Architettura del Protocollo NELO - Schema Concettuale

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

    %% Livello 2: Rete Mesh di Transito Sacrificabile
    subgraph L2 [Livello 2 - Rete Mesh di Transito COTS]
        N1[Nodo Mesh 1 <br/>Router di Maglia]
        N2[Nodo Mesh 2 <br/>Router di Maglia]
        N3[Nodo Mesh N... <br/>Router di Maglia]
        
        N1 <--> N2
        N2 <--> N3
        N3 <--> N1
    end

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
### 2. Instradamento COTS senza Stato (Livello 2)
 * *Involucri Anonimi:* I nodi mesh ricevono il payload crittografato contenente D firmato digitalmente con la chiave privata del sensore (SK_sensor), protetta da meccanismi hardware di anti-debugging (APPROTECT).
 * *Impossibilità di Manipolazione:* I nodi mesh fungono da meri router di transito. Non possiedono le chiavi per alterare D (pena l'invalidazione della firma) né supporti di memoria persistente (Read-Only RootFS) per registrare i transiti.
### 3. Strati di Rete e Applicazione della Frizione (Livello 3)
 * *Consensus Layer:* Quando i pacchetti viaggiano nella mesh, il protocollo estrae in modo probabilistico un pool di 127 nodi per validare l'integrità dell'evento tramite quorum bizantino (\ge 85 firme oneste).
 * *Interdiction Layer (Il vero bersaglio della Frizione):* Una volta validato un valore di D > 0.05, l'algoritmo di Backoff Logaritmico Adattivo calcola la latenza L(D). Questa latenza viene iniettata *all'interno delle code di instradamento dei Nodi Mesh (Livello 2)*. La rete rallenta se stessa, espandendo i tempi di trasmissione fino a L_MAX.
 * *Audit Layer:* Le transizioni critiche approvate dal quorum sbloccano la scrittura fisica su e-Fuse o registri non volatili immutabili distribuiti, creando una traccia crittografica dell'interdizione che esclude qualsiasi dato tracciabile o identità persistente.
## Flusso dei Dati

1. **Rilevamento** → I sensori captano segnali biometrici
2. **Trasmissione** → I nodi mesh inoltrano dati anonimi
3. **Calcolo** → Il Risk Layer calcola l'Indice D
4. **Validazione** → Il Consensus Layer conferma le decisioni
5. **Azione** → L'Interdiction Layer applica frizione
6. **Registrazione** → L'Audit Layer traccia gli eventi

## Proprietà Emergenti

- **Resistenza alla Cattura**: Nessun punto di controllo centrale
- **Privacy by Design**: Dati personali distrutti alla fonte
