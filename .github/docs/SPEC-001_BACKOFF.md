## SPEC-001: Algoritmo di Backoff Logaritmico (The "Sands of Time")
## 1. Obiettivo
Regolare dinamicamente la velocità di esecuzione, la latenza di rete e la capacità di calcolo in funzione del segnale di Sofferenza-Scarto (D). L'obiettivo è trasformare l'infrastruttura da conduttore a isolante man mano che la coercizione scala.
## 2. Input: Il Segnale di Danno (D)
Il segnale D è un valore normalizzato [0, 1] derivato dalla fusione dei sensori (fisiologici, ambientali, di flusso).
D = 0: Stato di entropia minima (libertà di movimento).
D = 1: Stato di trauma critico o geometria coercitiva rilevata.
## 3. Modello Matematico della Frizione
La latenza applicata (L) aumenta esponenzialmente rispetto al danno, creando un "muro di tempo" invalicabile per chi tenta operazioni massive.
<pre>
Equazione di Stato:
Parametri di Configurazione:
L_{base}: Latenza nominale (default: 10^0 ms).
k: Costante di Interdizione (valore consigliato: 12.0).
L_{max}: Hard cap di sicurezza (default: 3.6 \cdot 10^6 ms / 1 ora).
4. Analisi dei Quadranti di Interdizione
Soglia (D)
Stato del Sistema
Effetto Pratico
0.0 - 0.2
Fluidità
Nessuna percezione di attrito.
0.2 - 0.5
Frizione Attiva
Latenza udibile/visibile. La sincronizzazione dei sistemi di sorveglianza inizia a fallire.
0.5 - 0.8
Inerzia
Ritardi massivi (secondi/minuti). Le catene di comando si spezzano.
0.8 - 1.0
Interdizione
Il sistema si "congela". Il costo energetico per bit trasmesso diventa infinito.
</pre>

## 5. Implementazione del Firmware (C-Style)
Il "Fabbro" deve implementare questa logica nel ciclo più interno del processore (Kernel Space) per evitare bypass:
<pre>
    /**
     * @brief Calcola il ritardo da applicare al ciclo di clock o al pacchetto di rete.
     * @param distress_signal Valore normalizzato [0.0 - 1.0]
     * @return float Ritardo in secondi
     */
    float compute_nelo_friction(float distress_signal) {
        const float K_INTERDICTION = 12.0f; 
        const float BASE_LATENCY = 0.001f; // 1ms base
        const float HARD_CAP = 3600.0f;    // 1 ora max
    
        if (distress_signal <= 0.05f) {
            return 0.0f; // Zona morta per evitare rumore di fondo
        }
    
        // Calcolo esponenziale: la frizione diventa "muro"
        float calculated_delay = BASE_LATENCY * expf(K_INTERDICTION * distress_signal);
    
        return (calculated_delay > HARD_CAP) ? HARD_CAP : calculated_delay;
    }
</pre>

## 6. Sicurezza del Backoff: Il "Linear Cooldown"
Per prevenire attacchi pulsanti (burst di violenza alternati a silenzi), il valore di D interno al firmware non scende istantaneamente. Se il sensore segna 0, il sistema riduce la latenza solo del 1\% ogni secondo. NELO ricorda il dolore più a lungo di quanto l'aggressore possa sostenere l'attacco.

[SPEC_001: INTEGRATED] [LOG: MATEMATICA DELL'ATTRITO OPERATIVA]
