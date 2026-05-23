## Vettori di attacco noti
| Attacco | Mitigazione NELO |
| :--- | :--- |
| **Corruzione del vertice** | **Assenza di gerarchia:** Non esiste un punto centrale di vulnerabilità. Il potere decisionale è polverizzato su un pool asincrono di 127 nodi estratti casualmente a livello globale. |
| **Cattura della Soglia Bizantina** | **Hardware-Economic Lock:** Per forzare il gate hardware ($\ge 85/127$) o indurre il deadlock ($\ge 43/127$), un attaccante dovrebbe compromettere simultaneamente decine di nodi fisici protetti da enclave e challenge biometriche. Il costo logistico della coercizione distribuita supera il valore del bersaglio. |
| **Social Engineering del Quorum** | **Finestra compressa + Invisibilità:** L'estrazione è anonima e priva di registri pubblici. La finestra di voto limitata a **72 ore** toglie all'attaccante il tempo utile per imbastire campagne di ricatto o tracciamento dei flussi mesh. |
| **Backdoor in Fonderia (Fab)** | **Dispersione logistica:** La produzione e il deployment dei sensori hardware sono distribuiti su catene di approvvigionamento e fonderie concorrenti, rendendo impossibile l'inserimento di una backdoor hardware uniforme e coordinata. |
| **Deriva funzionale o Politica** | **Inerzia programmata:** Modificare il cuore logico del sistema richiede uno sforzo computazionale (PoW anti-Sybil) e un consenso antropico tali da rendere l'evoluzione del protocollo volutamente iper-conservativa. |
| **Sequestro Memoria o Analisi Forense** | **Meccanica dell'Oblio (SPEC-003):** Attacchi basati sul sequestro dei dispositivi o sull'analisi della memoria residua (Cold Boot Attack) falliscono grazie al Double-Buffering asincrono e al wipe hardware con rumore TRNG dei transistor ogni 120ms. |
