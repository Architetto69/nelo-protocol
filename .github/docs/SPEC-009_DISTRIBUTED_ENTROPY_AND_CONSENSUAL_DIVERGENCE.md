# SPEC-009: Gradiente di Entropia Distribuito e Divergenza di Consenso Sintattico

**Versione:** 3.1-HARDENED

**Stato:** APPROVATO / STRATO LOGICO DI LIVELLO 3

**Riferimenti di Sistema:** `SPEC-001_BACKOFF`, `SPEC-002_QUORUM`, `SPEC-006_THREAT_MODELING`, `SPEC-008_PACKET`

---

## 1. Obiettivo e Vettore Antiavversariale

Se la determinazione dell'Indice di Danno $D$ all'interno di un quadrante geografico mesh si basasse su una media matematica lineare o su una statistica cumulativa semplice, il protocollo cadrebbe vittima di manipolazioni semantiche. Un attaccante statale o un attore strategico con ingenti risorse computazionali potrebbe orchestrare un attacco Sybil immettendo milioni di pacchetti sintetici falsificati a stress zero ($D = 0.00$) per diluire la media d'area, nascondendo efficacemente un atto di violenza o una coercizione fisica in corso.

La `SPEC-009` definisce i vincoli matematici del *Consensus Layer* per calcolare la coerenza topologica dei dati distribuiti. Il sistema analizza la struttura informativa dei flussi radio tramite due metriche invarianti: il **Gradiente di Entropia Spaziale ($H$)** e la **Divergenza di Consenso Sintattico ($\Delta C$)**. L'obiettivo è rilevare istantaneamente le anomalie di partizionamento informativo e forzare transizioni di stato protettive a sicurezza pessimistica.

---

## 2. Modello Matematico del Consenso

### 2.1 Il Gradiente di Entropia Spaziale ($H$)

Per ogni cella topologica o quadrante di propagazione radio, il pool asincrono dei validatori calcola l'entropia informativa di Shannon dei segnali del fattore $D$ estratti dai pacchetti validati in transito.

L'entropia locale $H$ è formalizzata come:

$$H = -\sum_{i=1}^{K} p(D_i) \log_2 p(D_i)$$

Dove $p(D_i)$ rappresenta la frequenza relativa o densità di probabilità di occorrenza di un determinato sotto-intervallo discreto di stress (quantizzato su canali fixed-point Q1.15) tra i $K$ sensori unici geolocalizzati attivi nell'area.

* **Condizione di Coerenza Nominale ($H \to 0$):** Se l'intero quadrante si trova in stato di quiete ($D \approx 0.0$) o se l'intera area è uniformemente colpita da un trauma macrostrutturale coerente ($D \approx 1.0$), la dispersione informativa è minima. Il sistema valida il dato come strutturalmente integro.
* **Condizione di Attacco o Dissociazione ($H \to 1$):** Se un attaccante inietta flussi contraddittori di pacchetti sintetici per mascherare il panico biologico reale di una piazza, i valori di $D$ si frammentano nello spazio delle probabilità. L'entropia subisce un'impennata che supera la soglia critica di audit $\tau_H$.

### 2.2 La Divergenza di Consenso Sintattico ($\Delta C$)

La metrica calcola lo scostamento probabilistico tra i vettori di telemetria generati in modalità automatica dall'hardware edge dei sensori e i segnali di interazione antropica cosciente veicolati dalla mesh (il Quorum Asincrono e i "visti" manuali descritti nella `SPEC-002`).

$$\Delta C = \frac{| \Psi_{\text{antropico}} - \Phi_{\text{automatico}} |}{\Phi_{\text{automatico}} + \epsilon}$$

Dove $\Psi$ definisce il tasso di attivazione e firma dei nodi umani estratti, $\Phi$ definisce l'aspettativa matematica della telemetria d'area e $\epsilon$ è una costante infinitesima ($10^{-5}$) per la prevenzione di eccezioni hardware da divisione per zero.

* Un valore di $\Delta C > \tau_C$ indica l'impronta digitale di un attacco di **anestesia sociale**: i sensori biologici della popolazione gridano un'anomalia sistemica che i nodi di trasporto circostante tentano di attenuare o sopprimere informaticamente.

---

## 3. Algoritmo di Transizione a Soglia Pessimistica

Al rilevamento di una violazione degli invarianti di entropia o divergenza, il *Consensus Layer* revoca la coalescenza ordinaria dei dati (la media ponderata) e applica la **Clausola di Sfiducia**. Il sistema ignora i flussi di rumore a stress zero e assume lo scenario peggiore configurabile nel quadrante radio, isolando la cella infetta.

### 3.1 Implementazione dell'Algoritmo (C-Implementation)

```c
#include <math.h>
#include <stdbool.h>

#define QUORUM_SIZE 127
#define THRESHOLD_H 0.85f
#define THRESHOLD_C 0.70f

typedef struct {
    float d_value;
    bool is_human_confirmed;
} SensorReading_t;

float calculate_mesh_systemic_d(SensorReading_t readings[QUORUM_SIZE]) {
    float sum_d = 0.0f;
    float max_d = 0.0f;
    int human_inputs = 0;
    
    // 1. Parsing del blocco e mappatura delle metriche locali
    for (int i = 0; i < QUORUM_SIZE; i++) {
        float current_d = readings[i].d_value;
        sum_d += current_d;
        
        if (current_d > max_d) {
            max_d = current_d;
        }
        if (readings[i].is_human_confirmed) {
            human_inputs++;
        }
    }
    
    float mean_d = sum_d / QUORUM_SIZE;
    
    // Calcolo empirico della Divergenza Sintattica Delta C
    float expected_human = mean_d * (float)QUORUM_SIZE * 0.5f; 
    float delta_c = fabsf((float)human_inputs - expected_human) / (expected_human + 0.00001f);
    
    // Calcolo dell'Entropia di Shannon discretizzata (Stress vs Quiet)
    int stress_count = 0;
    for (int i = 0; i < QUORUM_SIZE; i++) {
        if (readings[i].d_value > 0.5f) {
            stress_count++;
        }
    }
    
    float p_stress = (float)stress_count / (float)QUORUM_SIZE;
    float p_quiet = 1.0f - p_stress;
    
    float h = 0.0f;
    if (p_stress > 0.0f && p_stress < 1.0f) {
        h = -(p_stress * log2f(p_stress) + p_quiet * log2f(p_quiet));
    }

    // 2. Valutazione della Teoria dei Giochi Avversaria
    if (h > THRESHOLD_H || delta_c > THRESHOLD_C) {
        // [ANOMALIA STRUTTURALE RILEVATA] 
        // L'ambiente informativo è in stato di manipolazione o partizionamento.
        // Attivazione della Soglia Pessimistica: se una frazione critica di nodi
        // autenticati nel perimetro segnala trauma, si applica l'interdizione massima.
        if (max_d > 0.7f) {
            return max_d; 
        }
    }

    // Regime nominale: assenza di vettori di attacco semantico
    return mean_d;
}

```

---

## 4. Chiusura del Ciclo di Feedback Energetico

L'output dell'algoritmo definisce il valore di `D_effettivo` che viene iniettato nello strato di transito di livello 1 (`SPEC-001`).

Quando la clausola di sfiducia della `SPEC-009` forza il sistema verso il valore `max_d`, la frizione logaritmica agisce immediatamente sulle code di routing dei router di maglia (`SPEC-005`), dilatando la latenza operativa ed estendendo la memoria del cooldown nell'area geografica interessata. La rete risponde all'aggressione asimmetrica bloccando le sue stesse linee di comunicazione digitali locali nel quadrante colpito, esaurendo l'efficienza logistica dei vettori di sorveglianza o incursione.

---

## 5. Note di Ispezione e Sicurezza Corrente

* **Prevenzione dei DoS Computazionali:** Il calcolo dei logaritmi in virgola mobile (`log2f`) deve essere eseguito esclusivamente sul pool ristretto e asincrono dei 127 nodi validatori estratti e mai dai nodi di semplice transito mesh, i quali applicano unicamente i filtri ad accesso costante $O(1)$ definiti nella `SPEC-008`.
* **Sanificazione Post-Calcolo:** Al termine di ogni ciclo di consenso, le variabili intermedie utilizzate per determinare la densità delle probabilità $p(D_i)$ subiscono un'operazione di sovrascrittura forzata con iniezione di rumore TRNG per preservare l'oblio topologico del sistema (`SPEC-003`).
