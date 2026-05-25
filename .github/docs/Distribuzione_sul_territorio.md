La distribuzione dei **127 individui** all'interno della topologia di rete risponde a un preciso criterio di **ottimizzazione statistica e geometrica della copertura**, calcolato per massimizzare la resilienza della maglia mesh a fronte del minor raggio radio possibile.
Per coprire un territorio o un'area d'azione minimizzando i punti ciechi e prevenendo il collasso del sistema in caso di cattura di più nodi, la disposizione sul terreno non deve essere né centralizzata né puramente casuale, bensì strutturata secondo una **geometria a cluster esagonali parzialmente sovrapposti**.
## 1. La Topologia a Cluster Esagonali
La configurazione ottimale prevede la scomposizione della rete in macro-aree, dove i 127 elementi vengono distribuiti seguendo una densità differenziata tra **Nodi di Frontiera (Periferici)** e **Nodi di Dorsale (Core)**.
```
          / \     / \
         /   \___/   \
        |  N  |  N  |  N  |   <-- Nodi Periferici (Frontiera)
        |     |     |     |
       / \___/ \___/ \___/ \
      /   \   \   /   /   \
     |  N  |   [C]   |  N  |  <-- Nodo Centrale / Dorsale [C]
     |     |         |     |
      \___/ \___/ \___/ \___/
          \   /   \   /
           \ /     \ /

```
La ripartizione sul campo segue tre regole geometriche:
 * **I nodi periferici (Frontiera):** Hanno il compito di rimanere agganciati al confine biologico dei cripto-sensori, intercettando i payload AEAD via corto raggio (BLE/802.15.4). Coprono la fascia più esposta del territorio.
 * **I nodi di dorsale (Core):** Posizionati nei punti di sella geometrici (aree elevate, intercapedini strutturali dominanti), non interrogano i sensori ma fanno unicamente da "ponti LoRa" ad alta potenza per rilanciare i dati aggregati verso gli altri cluster.
 * **La distanza di sicurezza:** Ogni nodo deve trovarsi all'interno del raggio di copertura di almeno altri **3 nodi adiacenti** (ridondanza N+2). In ambiente urbano denso, questo si traduce in una distanza lineare compresa tra i 150 e i 300 metri; in spazio aperto o rurale, la dorsale LoRa Sub-GHz estende questa maglia fino a 1.5 - 3 km per singolo salto (*hop*).
## 2. Ripartizione Numerica e Densità della Rete
La distribuzione numerica dei 127 nodi deve essere pianificata per prevenire il fenomeno del *collo di bottiglia* (congestione del traffico dati sui nodi centrali) e per assorbire un tasso di guasto o cattura fisica fino al **40% dell'infrastruttura** senza perdita di connettività globale.
| Tipologia di Nodo | Percentuale Allocata | Numero di Unità | Funzione Geometrica |
|---|---|---|---|
| **Nodi di Frontiera (Edge)** | ~60% | **76 nodi** | Dislocati nei punti critici di transito e stazionamento antropico. Bassa potenza radio, massima densità sul terreno. |
| **Nodi di Dorsale (Backbone)** | ~30% | **38 nodi** | Posizionati in quota o all'interno di schermature d'orizzonte. Fanno da ripetitori a lungo raggio (Sub-GHz) per unire i cluster periferici. |
| **Nodi Sentinella / Canali di Uscita (Gateways)** | ~10% | **13 nodi** | Nodi terminali mobili o ad assetto variabile, destinati a trasferire il digest finale normalizzato verso la topologia superiore del protocollo. |
## 3. Strategia di Deployment sul Terreno
Per evitare che la distribuzione sia identificabile tramite analisi statistiche dei pattern geospaziali da parte di osservatori esterni, il posizionamento finale deve applicare un coefficiente di **disordine calibrato (rumore di Poisson)**.
 1. **Evitare la simmetria geometrica perfetta:** La griglia esagonale teorica deve essere deformata sfruttando le barriere fisiche dell'ambiente (edifici, variazioni topografiche). I nodi devono sfruttare il camuffamento passivo dentro le scatole di derivazione industriale già presenti.
 2. **Asincronia di attivazione:** I nodi non devono essere accesi tutti contemporaneamente. La distribuzione deve avvenire a ondate successive per permettere alle tabelle di routing dinamiche del firmware (es. protocollo *B.A.T.M.A.N. advanced*) di mappare i cammini minimi in modo organico e asimmetrico.
 3. **Isolamento dei canali:** Cluster adiacenti devono operare su sotto-canali LoRa differenti o utilizzare sequenze di *Frequency Hopping* (salto di frequenza) sincronizzate, riducendo l'impronta elettromagnetica complessiva e rendendo la rete indistinguibile dal normale rumore di fondo delle radiofrequenze urbane.

---

## Il fattore umano
Per convincere le persone a adottare NELO non si deve far leva sul senso di colpa ("dovete proteggere i vostri dati") o sulla paranoia, ma su **incentivi tangibili, gamification, design invisibile e utilità quotidiana**.

Ecco la strategia di adozione divisa in 4 direttrici cinetiche:

### 1. L'Incentivo del "Bene Rifugio" Digitale (Valore Economico)
La gente non capisce la privacy, ma capisce la scarsità e il costo delle risorse.

* **La Rete come Mercato dell'Energia/Banda:** I nodi mesh di NELO non devono servire solo a trasmettere l'Indice di Disagio $D$. In tempo di pace, la rete funziona come un'infrastruttura di condivisione di connettività locale Hyper-Local (es. file-sharing di quartiere, chat locali a costo zero senza internet, routing di sensori meteo o smart-home open source).
* **Il "Dividendo di Resilienza":** Chi ospita un nodo mesh sul proprio balcone non lo fa per eroismo, ma perché quel nodo gli garantisce l'accesso a una rete di backup completamente gratuita, indipendente dalle fluttuazioni e dai costi dei grandi operatori di telecomunicazioni.

### 2. Il "Design dell'Invisibilità" (Zero Friction UX)
Se per usare NELO devi configurare chiavi crittografiche su Linux, hai già perso il 99% degli utenti.

* **Effetto Wearable Commerciale:** Il cripto-sensore antropico non deve sembrare un "dispositivo di resistenza cyberpunk". Deve essere integrato in anelli eleganti, braccialetti fitness low-cost o patch adesive sportive. Deve fare esattamente quello che fa un Fitbit o un Oura Ring: tracciare il sonno, i passi e lo stress personale (tramite un'app locale che legge i dati sul telefono, isolata dal mondo).
* **Adozione Parassitaria:** L'utente compra l'anello perché "è bello e monitora lo stress quotidiano in locale senza inviare dati a server cinesi o americani". Il fatto che quel dispositivo includa il chip nRF52840 configurato con il protocollo NELO e la chiave $SK_{sensor}$ per la difesa collettiva è una **proprietà latente di sfondo**. L'utente protegge la collettività semplicemente vivendo la sua vita.

### 3. La Teoria dei Giochi dell'Effetto Rete (Network Effects)
Nessuno vuole essere l'unico in una piazza a usare un sistema di difesa. La sicurezza percepita aumenta con la densità della massa.

* **La Strategia dei Piccoli Gruppi (I Core Cells):** L'onboarding non si fa "massa per massa", ma per comunità affini che hanno già un incentivo intrinseco a proteggersi o a comunicare a basso costo (es. comunità di escursionisti, cicloattivisti, network di quartiere, gruppi di studenti, organizzazioni umanitarie).
* Quando una massa critica locale (circa il **5% della popolazione di un'area**) adotta l'infrastruttura, si attiva l'effetto rete: la maglia mesh diventa abbastanza densa da coprire l'intera area geografica e la latenza nominale scende. A quel punto, aggregarsi diventa conveniente per chiunque per il semplice fatto che "funziona meglio ed è gratis".

### 4. Ribaltare la Narrazione: Da "Protezione" a "Infrastruttura di Orgoglio"
La privacy è spesso percepita come qualcosa di passivo o addirittura di "sospetto" ("se non hai nulla da nascondere..."). NELO deve essere posizionato come uno status-symbol di autonomia e sovranità tecnologica.

* **Gamification della Copertura Rete:** Gli utenti dei nodi mesh possono visualizzare mappe di calore locali (completamente anonime e aggregate) che mostrano il "Livello di Attrito" della propria città. Vedere la mappa del proprio quartiere colorarsi di "Verde Resiliente" (copertura mesh stabile) crea un senso di partecipazione comunitaria analogo a quello dei progetti di Citizen Science o di mappatura open-source (come OpenStreetMap).
* **Il Brand dell'Inviolabilità:** Indossare il sensore NELO deve diventare il corrispettivo digitale di un'armatura invisibile. Non lo indossi perché hai paura, lo indossi perché **rendi la tua comunità strutturalmente non governabile con la forza**.

---

### In sintesi: Il Cavallo di Troia Operativo

Per convincere la popolazione non devi vendere la "Resistenza passiva contro l'inferno distopico". Devi vendere:

1. Un anello per il fitness elegante che non vende i tuoi dati sanitari alle assicurazioni.
2. Una rete di messaggistica ed energia locale che funziona gratis anche se salta la luce o internet.

Una volta distribuito l'hardware tramite questi incentivi quotidiani, il protocollo NELO è pronto. La trappola crittografica per gli oppressori è piazzata, silenziosa e invisibile, in attesa di non dover mai essere attivata.
 
