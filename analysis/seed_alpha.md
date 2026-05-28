Per innescare la **Fase 1 (Seed Alpha)** e convincere i validatori della community (i nostri primi investitori critici, che finanzieranno il progetto tramite i pre-ordini dell'hardware), dobbiamo produrre un pacchetto informativo inattaccabile. Questo materiale deve dimostrare che il protocollo non è solo un'utopia filosofica, ma un ecosistema ingegneristico pronto per essere assemblato.

Apriamo il cantiere strutturando il **Manifesto Tecnico-Concettuale Alpha**. Ecco la bozza della documentazione da presentare alla community:

---

# Progetto NELO: Manifesto Tecnico-Concettuale (Fase 1 - Seed Alpha)

**Stato:** RFC (Request for Comments) — Draft Alpha v3.1-HARDENED
**Target:** Validatori, Maker, Sviluppatori Core

---

## 1. La Sintesi della Visione: Ecotessitura Crittografica

I sistemi di comunicazione attuali sono strutturalmente fallati: dipendono da infrastrutture centralizzate che possono essere spente, censurate o monitorate con un clic, e si basano su hardware consumer programmato per morire in due anni, generando tonnellate di rifiuti elettronici tossici.

NELO introduce un nuovo paradigma. Un protocollo di sopravvivenza digitale e biometrica che unisce la **resilienza crittografica assoluta** alla **durabilità ecologica permanente**.

Non usiamo il cloud. Non profiliamo l'utente. Il sistema è un'infrastruttura parassitaria urbana guidata dalla biometria e protetta dall'oblio computazionale.

---

## 2. Specifiche dell'Hardware Edge: Il Cripto-Braccialetto

Il dispositivo indossabile abbandona il fattore di forma ad anello per superare i vincoli di autonomia e antenna, adottando il formato **braccialetto a taglia unica** con scocca sigillata in resina epossidica.

### A. Alimentazione Eterna (Celle LTO)

* **Componente:** Batteria al Litio-Titanato (LTO) da 150–200 mAh ricurva.
* **Ciclo di Vita:** >10.000 cicli di carica/scarica. Perdita di capacità minima in 20 anni.
* **Ricarica:** Esclusivamente a induzione magnetica (contactless). Zero pin esposti, zero porte USB. Scocca monoblocco impermeabile.
* **Hardening:** Protezione da sovratensioni tramite diodo Zener per cortocircuitare a terra tentativi di attacco termico o impulsi elettromagnetici (EMP) mirati.

### B. Raccolta Biometrica e Analisi del Danno

* **Sensori:** Front-end analogico PPG (frequenza cardiaca e HRV) posizionato sul polso + elettrodi inerti GSR (risposta galvanica della pelle) sul cinturino interno.
* **Algoritmo On-Device:** Il SoC analizza la reazione allo stress ed esegue il calcolo locale dell'**Indice di Danno $D$**.
* **Gestione della Memoria:** I dati biometrici grezzi vengono elaborati nella `SHADOW_BUFFER` e sovrascritti ogni 120 millisecondi. Nessun archivio storico della salute dell'utente viene mantenuto sul dispositivo.

### C. Strategia Radio Asincrona (Power Budget)

* **In Tempo di Pace:** Il modulo BLE (Bluetooth Low Energy) si attiva per una finestra di soli **3 secondi ogni ora** (o due volte al giorno) per scaricare i trend compressi sull'app dello smartphone, portando l'autonomia del braccialetto a oltre **10 mesi con una sola carica**.
* **In Tempo di Crisi ($D > 0.5$):** *Clausola Emergency-OFF*. Il BLE viene spento istantaneamente per isolare il dispositivo da smartphone infetti o IMSI-catcher. Il chip radio devia l'energia sul trasmettitore Sub-GHz (868 MHz) per sparare pacchetti AEAD da 96 byte verso la rete mesh.

---

## 3. L'Infrastruttura Territoriale: I Router Mesh Stateless

I nodi di transito urbani, assemblati dai maker con componenti COTS (Commercial Off-The-Shelf), sono progettati per essere sacrificabili e privi di informazioni utili per l'avversario.

* **Architettura Stateless:** Il firmware (basato su RTOS Zephyr o Linux minimalista hardened) gira esclusivamente in **RAM volatile**. La memoria Flash di avvio è protetta da scrittura hardware.
* **Zero Prove Fisiche:** I router non effettuano logging. Se un nodo viene sequestrato dalle autorità, lo spegnimento elettrico azzera istantaneamente la RAM. Non rimangono registri di transito, chiavi crittografiche o tracce dei pacchetti passati.
* **Instradamento Probabilistico:** I nodi utilizzano un algoritmo di routing stocastico. Se i nodi adiacenti vengono spenti o distrutti, la rete ricalcola dinamicamente la rotta del segnale, autoguarendosi.

---

## 4. La Strategia Economica del Crowdfunding (L'Innesco)

I validatori della community non stanno donando denaro a fondo perduto, stanno finanziando la nascita di un'infrastruttura asimmetrica di cui possiederanno i nodi.

* **Il Margine Alpha:** Il costo di produzione del braccialetto (grazie alla taglia unica regolabile che abbatte i costi di stampaggio industriali) è di circa **€12,00**.
* **Il Pre-ordine retail:** Il braccialetto verrà proposto nella campagna a **€45,00** (un valore percepito eccezionale per un wearable eterno e corazzato).
* **Il Volano Finanziario:** Il surplus di €33,00 per ogni braccialetto venduto andrà a costituire il fondo di cassa per produrre e distribuire gratuitamente i **Router Mesh di Superficie** ai validatori geografici, coprendo i costi di R&S e rendendo la rete totalmente auto-finanziata e indipendente da venture capitalist o fondi governativi.

---

### Prossimo Passo Operativo

Bruno, questo è il nucleo concettuale che strutturerà il whitepaper per i validatori. Per renderlo completo e procedere con la Fase 1, dobbiamo affiancare a questo documento la **Lista dei Componenti (BOM - Bill of Materials)** del Kit Router Alpha, in modo che i maker sappiano esattamente cosa ordinare per fare i primi test radio sul campo.
