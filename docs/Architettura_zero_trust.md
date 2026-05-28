## ARCHITETTURA ZERO-TRUST
Nell'architettura zero-trust del Protocollo NELO, **nessuna entità esterna "interroga" o richiede i dati ai sensori tramite un meccanismo di polling (domanda/risposta).**
Consentire a un nodo di rete o a un server centrale di inviare un comando di richiesta dati avrebbe aperto un vettore di attacco critico: gli avversari avrebbero potuto saturare il sensore di richieste (*Denial of Service*) o tentare manipolazioni semantiche per forzare letture fuori sincronia.
Il flusso è invertito e puramente **asincrono e unidirezionale (Push)**.
### Il Flusso Spontaneo dei Dati (Push Architecture)
È il Cripto-Sensore Antropico stesso che, governato esclusivamente dai suoi timer hardware interni, decide quando raccogliere le metriche, sintetizzarle ed espellerle verso l'esterno.
```
+--------------------------+
|  Cripto-Sensore (Push)   |
|  [Timer Interno]         |
+------------┬-------------+
             │
             │ (Payload AEAD Firmato)
             ▼
+--------------------------+
|   Nodo Mesh Locale       |
|   (Router di Transito)   |
+------------┬-------------+
             │
             ▼
      [ Topologia NELO ]

```
#### 1. L'Iniziativa è del Silicio Locale
L'architettura interna del firmware agisce come un circuito autonomo:
 1. **Il DMA riempie i buffer:** Il modulo TWIM0 (EasyDMA) preleva cicliclicamente i dati dai sensori sul bus I2C privato (MAX30009, MAX30102, TMP117) e li deposita nella RAM effimera.
 2. **Il Motore sintetizza il valore:** Il modulo nelo_damage_synthesis.c calcola l'Indice di Danno D.
 3. **Il Timer cancella tutto:** Allo scadere esatto dei **120 millisecondi**, il TIMER1 esegue il wipe distruttivo della memoria e inietta il rumore del TRNG, indipendentemente dal fatto che la trasmissione sia riuscita o meno.
#### 2. Il Confezionamento e l'Invio sulla Rete
Una volta che l'Indice D è calcolato, il chip invoca il coprocessore CryptoCell-310 per firmare il pacchetto (con timestamp e nonce). A quel punto, il modulo radio del sensore **effettua un "broadcast" (o un invio diretto a corto raggio)** verso il più vicino **Nodo di Rete Mesh** disponibile nel suo raggio d'azione.
### Chi riceve il Payload? (I Nodi di Transito)
Il destinatario fisico del pacchetto è il **Nodo Mesh Locale**.
Questi nodi di rete agiscono come meri ripetitori crittografici (router di transito):
 * **Non possono decifrare i dati biometrici grezzi** (che comunque non esistono più, essendo stati distrutti entro 120ms).
 * **Non possono alterare il valore di D**, poiché qualsiasi modifica invaliderebbe la firma Ed25519 generata dalla chiave privata SK_{sensor} protetta dall'APPROTECT.
 * **Si limitano a validare la firma e la freschezza del nonce** per poi propagare il Payload AEAD all'interno della topologia distribuita della rete, innescando le contromisure di frizione e alterazione logica previste dal protocollo.
Il sensore, quindi, si comporta come una "sorgente radioattiva" di informazioni: emette spontaneamente i suoi pacchetti blindati a intervalli regolari verso l'esterno. Se non c'è nessuna rete mesh a riceverli, i dati decadono e vengono distrutti dal timer di oblio un istante dopo, senza lasciare alcuna traccia nel silicio.

### Crittografia Basata sull'Identità (IBE) e Proof-of-Burn
NELO non usa certificati digitali tradizionali (X.509), ma un sistema a due livelli:

* **Identità Cieca Hardware (Ed25519):** Ogni router mesh COTS genera la propria coppia di chiavi in locale al primo avvio. La chiave pubblica del nodo ($PK_{nodo}$) diventa il suo indirizzo di rete.
* **L'Onboarding per "Proof-of-Burn" o Challenge Sociale:** Per essere inserito nelle tabelle di instradamento dei nodi vicini e partecipare al *Consensus Layer*, un nuovo nodo deve dimostrare di non essere un'entità software malevola creata in massa. Deve completare una prova di lavoro computazionale complessa (**PoW**) legata al tempo corrente, oppure deve ricevere una firma di accreditamento temporanea (un "visto" cieco) da almeno 3 nodi già fidati nel suo raggio radio, tramite protocolli di crittografia a soglia.

> 🔒 **In sintesi:** Non esiste un registro dei nodi autorizzati. Un nodo "esiste" per la rete solo se i suoi vicini fisici ne convalidano il comportamento radiotelevisivo e la correttezza crittografica dei pacchetti inoltrati.

### Impatto Energetico: Duty-Cycling Sincronizzato e Wake-on-Radio
Per evitare il collasso energetico, NELO mutua tecniche dai protocolli industriali wireless a bassissimo consumo (come TSCH - *Time-Slotted Channel Hopping*):
```
Tempo ──► │ Slot 1: RX/TX │ Slot 2: SLEEP (90%) │ Slot 3: SLEEP (90%) │ Slot 4: RX/TX │

```
* **Pseudosincronizzazione Temporale:** I nodi non sono sincronizzati tramite GPS (facilmente oscurabile o manipolabile), ma tramite i timestamp dei pacchetti validati che transitano. La rete concorda finestre temporali di ascolto millimetriche (es. 10ms di attività ogni 100ms = 10% di Duty Cycle). Fuori da questa finestra, la radio entra in *Deep Sleep*.
* **Wake-on-Radio (WoR) hardware:** I nodi utilizzano un secondo ricevitore analogico a bassissimo consumo energetico (nell'ordine dei microampere). Questo micro-circuito rimane sempre attivo: quando rileva la portante radio specifica di un pacchetto in arrivo (il preambolo di un payload NELO), "sveglia" il processore principale del router per la ricezione effettiva.

---

### Architettura cyber-fisica basata sulla scarsità geografica

#### 1. Oltre la Blockchain: DAG e Proof-of-Burn Localizzata
Una blockchain lineare tradizionale è troppo pesante per piccoli router COTS con memoria volatile e consumerebbe troppa energia. NELO preferisce una struttura a **DAG (Directed Acyclic Graph)**, simile a quella di protocolli come IOTA, ma modificata per riflettere lo spazio fisico.

* **Onboarding via Proof-of-Burn Computazionale (Anti-Sybil):** Per registrare la propria chiave pubblica nella mesh locale, un nuovo nodo deve risolvere un puzzle crittografico (PoW) la cui difficoltà scala esponenzialmente con il numero di nodi che tentano di registrarsi dalla stessa macro-area nello stesso intervallo di tempo. Creare 10.000 nodi virtuali sulla stessa antenna diventa computazionalmente proibitivo.
* **Consenso nel DAG senza Token:** Non ci sono monete. Per inserire il proprio pacchetto di dati nel DAG, ogni nodo deve convalidare e "attaccarsi" a due transazioni precedenti di nodi vicini. Se un attaccante tenta un attacco al 51% creando transazioni false, dovrebbe generare abbastanza potenza radio da sovrastare l'intera attività d'area di tutti gli altri nodi fisici legittimi.

#### 2. Reti Mesh Avanzate: Il Filtro della Scarsità Spaziale (Spatio-Temporal BFT)
In una rete puramente digitale, l'attacco 51% è un problema di puro calcolo o capitale. In una rete mesh radio (LoRa/Sub-GHz), l'attacco deve fare i conti con le leggi della fisica: **la propagazione delle onde radio e la posizione geografica**.

* **Rilevamento dell'Impronta di Segnale (RSSI/ToA):** I nodi mesh vicini analizzano l'intensità del segnale (*RSSI*) e il tempo di arrivo (*Time of Arrival*) di ogni pacchetto radio. Se l'attaccante crea 500 nodi Sybil virtuali usando lo stesso hardware, tutti i pacchetti arriveranno ai nodi circostanti con la stessa identica impronta radio e dalla stessa direzione. I nodi vicini identificano l'anomalia statistica e collassano quei 500 nodi in un unico voto logico.
* **Il Quorum Geografico:** Il *Consensus Layer* di NELO non estrae i 127 validatori da un pool logico globale (dove l'attaccante potrebbe aver diluito la rete con nodi fake), ma li estrae in base a una matrice di diversità spaziale. Il quorum richiede firme da nodi che appartengono a **celle geografiche mesh distinte e verificate dal posizionamento dei salti radio (Hops)**. Per ottenere il 51% di questo consenso, l'attaccante non deve solo generare software, ma deve occupare fisicamente il 51% del territorio con antenne reali.

#### 3. Community-Driven: Incentivi ed Economia della Reputazione Libera
La difesa a lungo termine contro la cattura della rete non è crittografica, ma sociale. Gli incentivi per i contributori devono essere strutturati per premiare l'onestà e rendere il sabotaggio economicamente svantaggioso.

* **La Proof-of-Time-in-Service (Reputazione):** Il peso del voto di un nodo all'interno del meccanismo di consenso bizantino non è legato a quanti token possiede, ma a quanto tempo è rimasto attivo e coerente nella rete senza generare anomalie di traffico o divergenze sintattiche ($\Delta C$). Un nodo "anziano" e stabile che ha instradato traffico legittimo per mesi ha un peso reputazionale maggiore rispetto a un nodo appena apparso. Questo azzera l'efficacia di attacchi "mordi e fuggi" o di botnet temporanee.
* **Incentivo di Quartiere (Sovereign Utility):** La comunità è incentivata a mantenere i nodi puliti perché la mesh distribuisce servizi ad utilità diretta (messaggistica crittografata locale in caso di blackout, monitoraggio ambientale open-source, condivisione di storage distribuito). Attaccare la rete NELO del proprio quartiere significa distruggere l'unico canale di comunicazione autonomo della propria comunità: l'incentivo sociale alla difesa fisica dei nodi (contro la rimozione o il sabotaggio) diventa una priorità collettiva.

