La scalabilità del Protocollo NELO su reti più grandi e complesse senza compromettere l'efficienza e la sicurezza rappresenta la sfida ingegneristica centrale del framework. Per evitare che l'aumento dei nodi saturi lo spettro radio (Sub-GHz) o introduca colli di bottiglia computazionali, il protocollo implementa una strategia di **scalabilità frazionaria e compartimentata**.

Invece di far crescere la rete in modo lineare (il che ne causerebbe il collasso), NELO scala applicando principi di **geometria frattale, routing probabilistico e separazione rigida dei livelli di calcolo**.

Ecco i tre pilastri principali su cui si fonda la scalabilità del sistema:

---

### 1. Sharding Topologico e Compartimentazione Frattale

NELO non opera mai come un'unica grande rete globale piatta. Man mano che il numero di nodi cresce su un intero territorio metropolitano o nazionale, il sistema applica uno **sharding topologico automatico**.

* **Celle Autonome:** La rete si scompone in cluster esagonali indipendenti (come formalizzato nella `SPEC-007`). Ogni cluster gestisce la propria densità di traffico e calcola il proprio Gradiente di Entropia locale ($H$).
* **Contenimento dell'Interdizione:** Se il fattore di danno $D$ si impenna nel "Quadrante A", l'algoritmo di Backoff logaritmico (`SPEC-001`) dilata la latenza *esclusivamente* all'interno delle code di instradamento di quel perimetro geografico. Questo isolamento impedisce l'effetto "domino", garantendo che il resto della rete globale continui a viaggiare a latenza nominale (1ms).

---

### 2. Disaccoppiamento del Calcolo Crittografico

In una rete mesh estesa, se ogni nodo intermedio dovesse verificare l'autenticità e la firma asimmetrica di ogni singolo pacchetto in transito, la CPU dei router subirebbe un DoS computazionale immediato, esaurendo le batterie. NELO risolve questo problema separando rigidamente i compiti:

* **I Nodi di Transito (Livello 2):** Applicano esclusivamente la pipeline di *Early Drop* definita nella `SPEC-008`. Lavorano a complessità costante $O(1)$ controllando solo i metadati, la freschezza del timestamp e la collisione dei nonce tramite un filtro di Bloom volatile. Non decifrano la firma Ed25519 e spendono una frazione infinitesima di energia per scartare i pacchetti fake.
* **Il Consensus Layer (Livello 3):** Il calcolo crittografico pesante (verifica della firma a 512 bit, calcolo dell'entropia di Shannon e della divergenza $\Delta C$ via `SPEC-009`) viene delegato esclusivamente al pool asincrono dei **127 nodi validatori** estratti casualmente. Questo pool è l'unico a spendere cicli di calcolo intensivi, mantenendo l'overhead di rete confinato a un numero fisso di attori indipendentemente dalla dimensione totale della maglia.

---

### 3. Instradamento Probabilistico e Tolleranza Bizantina Estesa

Nelle reti tradizionali, tabelle di routing troppo grandi saturano la memoria dei dispositivi edge. NELO aggira questo limite strutturale adottando un modello stateless e probabilistico:

* **Zero Routing Tables:** I nodi mesh non memorizzano l'intera mappa della rete né la cronologia dei percorsi (`SPEC-005`). I pacchetti avanzano sfruttando un meccanismo di propagazione epidemica controllata o *geocasting* (instradamento basato sulla prossimità geografica dei nodi radio).
* **Filtro delle Collisioni stocastiche:** Per evitare che i pacchetti civetta del traffico di copertura (*Chaffing*) intasino la banda disponibile all'aumentare dei nodi, l'iniezione dei falsi payload segue una **distribuzione stocastica di Poisson** modellata sul rumore di fondo della città. Questo garantisce che la densità dello spettro rimanga omogenea sia in tempo di pace che in tempo di crisi, autolimitando il traffico parassita.

---

### In sintesi

NELO scala non espandendo la propria memoria, ma **approfondendo il proprio oblio**. Più la rete diventa grande, più i nodi operano in modalità localizzata, stateless e focalizzata sul presente immediato (finestra di volatilità a 120ms). L'efficienza emerge dalla cooperazione di migliaia di micro-celle protette dalle leggi della fisica hardware, rendendo il sistema un'infrastruttura intrinsecamente ostile a qualsiasi tentativo di saturazione o cattura centralizzata.
