## SPEC-001: Algoritmo di Backoff Logaritmico Adattivo (The "Sands of Time v1.1-ADAPTIVE")
## 1. Obiettivo
Regolare dinamicamente latenza di rete, velocità di esecuzione e throughput in funzione del segnale di Sofferenza-Scarto (D). L'obiettivo è rendere le operazioni di coercizione massiva logisticamente non sostenibili, senza bloccare la liveness della rete per falsi positivi.

## 2. Input: Il Segnale di Danno (D)
D ∈ [0, 1] è un valore normalizzato ottenuto tramite **Quorum Byzantine di sensori**.
Requisito: D deve essere firmato da ≥2f+1 nodi indipendenti su f avversari. Input non firmati vengono scartati.
D = 0: Stato di entropia minima.
D = 1: Stato di trauma critico o geometria coercitiva rilevata.
## 3. Modello Matematico della Frizione
La latenza applicata L è una funzione a tratti per evitare freeze totali da falsi positivi.

### Equazione di Stato:
<pre>
L(D) = 
 0,                                      se D ≤ 0.05
  L_base _ exp(k1 _ D),                   se 0.05 < D ≤ 0.7
  L_base _ exp(k1 _ 0.7) + m * (D-0.7),   se 0.7 < D ≤ 1.0
**Parametri di Configurazione:**
- `L_base`: Latenza nominale (default: 1ms)
- `k1`: Costante di Interdizione fase esponenziale (default: 12.0)
- `m`: Pendenza fase lineare (default: 5000 ms per D)
- `L_max`: Hard cap di sicurezza (default: 3600s)
</pre>
Questo crea una curva esponenziale fino a D=0.7, poi lineare verso L_max. Mantiene il “muro” per attacchi massivi, ma evita freeze infiniti per rumore.

## 4. Quadranti di Interdizione
| Soglia (D) | Stato del Sistema | Latenza Tipica | Effetto Pratico |
| --- | --- | --- | --- |
| 0.0 - 0.2 | Fluidità | 0-15ms | Nessun attrito percepibile |
| 0.2 - 0.5 | Frizione Attiva | 15ms-4.8s | Sincronizzazione sorveglianza degradata |
| 0.5 - 0.7 | Inerzia | 4.8s-120s | Catene di comando si spezzano |
| 0.7 - 1.0 | Interdizione Controllata | 120s-3600s | Costo/bit cresce linearmente, no deadlock |

## 5. Cooldown Adattivo: "Memoria del Dolore"
Per prevenire attacchi pulsanti, D interno non scende istantaneamente. Il decadimento dipende dalla storia recente:

τ = τ_base _ (1 + α _ burst_factor)
D_internal(t+1) = D_internal(t) * exp(-Δt / τ)
- `τ_base`: 100s default
- `burst_factor`: 0 se <2 picchi in 60s, fino a 5 se pattern pulsante rilevato
- `α`: 0.8

Se rilevi burst ripetuti, la memoria si allunga. Se è un evento singolo, il sistema torna operativo più velocemente.

## 6. Implementazione Firmware C-Style
Implementare in Kernel Space. Mai in userland.

```c
typedef struct {
    float d_internal;
    uint64_t last_update_ms;
    uint32_t burst_count;
} friction_state_t;

float compute_nelo_friction(friction_state_t *state, float d_raw, uint64_t now_ms) {
    const float K1 = 12.0f;
    const float L_BASE = 0.001f;
    const float L_MAX = 3600.0f;
    const float M = 5.0f; // 5000ms per unità D in fase lineare
    const float TAU_BASE = 100.0f;
    const float ALPHA = 0.8f;

    // Aggiorna D_internal con cooldown adattivo
    float dt = (now_ms - state->last_update_ms) / 1000.0f;
    float burst_factor = (state->burst_count > 2) ? 5.0f : 0.0f;
    float tau = TAU_BASE * (1.0f + ALPHA * burst_factor);
    state->d_internal = state->d_internal * expf(-dt / tau);
    state->d_internal = fmaxf(state->d_internal, d_raw);
    
    // Rileva burst
    if (d_raw > 0.5f) state->burst_count++; 
    else if (dt > 60.0f) state->burst_count = 0;

    float d = state->d_internal;
    if (d <= 0.05f) return 0.0f;

    float delay;
    if (d <= 0.7f) {
        delay = L_BASE * expf(K1 * d);
    } else {
        delay = L_BASE * expf(K1 * 0.7f) + M * (d - 0.7f);
    }

    return (delay > L_MAX) ? L_MAX : delay;
}
```
## 7. Invarianti di Sicurezza
1. *Integrità di D*: D non entra mai nel calcolo senza firma quorum Byzantine.
2. *Monotonicità*: L(D) è monotona non decrescente.
3. *Liveness garantita*: L(D) ≤ L_max per ogni D. La rete non si blocca mai indefinitamente.
4. *Impossibilità di reset*: Il cooldown è gestito solo in kernel space. Chiamate userland non possono resettare `d_internal`.

## 6. Sicurezza del Backoff: Il "Linear Cooldown"
Per prevenire attacchi pulsanti (burst di violenza alternati a silenzi), il valore di D interno al firmware non scende istantaneamente. Se il sensore segna 0, il sistema riduce la latenza solo del 1\% ogni secondo. NELO ricorda il dolore più a lungo di quanto l'aggressore possa sostenere l'attacco.

[SPEC_001: v1.1-ADAPTIVE] [LOG: ATTRITO CONTROLLATO, LIVENESS PRESERVATA]
