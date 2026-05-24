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
