# Architettura del Protocollo NELO - Schema Concettuale

```mermaid
graph TB
    %% Livello 1: Sensori Biometrici
    subgraph L1 [Livello 1 - Rilevamento Biometrico]
        S1[Sensore Umano 1]
        S2[Sensore Umano 2]
        S3[Sensore Umano N...]
    end

    %% Livello 2: Nodi di Transito
    subgraph L2 [Livello 2 - Rete Mesh di Transito]
        N1[Nodo Mesh 1]
        N2[Nodo Mesh 2]
        N3[Nodo Mesh N...]
    end

    %% Livello 3: Strati Funzionali
    subgraph L3 [Livello 3 - Strati Funzionali NELO]
        RL[Risk Layer<br/>Calcolo Indice Danno D]
        CL[Consensus Layer<br/>Validazione Byzantine]
        IL[Interdiction Layer<br/>Applicazione Frizione]
        AL[Audit Layer<br/>Registro Immutabile]
    end

    %% Collegamenti principali
    S1 -- Segnale Biometrico --> N1
    S2 -- Segnale Biometrico --> N2
    S3 -- Segnale Biometrico --> N3

    N1 -- Dati Anonimi --> RL
    N2 -- Dati Anonimi --> RL
    N3 -- Dati Anonimi --> RL

    RL -- Indice D --> CL
    CL -- Decisione --> IL
    IL -- Azione --> AL

    %% Collegamenti di ritorno
    IL -. Feedback Frizione .-> S1
    IL -. Feedback Frizione .-> S2
    IL -. Feedback Frizione .-> S3

    %% Collegamenti orizzontali (mesh)
    N1 <--> N2
    N2 <--> N3
    N3 <--> N1

    %% Stile e disposizione
    classDef sensor fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    classDef node fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    classDef layer fill:#e8f5e8,stroke:#1b5e20,stroke-width:2px

    class S1,S2,S3 sensor
    class N1,N2,N3 node
    class RL,CL,IL,AL layer
```

## Spiegazione dello Schema

### 1. **Livello 1 - Rilevamento Biometrico** (Cerchio Blu)
- **Cripto-Sensori Antropici** indossati dagli utenti
- Misurano segnali biologici: frequenza cardiaca, conduttanza cutanea, temperatura
- Trasmettono solo dati anonimi e cifrati
- Distruggono i dati identificativi ogni 120ms

### 2. **Livello 2 - Rete Mesh di Transito** (Cerchio Viola)
- **Nodi sacrificabili** nascosti nell'ambiente
- Ricevono dati dai sensori via Bluetooth/LoRa
- Inoltrano i dati attraverso la rete mesh
- Non memorizzano informazioni persistenti
- Autonomia energetica (solare/parassitismo)

### 3. **Livello 3 - Strati Funzionali** (Cerchio Verde)

#### **Risk Layer**
- Calcola l'**Indice di Danno D** (0.0 - 1.0)
- Formula: `D = α·G + β·(1-V) + γ·ΔT - δ`
- Dove: G = conduttanza, V = variabilità cardiaca, ΔT = gradiente termico

#### **Consensus Layer**
- **Quorum Asincrono Probabilistico**
- 127 nodi estratti casualmente
- Richiede 85 firme valide per decisioni
- Previene la cattura del sistema

#### **Interdiction Layer**
- Applica **frizione sistematica**
- Aumenta la latenza in base al valore D
- Rende le operazioni coercitive logisticamente fallimentari

#### **Audit Layer**
- Registro immutabile delle azioni
- Trasparente e verificabile
- Senza dati identificativi

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