# Progetto NELO: Strategia Esecutiva di Crowdfunding (Fase 1)

**Stato:** OPERATIVO / ALPHA  
**Obiettivo Finanziario Innesco:** €30.000 - €45.000 (Finanziamento prima cella metropolitana pilota)  
**Modello:** Reward-Based (Pre-ordine basato sul prodotto) con allocazione parassitaria dei margini  
**Canale di Lancio:** Piattaforma indipendente non censurabile o portale Web3 self-hosted  

#### Questo documento definisce l'esecuzione tattica della Fase 1. Per l'analisi filosofica e la roadmap di scaling globale in 3 fasi, fare riferimento a [crowdfunding.md](./crowdfunding.md).
---

## 1. Analisi di Tolleranza e Variabilità del Capitale di Fabbricazione (CapEx)

Per blindare la sostenibilità del volano finanziario a fronte delle oscillazioni internazionali dei prezzi dei semiconduttori (SoC nRF52840, chip LoRa SX1262) e dei materiali (celle LTO, resine epossidiche caricate), il piano economico introduce un **margine di variabilità di ±€3,00** sul costo nominale di produzione del braccialetto, fissando il prezzo al pubblico a **€30,00**.

### Forchetta di Fluttuazione Industriale (Prezzo di Vendita Fisso a €30,00)

| Scenario di Supply Chain | Costo Industriale Bracciale | Margine Lordo Residuo | Impatto Operativo sulla Rete Mesh |
| :--- | :--- | :--- | :--- |
| **Scenario A (Stress di Mercato: +€3,00)** | €15,00 | **€15,00** | Il margine non copre interamente il costo di un Router Alpha (€18,00). Il deficit di €3,00 viene compensato dai margini generati dai Tier superiori (Maker Kit e Celle Locali). |
| **Scenario B (Costo Nominale)** | €12,00 | **€18,00** | **Equilibrio perfetto (Rapporto 1:1).** Il margine di un singolo braccialetto copre esattamente ed interamente il costo di fabbricazione di un Router Alpha di superficie (€18,00). |
| **Scenario C (Ottimizzazione Lotti: -€3,00)** | €9,00 | **€21,00** | **Surplus di Rete.** Ogni braccialetto venduto finanzia un Router Alpha (€18,00) e genera un extra netto di €3,00 destinato al fondo di logistica protetta e audit crittografici. |

---

## 2. Architettura dei Reward Tiers (Livelli di Ricompensa)

La struttura dei pacchetti differenzia l'offerta tra l'utente mainstream (attratto dal binomio arte/privacy e dal monitoraggio biologico locale zero-cloud) e il nucleo dei validatori tecnici (maker e custodi della rete).

### Schema dei Pacchetti di Lancio

#### Tier 1: L'Innesco Popolare (Wearable Singolo) — €30,00
* **Contenuto:** 1× Cripto-Braccialetto Edge + Applicazione Mobile per l'analisi locale dei dati sanitari (HRV, stress, sonno).
* **Valore Strategico:** Abbassa drasticamente la barriera d'ingresso per la popolazione non tecnica. Serve a generare volume, passaparola sul territorio e a raggiungere la densità geometrica minima di sensori biometrici per quadrante urbano.

#### Tier 2: Il Custode della Rete (Maker Kit) — €55,00
* **Contenuto:** 1× Cripto-Braccialetto Edge + 1× Kit Hardware Router Alpha (Componenti COTS standard: ESP32-S3, SX1262, PMIC BQ25504, antenna + file STL per la stampa 3D del guscio).
* **Valore Strategico:** Destinato alla community degli sviluppatori e dei maker. Fornisce gli strumenti per accendere i primi nodi stazionari sul territorio, unendo i dispositivi in un'unica spedizione logistica per abbattere i costi.

#### Tier 3: La Cella Metropolitana (Pack Resilienza) — €80,00
* **Contenuto:** 2× Cripto-Braccialetto Edge + 1× Router Alpha pre-assemblato e calibrato, pronto all'installazione in ambiente esterno.
* **Valore Strategico:** Il vero acceleratore dell'infrastruttura. Permette a piccoli nuclei, famiglie o collettivi locali di blindare elettromagneticamente una piazza o una via fin dal primo giorno, massimizzando il margine di sicurezza.

---

## 3. Ripartizione Finanziaria dei Flussi (Modello Nominale)

La gestione dei capitali raccolti durante la campagna non segue logiche speculative aziendali, ma viene distribuita in modo matematico per garantire l'autosufficienza della rete:


```

[ FONDO PRE-ORDINI CROWDFUNDING ]
|
v
+------------+------------+
| Prezzo del Bracciale    | -> €30,00
+------------+------------+
|
v
+------------+------------+
| CapEx Produzione Corrente| -> €12,00 (Forchetta: €9,00 - €15,00)
+------------+------------+
|
v Margine Lordo: €18,00 (Scenario Nominale)
|
v
+------------+------------+
| FINANZIAMENTO RETE MESH | -> €18,00 -> Copre 100% dei costi di un
| (Sottoscrizione Parassita)|               Router Alpha di Superficie
+-------------------------+

```

1. **Allocazione Router Automatico (Scenario Nominale):** €18,00 estratti da ciascun braccialetto venduto vengono immediatamente convertiti nell'acquisto dei componenti dei Router Mesh Alpha, che verranno distribuiti gratuitamente ai validatori geolocalizzati nelle zone a bassa densità di copertura.
2. **Assorbimento delle Fluttuazioni:** Eventuali extra generati dallo Scenario C (€3,00 per unità) confluiscono in un conto di riserva protetto per neutralizzare le perdite dello Scenario A e coprire i costi vivi delle spedizioni frazionate.

---

## 4. Linee Guida di Comunicazione e Narrazione Asimmetrica

Il messaggio della campagna deve scardinare la retorica commerciale delle Big Tech, puntando sulla durabilità e sulla sovranità dei dati:

* **Contro la Profilazione Cloud:** Sottolineare che i wearable commerciali estraggono l'intimità biologica per scopi commerciali. Il bracciale NELO, invece, elabora tutto sul silicio al polso. La privacy è un vincolo hardware, non una promessa legale.
* **Contro l'Obsolescenza Programmata:** Evidenziare che l'uso di celle Litio-Titanato (LTO) e la sigillatura monoblocco in resina estendono la vita utile del dispositivo a oltre 20 anni, rendendolo un investimento permanente per la propria sicurezza e benessere.
* **L'Arte come Scudo:** L'apporto artistico e l'assenza di display mimetizzano il dispositivo, rendendolo un accessorio esclusivo ed elegante per il pubblico mainstream, e al contempo invisibile agli analizzatori di spettro avversari.

