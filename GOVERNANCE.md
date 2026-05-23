## GOVERNANCE: Regolamento del Quorum Distribuito (v1.1-HARDENED)

## 1. Estrazione e Anonimato
*   **Campionamento:** Selezione casuale, crittografata e globale di $N=127$ nodi attivi tramite Proof-of-Work locale.
*   **Invisibilità:** La verifica dell'identità antropica (challenge biometrica + OTP) avviene esclusivamente in sede locale nel Secure Enclave. Non esiste correlazione pubblica tra l'identità del nodo e il voto espresso, tutelando i validatori dalla coercizione.

## 2. Ambito di Applicazione
*   **Vincolo di Scopo:** Il Quorum può essere attivato solo per patch e modifiche strutturali che toccano le sezioni invarianti del protocollo (`SPEC.md` sezioni I-VI). Qualsiasi altra modifica ordinaria viene rigettata dalle regole di consenso sintattico.

## 3. Soglia Bizantina Qualificata
*   **Nessun Veto Singolo:** Per evitare il deadlock infrastrutturale indotto da nodi silenti, guasti o compromessi, la soglia di approvazione è fissata a **$\ge 85$ firme valide su 127** ($\lfloor\frac{2}{3}N\rfloor + 1$). 
*   **Tolleranza ai Guasti (BFT):** Il sistema tollera fino a un massimo di 42 nodi non cooperativi. Raggiunta la quota di 85, il gate hardware esegue il merge automatico.

## 4. Finestra di Validazione (Anti-Coercizione)
*   **Finestra Stretta:** Il periodo di votazione e validazione è limitato a un massimo di **72 ore** dall'emissione della proposta. 
*   **Inerzia contro il Target:** Limitare la finestra a 72 ore riduce drasticamente il tempo utile per attacchi di computazione coordinati o tentativi di tracciamento fisico dei nodi estratti. La fretta dell'attaccante viene neutralizzata dalla latenza asincrona della rete mesh.

## 5. Sunset e Amnesia del Voto
*   **Decadenza Istantanea:** Ogni firma è vincolata a un nonce temporale monouso. Al termine delle 72 ore, la proposta si chiude: le firme non coordinate vengono scartate e i vettori di identità residui nella `SHADOW_BUFFER` vengono rasi al suolo entro 120ms. Nessun diritto di voto può essere accumulato, delegato o capitalizzato nel tempo.
