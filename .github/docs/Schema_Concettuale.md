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


# Architettura del Protocollo NELO - Schema Completo di Flusso

```mermaid
flowchart TD
    %% CONTENITORE GLOBALE
    NELO["<h3>🛡️ NETWORK FOR ETHICAL LIMITATION OF OUTCOMES</h3>
    <i>Attrito Cablato - Frição Sistémica</i>"]

    %% SEZIONE 1: SENSORI ANTROPICI (SOURCE)
    subgraph SRC ["📡 FONTE: CRIPTO-SENSORI ANTROPICI"]
        S1[["<h4>SENSORE #001</h4>
        nRF52840 + CryptoCell-310"]]
        
        S1 --> B1["<h4>ACQUISIZIONE BIOMETRICA</h4>
        • MAX30102: PPG/HRV<br>
        • MAX30009: GSR/Impedenza<br>
        • TMP117: ΔT termico"]
        
        B1 --> BUF1["<h4>SHADOW_BUFFER</h4>
        ⚡ RAM volatile<br>
        ⏱️ Timer hardware 120ms<br>
        🧹 Wipe TRNG post-elab."]
        
        BUF1 --> CALC1["<h4>RISK ENGINE Q16.16</h4>
        D = σ(α·G + β·(1-V^γ·ΔT - δ))<br>
        ⚙️ Fixed-point deterministico"]
        
        CALC1 --> PKT1["<h4>PAYLOAD AEAD</h4>
        ▸ Timestamp<br>
        ▸ Nonce RNG<br>
        ▸ D ∈ [0, 1]<br>
        ▸ Sig Ed25519 (SK_sensor)"]
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
        ▸ Quorum Asincrono<br>
        ▸ 127 nodi estratti<br>
        ▸ ≥85 firme bizantine<br>
        ▸ Timeout 72h"]
        
        INTER["<h4>INTERDICTION LAYER</h4>
        ▸ Backoff logaritmico<br>
        L(D) = L_base·e^{k·D}<br>
        ▸ Fase lineare D>0.7<br>
        ▸ L_max = 3600s"]
        
        AUDIT["<h4>AUDIT LAYER</h4>
        ▸ Registro immutabile<br>
        ▸ Solo metadata anonimi<br>
        ▸ No identità persistenti"]
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

    %% FLUSSO PRINCIPALE
    SRC -- "BLE/802.15.4<br>Payload D firmato" --> MESH
    MESH -- "Trasmissione mesh<br>Dati criptati" --> LAYERS
    LAYERS -- "Validazione e attuazione" --> STATES
    
    %% Connessioni di Feedback corrette senza conflitti lessicali
    STATES -.->|Feedback latenza| MESH
    STATES -->|Audit trail| AUDIT

    %% LEGENDA VISIVA
    LEG["<h4>🧩 LEGENDA ARCHITETTURALE</h4>
    <b>🟦 SORGENTE</b>: Rilevamento biometrico localizzato<br>
    <b>🟪 TRASPORTO</b>: Rete sacrificabile stateless<br>
    <b>🟩 ELABORAZIONE</b>: Strati distribuiti BFT<br>
    <b>🟨 STATO</b>: Ciclo adattivo con memoria"]

    style NELO fill:#1a237e,stroke:#fff,stroke-width:2px,color:#fff
    style SRC fill:#e3f2fd,stroke:#1565c0,stroke-width:2px
    style MESH fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    style LAYERS fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px
    style STATES fill:#fffde7,stroke:#f57f17,stroke-width:2px
    style LEG fill:#f5f5f5,stroke:#616161,stroke-width:1px```
```
## **SPIEGAZIONE DETTAGLIATA DEL FLUSSO**

### **FASE 1: SORGENTE BIOMETRICA** 🟦
1. **Acquisizione hardware**: Tre sensori specializzati catturano:
   - **PPG/HRV** (variabilità cardiaca) → stress psicofisico
   - **GSR** (conduttanza cutanea) → attivazione simpatico
   - **ΔT termico** (gradiente temperatura) → shock vascolare

2. **Elaborazione locale**: 
   - I dati grezzi entrano in `SHADOW_BUFFER` (RAM volatile)
   - Vengono processati dal **Risk Engine** in virgola fissa Q16.16
   - Viene calcolato `D = sigmoid(2·G + 2.5·(1-V) + 3·ΔT - 3)`
   - **Ogni 120ms il buffer viene sovrascritto con rumore TRNG**

3. **Cifratura e firma**:
   - Il valore `D` viene firmato con la chiave privata del sensore (protetta da APPROTECT)
   - Il payload include timestamp, nonce random e firma Ed25519
   - **Zero dati identificativi persistono**

### **FASE 2: TRASPORTO MESH** 🟪
1. **Ricezione radio**: I nodi mesh catturano i pacchetti via BLE/LoRa
2. **Validazione minima**: 
   - Controllo freshness (timestamp recente)
   - Verifica nonce univoco (anti-replay)
   - **Nessuna decifratura del contenuto**
3. **Forwarding stateless**:
   - I pacchetti vengono inoltrati via mesh
   - Ogni nodo **non memorizza** dati di routing persistenti
   - File system in sola lettura (Read-Only RootFS)
4. **Caratteristiche fisiche**:
   - Hardware COTS anonimizzato (rimozione serigrafie)
   - Involucro camuffato (stealth enclosure)
   - Auto-alimentazione (solare/parassita)

### **FASE 3: ELABORAZIONE DISTRIBUITA** 🟩 - Dettaglio Tecnico

1. **CONSENSUS LAYER (Validazione Byzantine)**
Il meccanismo di consenso più robusto mai concepito per prevenire la cattura istituzionale.

#### **Algoritmo del Quorum Asincrono Probabilistico** 
```c
// PSEUDOCODICE SEMPLIFICATO DEL GATE UMANO
bool validate_protocol_change(Proposal *p) {
    // 1. SELEZIONE CASUALE DEI VALIDATORI
    Validator_Pool = extract_random_nodes(127, GLOBAL_NETWORK);
    
    // 2. FINESTRA TEMPORALE STRETTA (72h)
    Start_Timer(72_hours);
    
    // 3. RACCOLTA ASINCRONA DI FIRME
    Signatures_Collected = 0;
    while (!Timer_Expired) {
        foreach (Validator in Validator_Pool) {
            if (Validator.submits_signature(p)) {
                // VERIFICA HUMAN-IN-THE-LOOP
                if (Validator.completes_biometric_challenge() && 
                    Validator.enters_correct_OTP()) {
                    
                    Signatures_Collected++;
                    
                    // 4. SOGLIA BIZANTINA CRITICA
                    if (Signatures_Collected >= 85) {
                        // QUORUM RAGGIUNTO - GATE APERTO
                        commit_to_hardware_efuse(p);
                        return true;
                    }
                }
            }
        }
    }
    
    // 5. FALLBACK DI SICUREZZA
    if (Signatures_Collected < 64) {
        activate_emergency_lockdown();
    }
    
    return false; // CAMBIAMENTO RESPINTO
}
```

#### **Caratteristiche Critiche del Consenso**:
| Caratteristica | Spiegazione | Effetto Protettivo |
|--------------|------------|-------------------|
| **Pool di 127** | Numero di Mersenne (2⁷-1) ottimale | Supera soglia "gruppo piccolo" rendendo collusioni impraticabili |
| **Soglia 85/127** | Maggioranza qualificata (≥2/3+1) | Tolleranza bizantina: supporta fino a 42 nodi malevoli/disconnessi |
| **Human-in-the-Loop** | Challenge biometrica + OTP | Impedisce automazione/attacchi remoti - richiede azione umana conscia |
| **Finestra 72h** | Tempistica stretta | Previene attacchi "slow drip" e coordinazioni occulte |
| **Estrazione casuale** | Validatori scelti probabilisticamente | Rende impossibile corrompere in anticipo i validatori |

2. **INTERDICTION LAYER (Meccanica della Frizione)**
Il cuore operativo di NELO: trasforma il segnale `D` in attrito di rete misurabile.

#### **Funzione di Backoff Logaritmico Adattivo**:
```math
L(D) = 
\begin{cases} 
0 & \text{se } D \le 0.05 \\
L_{base} \cdot e^{k \cdot D} & \text{se } 0.05 < D \le 0.7 \\
L_{base} \cdot e^{k \cdot 0.7} + m \cdot (D - 0.7) & \text{se } D > 0.7 
\end{cases}
```

#### **Parametri di Configurazione**:
```c
#define L_BASE    0.001      // 1ms (latenza nominale)
#define K12.0       // Costante di interdizione
#define M         11985.0    // Pendenza fase lineare (ms per unità D)
#define L_MAX     3600000    // 1 ora = limite massimo assoluto
```

#### **Quadranti Operativi del Sistema**:
| Valore D | Fase | Latenza Tipica | Effetto sul Controllo di Massa |
|----------|------|----------------|--------------------------------|
| **0.00 - 0.20** | Fase Fluida | 0-15ms | Nessun impatto su operazioni legittime |
| **0.20 - 0.50** | Frizione Attiva | 15ms - 4.8s | Sincronizzazione sorveglianza degradata |
| **0.50 - 0.70** | Inerzia Sistemica | 4.8s - 120s | Catene di comando si spezzano |
| **0.70 - 1.00** | Interdizione Controllata | 120s - 3600s | Costo/bit cresce linearmente, sistema in stallo operativo |

#### **Memoria del Dolore (Cooldown Adattivo)**:
```c
// Il sistema "ricorda" gli attacchi ripetuti
float calculate_decay_factor(int burst_count) {
    float tau_base = 100.0;  // Decadimento base: 100 secondi
    float burst_factor = (burst_count > 2) ? 5.0 : 0.0;
    float alpha = 0.8;
    
    // Se rilevi pattern pulsante, la memoria si allunga
    return tau_base * (1.0 + alpha * burst_factor);
}
```

3. **AUDIT LAYER (Trasparenza Senza Identità)**
Registro immutabile che documenta le azioni senza mai compromettere la privacy.

#### **Strutt Record di Audit**:
```json
{
  "event_hash": "sha3-512 dei metadati",
  "timestamp": "2024-01-01T00:00:00Z",
  "consensus_reached": true,
  "signature_count": 92,
  "action_type": "latency_injection",
  "duration_ms": 45000,
  "risk_level": "high",
  "anomaly_pattern": "coordinated_oppression",
  "geographic_region": "hash(continent) [anonimo]",
  "data_volume_affected": "10-100GB range",
  "quorum_validators_hash": "hash(127 node IDs)"
}
```

#### **Principi di Audit NELO**:
1. **Hash, mai dati grezzi**: Solo fingerprint crittografiche
2. **Range, mai precisione**: "10-100GB" non "47.3GB"
3. **Categorie, mai identità**: "regione: Europa Nord" non "coordinate GPS"
4. **Pattern, mai individui**: "10000+ segnali stress simultanei" non "lista nomi"

4. **INVARIANTI SISTEMICHE (Monitoraggio Continuo)**
Tre metriche fondamentali che il sistema monitora per preservare la propria integrità:

**1. Gradiente di Entropia dei Nodi (H)**
```math
H = -\sum_{i=1}^{N} p_i \log_2 p_i
```
**Monitora**: Diversità e interconnessione della rete  
**Allarme**: Crollo di H → nodi si isolano per panico  
**Contromisura**: Dynamic Friction - rallenta il contagio della paura

### **2. Rapporto di Ancoraggio Reale (α)**
```math
\alpha = \frac{\text{Risorse Reali}}{\text{Valore Finanziario Derivato}}
```
**Monitora**: Distacco tra economia reale e finanza speculativa  
**Allarme**: α → 0 (bolla puramente sintattica)  
**Contromisura**: Aumento costo computazionale → "affama" i contratti ricorsivi

### **3. Divergenza di Consenso Sintattico (ΔC)**
```math
\Delta C = \frac{\text{Decisioni Esterne}}{\text{Decisioni Automatiche}}
```
**Monitora**: Tentativi di intervento manuale/eccezionale  
**Allarme**: ΔC > soglia → "stato di emergenza" dichiarato  
**Contromisura**: Firewall logici isolano il nodo deviante

## **FLUSSO COMPLETO DELLA FASE 3**:

```
[PACCHETTO D VALIDO IN ARRIVO]
        ↓
[CONSENSUS LAYER: Verifica firme ≥85/127]
        ↓
[SE VALIDO → Calcola L(D) con backoff]
        ↓
[INIEZIONE LATENZA nei router mesh]
        ↓
[AUDIT: Registra hash evento immutabile]
        ↓
[MONITORAGGIO: Aggiorna H, α, ΔC]
        ↓
[FEEDBACK: Adjust cooldown per pattern burst]
```

## **PROPRIETÀ EMERGENTI DI QUESTA FASE**:

1.	Resistenza alla Cattura: Nessun punto di controllo centrale
2.	Privacy by Design: Dati personali distrutti alla fonte

