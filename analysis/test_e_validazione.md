# TEST E VALIDAZIONE

Per testare e validare un'infrastruttura asimmetrica e ostile come il Protocollo NELO senza esporre i partecipanti a rischi reali durante la fase di sviluppo, il piano di deployment e validazione prevede una strategia rigorosamente compartimentata. Non si può testare un sistema simile "in produzione" in uno scenario di crisi aperta senza aver prima quantificato matematicamente e fisicamente ogni singola metrica di vulnerabilità.

Il piano di validazione si articola su **4 fasi cinematiche incrementali**, muovendosi dall'isolamento software controllato fino alla simulazione sul campo ad alta fedeltà.

---

## Fase 1: Validazione in Ambiente Sintetico (Simulazione Digitale)

Prima di toccare qualsiasi componente hardware, l'intero protocollo viene mappato all'interno di un ambiente di simulazione discreta (utilizzando framework di rete come NS-3 o emulatori accoppiati a container Docker/Core).

* **Test del Backoff Logaritmico (`SPEC-001`):** Si simulano cluster di macro-aree con oltre 100.000 nodi virtuali. Vengono iniettati vettori di danno $D$ artificiali per verificare che la latenza delle code di instradamento scali esattamente secondo l'equazione a tratti stabilita, verificando la continuità della funzione nel punto di giunzione $D = 0.7$.
* **Iniezione di Attacchi Semantici (Stress-Test per `SPEC-009`):** Si implementano botnet di nodi Sybil virtuali che tentano di saturare la rete con messaggi falsificati a stress zero ($D = 0.00$) per mascherare un picco di trauma localizzato. Il test ha successo se il calcolo del Gradiente di Entropia $H$ fa scattare la *Soglia Pessimistica* entro i primi 3 cicli di sincronizzazione, isolando l'attacco.

---

## Fase 2: Validazione Hardware Edge ("In-the-Loop" Benchmarking)

In questa fase si passa all'hardware reale isolato in laboratorio per verificare la tenuta fisica dei chip e l'efficienza energetica.

* **Audit dell'Oblio Hardware (`SPEC-003` / `SPEC-004`):** I prototipi dei cripto-sensori (basati su chip nRF52840 o similari) vengono sottoposti a test di interruzione controllata. Si verifica tramite oscilloscopi e analizzatori logici che l'interrupt del timer a 120ms esegua il wipe della `SHADOW_BUFFER` con rumore TRNG prima che qualsiasi routine radio possa fare il leak dei dati biologici grezzi. Si applicano attacchi di tipo *Cold Boot* (congelamento della RAM) per dimostrare l'impossibilità di recuperare residui elettrici dei vettori bio-sensibili.
* **Benchmark dell'Early Drop Pipeline (`SPEC-008`):** Un generatore di segnali RF bombarda un nodo mesh COTS con pacchetti malformati o con timestamp obsoleti a frequenze di saturazione. Si misura l'assorbimento energetico del chip: il test è superato se il consumo rimane nell'ordine dei microampere, dimostrando che lo scarto a complessità $O(1)$ protegge la CPU del router dal DoS computazionale senza drenare la batteria.

---

## Fase 3: Scenari di Stress Controllato (Hobby-Scale / Citizen Science)

Il protocollo esce dal laboratorio per essere integrato in scenari civili pacifici ma caratterizzati da stress fisico, isolamento infrastrutturale e necessità di coordinamento autonomo.

* **Scenario A: Validazione Biometrica in Eventi Sportivi Estremi:** I cripto-sensori antropici vengono indossati da atleti durante gare di ultra-maratona, sessioni di apnea o simulazioni di soccorso alpino. Questo serve a calcolare la precisione dei convertitori SAADC nel mappare le variazioni reali di PPG (variabilità cardiaca) e GSR (risposta galvanica) traducendole correttamente nel fattore $D$ fixed-point Q1.15, minimizzando i falsi positivi generati da sforzi fisici consensuali rispetto al panico da trauma.
* **Scenario B: Mesh-Testing in Blackout Simulati:** Sfruttando network di quartiere o comunità di escursionisti, si disattiva intenzionalmente la rete cellulare/internet in una vallata o in un complesso edilizio isolato. I partecipanti utilizzano i nodi mesh per comunicare e scambiare dati a corto raggio. Si misura l'accuratezza del *Quorum Asincrono Probabilistico* (`SPEC-002`) e la stabilità del routing ad albero in condizioni di topologia dinamica fluttuante.

---

## Fase 4: Red Teaming e Validazione Avversaria Asimmetrica

L'ultima fase prevede l'ingaggio di un "Red Team" (un gruppo di esperti di sicurezza e penetration tester esterni) incaricato di agire come un attaccante governativo o un sabotatore industriale per scardinare la rete.

* **Attacco di Cattura del Quorum:** Il Red Team tenta di identificare e corrompere i 127 nodi estratti per la validazione di un aggiornamento del sistema. Il test convalida l'efficacia del protocollo se l'invisibilità crittografica dei validatori impedisce l'identificazione del target prima della scadenza della finestra di voto delle 72 ore.
* **Analisi del Traffico Elettromagnetico:** Il Red Team posiziona analizzatori di spettro industriali attorno all'area di test per mappare i pattern di trasmissione radio. NELO supera il test se gli attaccanti non riescono a distinguere visivamente o statisticamente i momenti di quiete dai momenti di picco di stress, confermando che il traffico di copertura (*Chaffing*) modellato sulla distribuzione stocastica di Poisson rende la rete una lavagna elettromagnetica omogenea e impenetrabile.

---

### Indicatori Chiave di Successo (KPI di Validazione)

Al termine di questi cicli di test, il protocollo viene considerato stabile e pronto per il deployment di massa solo se soddisfa i requisiti minimi estratti dalla RFC ufficiale:

1. **Harm Reduction Rate (HRR) > 0.80:** L'attivazione della frizione riduce l'efficienza logistica dei vettori avversari di almeno l'80% rispetto a un ambiente non protetto.
2. **Interdiction Accuracy (IA) > 0.95:** Meno del 5% di falsi positivi nell'innesco automatico del backoff d'area durante l'attività nominale.
3. **Memory Remanence Time = 0.00ms:** Certezza matematica dell'azzeramento della memoria biologica dopo lo scatto del timer hardware.

Questo piano metodologico garantisce che quando il framework NELO viene distribuito sul territorio, ogni sua contromisura sia già stata cinematicamente verificata contro le leggi della fisica e della teoria dei giochi.
