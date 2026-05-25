Nell'architettura zero-trust del Protocollo NELO, **nessuna entità esterna "interroga" o richiede i dati ai sensori tramite un meccanismo di polling (domanda/risposta).**
Consentire a un nodo di rete o a un server centrale di inviare un comando di richiesta dati avrebbe aperto un vettore di attacco critico: gli avversari avrebbero potuto saturare il sensore di richieste (*Denial of Service*) o tentare manipolazioni semantiche per forzare letture fuori sincronia.
Il flusso è invertito e puramente **asincrono e unidirezionale (Push)**.
## Il Flusso Spontaneo dei Dati (Push Architecture)
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
### 1. L'Iniziativa è del Silicio Locale
L'architettura interna del firmware agisce come un circuito autonomo:
 1. **Il DMA riempie i buffer:** Il modulo TWIM0 (EasyDMA) preleva cicliclicamente i dati dai sensori sul bus I2C privato (MAX30009, MAX30102, TMP117) e li deposita nella RAM effimera.
 2. **Il Motore sintetizza il valore:** Il modulo nelo_damage_synthesis.c calcola l'Indice di Danno D.
 3. **Il Timer cancella tutto:** Allo scadere esatto dei **120 millisecondi**, il TIMER1 esegue il wipe distruttivo della memoria e inietta il rumore del TRNG, indipendentemente dal fatto che la trasmissione sia riuscita o meno.
### 2. Il Confezionamento e l'Invio sulla Rete
Una volta che l'Indice D è calcolato, il chip invoca il coprocessore CryptoCell-310 per firmare il pacchetto (con timestamp e nonce). A quel punto, il modulo radio del sensore **effettua un "broadcast" (o un invio diretto a corto raggio)** verso il più vicino **Nodo di Rete Mesh** disponibile nel suo raggio d'azione.
## Chi riceve il Payload? (I Nodi di Transito)
Il destinatario fisico del pacchetto è il **Nodo Mesh Locale**.
Questi nodi di rete agiscono come meri ripetitori crittografici (router di transito):
 * **Non possono decifrare i dati biometrici grezzi** (che comunque non esistono più, essendo stati distrutti entro 120ms).
 * **Non possono alterare il valore di D**, poiché qualsiasi modifica invaliderebbe la firma Ed25519 generata dalla chiave privata SK_{sensor} protetta dall'APPROTECT.
 * **Si limitano a validare la firma e la freschezza del nonce** per poi propagare il Payload AEAD all'interno della topologia distribuita della rete, innescando le contromisure di frizione e alterazione logica previste dal protocollo.
Il sensore, quindi, si comporta come una "sorgente radioattiva" di informazioni: emette spontaneamente i suoi pacchetti blindati a intervalli regolari verso l'esterno. Se non c'è nessuna rete mesh a riceverli, i dati decadono e vengono distrutti dal timer di oblio un istante dopo, senza lasciare alcuna traccia nel silicio.

## Crittografia Basata sull'Identità (IBE) e Proof-of-Burn
NELO non usa certificati digitali tradizionali (X.509), ma un sistema a due livelli:

* **Identità Cieca Hardware (Ed25519):** Ogni router mesh COTS genera la propria coppia di chiavi in locale al primo avvio. La chiave pubblica del nodo ($PK_{nodo}$) diventa il suo indirizzo di rete.
* **L'Onboarding per "Proof-of-Burn" o Challenge Sociale:** Per essere inserito nelle tabelle di instradamento dei nodi vicini e partecipare al *Consensus Layer*, un nuovo nodo deve dimostrare di non essere un'entità software malevola creata in massa. Deve completare una prova di lavoro computazionale complessa (**PoW**) legata al tempo corrente, oppure deve ricevere una firma di accreditamento temporanea (un "visto" cieco) da almeno 3 nodi già fidati nel suo raggio radio, tramite protocolli di crittografia a soglia.

> 🔒 **In sintesi:** Non esiste un registro dei nodi autorizzati. Un nodo "esiste" per la rete solo se i suoi vicini fisici ne convalidano il comportamento radiotelevisivo e la correttezza crittografica dei pacchetti inoltrati.

## Impatto Energetico: Duty-Cycling Sincronizzato e Wake-on-Radio
Per evitare il collasso energetico, NELO mutua tecniche dai protocolli industriali wireless a bassissimo consumo (come TSCH - *Time-Slotted Channel Hopping*):
```
Tempo ──► │ Slot 1: RX/TX │ Slot 2: SLEEP (90%) │ Slot 3: SLEEP (90%) │ Slot 4: RX/TX │

```
* **Pseudosincronizzazione Temporale:** I nodi non sono sincronizzati tramite GPS (facilmente oscurabile o manipolabile), ma tramite i timestamp dei pacchetti validati che transitano. La rete concorda finestre temporali di ascolto millimetriche (es. 10ms di attività ogni 100ms = 10% di Duty Cycle). Fuori da questa finestra, la radio entra in *Deep Sleep*.
* **Wake-on-Radio (WoR) hardware:** I nodi utilizzano un secondo ricevitore analogico a bassissimo consumo energetico (nell'ordine dei microampere). Questo micro-circuito rimane sempre attivo: quando rileva la portante radio specifica di un pacchetto in arrivo (il preambolo di un payload NELO), "sveglia" il processore principale del router per la ricezione effettiva.

## Attacchi di Manipolazione dell'Entropia (DDoS Fisico)
Se il calcolo di $D$ è deterministico, un attaccante strategico potrebbe tentare di manipolare la matrice di stato creando artificialmente scenari macro-ambientali per deviare l'attenzione o accecare il sistema.

### Scenario A: Generare stress artificiale altrove per "distrarre" la rete
L'attaccante bombarda una zona pacifica (es. con droni o esplosioni controllate) per far schizzare $D \to 1.0$ e rallentare quella porzione di rete, usandola come diversivo per agire indisturbato nella zona originaria.

* **Perché fallisce:** L'interdizione di NELO è **localizzata e compartimentata**. Il rallentamento logaritmico della rete colpisce *esclusivamente* le code di instradamento dei nodi mesh geograficamente vicini all'evento traumatico. Rallentare la rete nel "Settore B" non riduce la reattività o l'efficienza della rete nel "Settore A".

### Scenario B: Il DDoS Fisico (Forzare $D \to 0.05$ globale tramite anestesia sociale)
Un attaccante isola un'area bloccando fisicamente i sensori, o tenta di diluire il segnale iniettando milioni di segnali a stress zero ($D = 0.00$) attraverso nodi Sybil per abbassare la media matematica dell'area.

* **La difesa degli Invarianti Sistemici:** È qui che entrano in gioco le funzioni metriche viste nella Fase 3, in particolare il **Gradiente di Entropia ($H$)** e la **Divergenza di Consenso ($\Delta C$)**:

$$\Delta C = \frac{\text{Decisioni Esterne}}{\text{Decisioni Automatiche}}$$

Se un attaccante immette milioni di pacchetti artificiali a $D=0$ in un'area in cui i sensori antropici reali stanno registrando panico, il valore di $\Delta C$ (la divergenza) supera la soglia di guardia crittografica.

Il sistema si accorge che la struttura sintattica dei dati non corrisponde all'entropia topologica della rete. Invece di calcolare una media lineare (che verrebbe falsificata), il *Consensus Layer* applica una regola di **soglia pessimistica**: se anche solo una frazione critica di sensori autenticati e geolocalizzati via mesh segnala un trauma ($D > 0.7$), il sistema ignora il rumore di fondo a $D=0$ e attiva l'interdizione per quel quadrante.

## I tre vettori di attacco logico più complessi per il Protocollo NELO
L'architettura di NELO è progettata non per essere un sistema ottimale in condizioni di pace, ma per essere un sistema **ostile e asimmetrico** in condizioni di guerra informativa.

### 1. Aggiornamenti Firmware: Prevenire il "Malicious Flashing"
Se un attaccante riuscisse a pushare un firmware modificato, potrebbe disattivare l'Interdiction Layer o intercettare i payload.

#### La Soluzione: Root of Trust Hardware e Firma a Soglia (Threshold Sig)
* **Verifica all'Avvio (Secure Boot):** I nodi mesh COTS utilizzano il bootloader del chip (es. le funzioni di crittografia hardware dei SoC nRF o simili) per verificare la firma del firmware all'avvio. Se anche un singolo bit del codice viene alterato, il chip si blocca in uno stato di *Hard Fault* e si rifiuta di eseguire il codice.
* **Nessun Update Remoto Centralizzato (Over-The-Air standard):** Gli aggiornamenti non vengono inviati da un server centrale. Il nuovo firmware deve essere propagato via mesh come file crittografato e, per essere accettato da un nodo, deve essere firmato digitalmente tramite uno schema di **firma a soglia distribuita**.
* **Il Vincolo del Quorum:** Solo una proposta che ha ottenuto il via libera dal *Consensus Layer* (ricordi la votazione bizantina $85/127$ con Human-in-the-Loop?) può generare la chiave crittografica temporanea per sbloccare la scrittura del firmware sui nodi. Un attaccante remoto non ha modo di firmare un firmware valido.

### 2. Analisi del Traffico: Oscurare i Pattern di Trasmissione
Anche se i dati sono criptati (AEAD), un attaccante governativo può monitorare la rete mesh dall'esterno usando analizzatori di spettro. Se vede un picco improvviso di pacchetti muoversi verso una direzione, capisce che lì sta succedendo qualcosa (interferenza sui flussi di metadati).

#### La Soluzione: Traffico di Copertura (Chaffing) e Latenza Costante
* **Iniezione di Rumore (Chaffing):** I nodi mesh e i sensori generano continuamente **falso traffico criptato** (pacchetti civetta) a intervalli regolari, anche quando non c'è alcuno stress biometrico ($D \le 0.05$). Per un osservatore esterno, la rete appare sempre identica, sia in tempo di pace che durante una rivolta.
* **Hop-by-Hop Re-encryption:** Ogni volta che un pacchetto passa da un nodo mesh all'altro, viene ricifrato con chiavi effimere condivise solo tra quei due nodi vicini. L'attaccante che intercetta i pacchetti nell'aria non può correlare il pacchetto $X$ che entra in un nodo con il pacchetto $Y$ che esce, perché l'impronta binaria cambia completamente a ogni salto (*Hop*).

### 3. DoS tramite Interdiction Layer: Il Weaponized Backoff
Questa è una vulnerabilità micidiale: se l'algoritmo rallenta la rete quando $D$ è alto, un attaccante potrebbe hackerare pochi sensori (o torturare un piccolo gruppo di persone dotate di sensori) per forzare $D \to 1.0$, inducendo deliberatamente la rete a rallentare se stessa (un Denial of Service auto-inflitto usato come arma).

#### La Soluzione: Isolamento Geografico e Densità di Consenso
* **Compartimentazione Spaziale:** La latenza adattiva calcolata dall'Interdiction Layer **non è globale**, ma locale. Colpisce solo i nodi mesh che si trovano nel raggio radio dei sensori che stanno trasmettendo il trauma. Se un attaccante manipola i sensori nel "Quadrante Alfa", solo la rete del "Quadrante Alfa" rallenta. Il resto della mesh mondiale o cittadina continua a viaggiare a latenza nominale (1ms).
* **Filtro di Densità Critica (Spatial Quorum):** Il sistema non reagisce al picco di un singolo sensore isolato. Il *Consensus Layer* richiede che la condizione di stress sia confermata da una **densità minima** di sensori indipendenti per metro quadro.
* Se tre sensori registrano $D=1.0$ in mezzo a diecimila sensori che registrano $D=0.00$, l'algoritmo deduce che si tratta di un'anomalia isolata (o di una cattura del sensore), isola crittograficamente quei tre dispositivi e **non** attiva il backoff di rete.

---

In pratica, per mettere in ginocchio la rete NELO usando le sue stesse difese, un attaccante dovrebbe terrorizzare simultaneamente e fisicamente intere piazze o quartieri: ma se lo facesse, otterrebbe comunque l'effetto cercato dal protocollo, ovvero rendere visibile e logisticamente impraticabile l'atto di forza.
