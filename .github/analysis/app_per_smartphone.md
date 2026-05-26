## I Cripto-Sensori come Wearable di Consumo
L'utente vede i propri dati sanitari tramite una app per smartphone connessa al cripto-sensore via BLE (Bluetooth Low Energy).

L'integrazione di una trasmissione **BLE (Bluetooth Low Energy)** verso un'applicazione per smartphone è un passaggio logico eccellente per la commercializzazione del dispositivo (il nostro "Cavallo di Troia" per il crowdfunding), ma introduce un **conflitto architetturale critico** con il modello di minaccia di NELO v3.1-HARDENED.

Se analizziamo la proposta dal punto di vista dell'ingegneria del protocollo, dobbiamo scindere rigorosamente l'uso del BLE in due scenari: **Cosa l'utente vede (I dati sanitari)** e **Cosa la rete mesh fa (L'Indice di Danno $D$)**.

Ecco l'analisi d'impatto e la soluzione ingegneristica per implementarlo in sicurezza:

---

### 1. Il Pericolo del BLE: La De-anonimizzazione (SPEC-006)

Se il sensore antropico usasse il BLE per trasmettere l'Indice di Danno $D$ allo smartphone affinché sia il telefono a inviarlo alla rete mesh, avremmo un collasso istantaneo della sicurezza:

* **Tracciabilità dell'indirizzo MAC:** Il protocollo BLE richiede un indirizzo MAC (anche se randomizzato, è intercettabile). Un attaccante statale dotato di antenne di monitoraggio urbano (*IMSI-Catchers* o sniffer BLE) potrebbe associare i pacchetti radio di stress all'identità dello smartphone, scoprendo esattamente **chi** sta subendo la coercizione.
* **Dipendenza dal Sistema Operativo:** Gli smartphone (iOS/Android) non sono hardware sicuro. I sistemi operativi possono essere infettati da spyware (come Pegasus) o costretti a loggare i dati, violando la `SPEC-003` (Meccanica dell’Oblio).

---

### 2. La Soluzione: Architettura a Due Canali Separati (Dual-Stack Radio)

Per mantenere l'app commerciale bellissima e funzionale (garantendo il successo del crowdfunding) senza compromettere la sicurezza della rete di resistenza, il cripto-sensore deve implementare una separazione fisica dei flussi radio.

Fortunatamente, i moderni chip IoT (come il già citato **Nordic nRF52840**) supportano nativamente il protocollo multiprotocollo dinamico. L'anello opererà quindi su due canali paralleli e stagni:

```
                      +----------------------------------+
                      |     CRIPTO-SENSORE ANTROPICO     |
                      +----------------------------------+
                                       |
            ┌──────────────────────────┴──────────────────────────┐
            ▼ (Canale Commerciale)                                ▼ (Canale NELO Core)
+---------------------------------------+             +---------------------------------------+
|        BLUETOOTH LOW ENERGY           |             |           RADIO SUB-GHz / LORA        |
|  Transito locale cifrato end-to-end   |             |    Pacchetti AEAD anonimi a 96 byte   |
|     dei dati biometrici storici       |             |     Diretto verso i Router di Maglia  |
+---------------------------------------+             +---------------------------------------+
            │                                                     │
            ▼                                                     ▼
+---------------------------------------+             +---------------------------------------+
|         APPLICAZIONE SMARTPHONE       |             |          RETE MESH DI TRANSITO        |
| Database SQLCipher locale e offline.  |             |  Frizione logaritmica e calcolo       |
| Visualizzazione grafici HRV e sonno.  |             |  dell'Entropia di Shannon (OBLIO).    |
+---------------------------------------+             +---------------------------------------+

```

#### Canale A: BLE $\to$ Smartphone (La feature per l'Utente)

* **Cosa viaggia:** Solo i dati biometrici storici ed elaborati (andamento del sonno, grafici HRV settimanali, trend dello stress).
* **Sicurezza:** La connessione BLE avviene solo tramite accoppiamento protetto (*Authenticated Pairing con Numeric Comparison*) e cifratura AES-128. L'app riceve i dati, li scrive nel database locale cifrato dello smartphone e non tocca mai internet.
* **Isolamento:** Il modulo BLE dell'anello viene **spento automaticamente** non appena viene rilevata un'anomalia biometrica critica ($D > 0.5$), impedendo che il sensore emetta segnali intercettabili durante un evento di pericolo.

#### Canale B: Sub-GHz / Proprietary $\to$ Mesh Relays (Il Protocollo NELO)

* **Cosa viaggia:** Esclusivamente il payload binario standardizzato da 96 byte (`SPEC-008`), contenente l'Indice $D$ anonimo e la firma asimmetrica del sensore.
* **Sicurezza:** Questo canale non passa dal telefono. Viene trasmesso direttamente dall'antenna dell'anello verso i router mesh distribuiti nell'ambiente, sfruttando frequenze Sub-GHz per massima penetrazione muraria e totale anonimato a livello di pacchetto.

---

### Impatto sulla Campagna di Crowdfunding

Presentare questa architettura duale nella campagna di crowdfunding non è un limite, ma un enorme **punto di forza commerciale**:

> *"A differenza degli altri smartwatch che usano il Bluetooth continuamente per inviare i tuoi dati medici ai server della compagnia, il nostro anello usa un canale Bluetooth protetto e isolato che si attiva solo quando decidi tu di guardare i tuoi grafici sul telefono. I tuoi dati sanitari non lasciano mai la stanza. E in caso di emergenza o blackout, l'anello smette di comunicare con il telefono e si collega direttamente alla rete di emergenza cittadina, garantendo che nessuno possa tracciare la tua posizione o la tua identità."*

In questo modo, l'utente ottiene l'esperienza fluida e moderna di un'app sul telefono (grafici, notifiche di benessere, analisi HRV), ma l'architettura profonda del protocollo NELO rimane matematicamente protetta, invisibile e inattaccabile.