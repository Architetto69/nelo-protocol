# Progetto NELO: Specifiche Industriali Cripto-Braccialetto Edge

**Stato:** SPEC / ALPHA  
**Versione:** 1.1 (Aggiornato con Canale Informativo Aptico e Mimetizzazione Artistica)  
**Target:** Designer industriali, Ingegneri dei materiali, Maker  

---

## 1. Visione Estetica e Mimetizzazione Artistica (Il Vettore "Cavallo di Troia")

Il braccialetto deve rompere totalmente con l'estetica fredda, plastica e standardizzata dei wearable commerciali delle Big Tech. La scocca esterna deve integrare il concetto di **"Ecotessitura Crittografica"**, coniugando materiali organici o minerali stabili a geometrie ispirate alle forme frattali naturali.

* **Materiali d'Apporto Artistico:** L'esterno del bracciale deve utilizzare materiali compositi non schermanti dal punto di vista radio (Sub-GHz), come il legno fossile stabilizzato, la ceramica tecnica microporosa, la pietra lavica o resine poliuretaniche caricate con polveri di quarzo, mica o ardesia. La texture superficiale deve apparire come un'opera scultorea, un pezzo di gioielleria contemporanea minimale o un accessorio materico unico.
* **Mimetizzazione dell'Interfaccia:** Non sono presenti schermi, porte USB esposte, fessure o pulsanti visibili. Questo azzera l'ansia da notifica, garantisce l'estetica pura dell'oggetto d'arte e impedisce a osservatori esterni di identificare il dispositivo come un apparato elettronico attivo.

---

## 2. Architettura Fisica e Corazzatura Monoblocco

I vincoli di sicurezza stabiliti nella `SPEC-006` e nella `SPEC-010` impongono l'inaccessibilità fisica ai componenti interni per neutralizzare i tentativi di analisi forense da laboratorio (*Hardware Tampering* e *Cold Boot Attacks* sulla RAM).

* **Sigillatura in Resina ad Alta Densità:** L'intera elettronica interna (SoC nRF52840, antenna Sub-GHz, sensori biometrici, micro-attuatore e batteria) viene posizionata nello stampo artistico e completamente colata in una resina epossidica o poliuretanica bicomponente ad alta densità, opacizzata con particelle minerali.
* **Autodistruzione Meccanica Passiva:** Una volta solidificata, la resina forma un blocco chimicamente e strutturalmente unito ai componenti e ai minuscoli reofori del silicio. Qualsiasi tentativo di fresatura, taglio manuale o scioglimento termico/chimico della scocca per raggiungere i pin della memoria provocherà la distruzione fisica dei micro-conduttori, rendendo il sensore strutturalmente incapace di testimoniare il passato ex-post.

---

## 3. Sottosistema di Alimentazione e Ricarica Induttiva Anti-Tracciamento

L'assenza di pin di ricarica fisici preserva l'ermeticità della scocca, ma introduce il rischio di tracciamento magnetico tramite attacchi di canale laterale (*Side-Channel Attacks*), analizzando le variazioni del campo magnetico emesso durante la carica.

* **Cella Litio-Titanato (LTO):** Viene preferita alle comuni celle LiPo consumer. Tollerando oltre 15.000 cicli di carica/scarica (durata stimata dell'oggetto > 20 anni), non rischia l'esplosione o il rigonfiamento sotto stress termico e supporta correnti di micro-carica estremamente stabili in ambienti estremi (-20°C / +60°C).
* **Schermatura Selettiva in Mu-Metal:** La bobina di ricezione interna è accoppiata a uno strato microscopico di ferrite e Mu-Metal (lega nichel-ferro ad altissima permeabilità magnetica). Questa schermatura devia i flussi magnetici parassiti diretti verso i circuiti logici e i sensori biometrici, impedendo che l'induzione lasci una firma elettromagnetica geometricamente tracciabile nello spazio circostante durante la ricarica.
* **Protocollo Anti-Tracciamento:** La base di ricarica associata non opera a frequenza fissa. Il firmware implementa una micro-variazione stocastica continua della frequenza di commutazione induttiva. Per un rilevatore esterno, lo spettro magnetico emesso si confonde con il normale rumore di fondo di un qualsiasi elettrodomestico.

---

## 4. Il Canale Informativo Aptico-Sensoriale (Diritto all'Informazione Protetta)

Per risolvere il conflitto tra la necessità di isolamento radio e il diritto dell'utente di conoscere il proprio stato di sicurezza, il bracciale sostituisce l'interfaccia visiva con un linguaggio aptico basato su un micro-attuatore a risonanza lineare (LRA) a bassissimo consumo energetico, posizionato a diretto contatto con la parte interna del polso.

Questo linguaggio sensoriale traduce l'Indice di Danno $D$ in impulsi cinematici privati, percepibili esclusivamente per via ossea o cutanea dall'utente, garantendo la consapevolezza dello stato della rete senza esporre schermi luminosi.

### Mappatura degli Stati Aptici e Comportamento Radio

#### A. Stato Nominale (Stato Libero: $D < 0.4$)
* **Comportamento Radio:** Canale BLE (Bluetooth Low Energy) attivo su richiesta hardware (doppio tap rilevato dall'accelerometro). La radio Sub-GHz opera in trasmissione nominale a bassissimo duty-cycle. I dati sanitari privati (grafici HRV, sonno) vengono scaricati localmente sull'applicazione per smartphone senza transitare su alcun cloud.
* **Feedback Aptico:** Silenzio sensoriale automatico. Un singolo micro-impulso morbido (durata 0.1 secondi) viene generato solo al tocco della superficie capacitiva per confermare l'avvenuta sincronizzazione o l'accensione del dispositivo.

#### B. Stato di Allerta (Sofferenza Moderata: $0.4 \le D \le 0.7$)
* **Comportamento Radio:** Il BLE riduce drasticamente la finestra temporale di visibilità radio. Il transceiver Sub-GHz incrementa la frequenza di invio dei pacchetti basandosi su una distribuzione stocastica.
* **Feedback Aptico:** Una doppia pulsazione leggera a bassa frequenza (simile a un battito cardiaco rallentato) generata automaticamente ogni 10 minuti. Comunica all'utente in modo discreto: *La rete rileva un incremento dello stress bio-ambientale d'area. Il sistema sta modulando le difese.*

#### C. Stato di Emergenza e Isolamento (Frizione Asimmetrica: $D > 0.7$)
* **Comportamento Radio:** **Il modulo BLE viene disattivato permanentemente a livello hardware (Kill-Switch logico dei registri radio).** Lo smartphone viene completamente isolato per impedire attacchi di triangolazione RSSI e la cattura forense dei dati biometrici storici in caso di sequestro del telefono. La radio Sub-GHz commuta alla massima potenza di trasmissione per iniettare il payload cifrato da 96 byte nella mesh di superficie.
* **Feedback Aptico:** Una micro-vibrazione nitida ad alta frequenza (un "brivido" meccanico avvertibile solo dall'utente, della durata di 0.3 secondi) ripetuta tassativamente ogni 2 minuti.

> **Valore Operativo del Canale Aptico:** Questo meccanismo tutela il diritto all'informazione dell'utente senza esporlo a rischi balistici. L'utente riceve la certezza sensoriale che:
> 1. Lo smartphone è stato isolato con successo (scudo forense attivo).
> 2. Il bracciale è operativo, integro e non ha subito spegnimenti o guasti hardware.
> 3. Il pacchetto di emergenza viene correttamente propagato nello spazio aereo verso i Router Mesh di Superficie.

---

## 5. Architettura Logica del Firmware (Dual-Channel Isolation)

La macchina a stati finiti implementata nel firmware del cripto-braccialetto gestisce l'isolamento dei canali radio e i cicli di feedback in base al flusso logico seguente:


```
+-----------------------------------+
|        ACCENSIONE / BOOT          |
+-----------------+-----------------+
|
v
+-----------------------------------+
|      Campionamento Biometrico     |
|         (PPG / GSR via DMA)       |
+-----------------+-----------------+
|
v
+-----------------------------------+
|    Calcolo Indice di Danno (D)    |
+-----------------+-----------------+
|
+-----------------------+-----------------------+
|                                               |
| (D <= 0.7)                                    | (D > 0.7)
v                                               v
+-----------------------------+               +-----------------------------+
|    STATO NOMINALE / ALLERTA |               |    STATO DI EMERGENZA       |
|                             |               |                             |
| * BLE: Attivo / Condizionato|               | * BLE: KILL-SWITCH HARDWARE |
| * Sub-GHz: Config. Nominale |               | * Sub-GHz: Max Power TX     |
| * Aptico: Silenzio / 10 min |               | * Aptico: Impulso ogni 2 min|
+-----------------------------+               +-----------------------------+
```

La transizione allo Stato di Emergenza è unidirezionale per tutta la durata del picco di stress biologico rilevato: il ripristino del canale BLE nominale avviene solo dopo la stabilizzazione dell'Indice $D$ al di sotto della soglia di allerta per un tempo di consolidamento minimo pari a 30 minuti, per prevenire attacchi basati su transizioni rapide indotte (*Oscillation Attacks*)
