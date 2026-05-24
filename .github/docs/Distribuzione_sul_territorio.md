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
 
