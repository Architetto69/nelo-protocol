# SPEC-004: Architettura e Ingegnerizzazione dei Cripto-Sensori Antropici (Antropic Sensor Nodes)
## 1. Obiettivo e Vettore Semantico
Il calcolo dell'Indice di Danno D \in [0.0, 1.0] costituisce la base logica su cui il Protocollo NELO innesca le contromisure di frizione e alterazione topologica della rete. Come evidenziato nelle analisi di vulnerabilità, se la determinazione di D fosse delegata a un'analisi puramente software, centralizzata o basata su metriche arbitrarie, il sistema cadrebbe vittima di manipolazioni semantiche o attacchi di cattura burocratica.
La SPEC-004 definisce le specifiche hardware, logiche e crittografiche dei **Cripto-Sensori Antropici**. Il loro scopo è trasformare la sofferenza non consensuale causata da atti di coercizione fisica o sistemica da un concetto filosofico a un **fatto fisico, multimodale e non falsificabile**, misurato direttamente sul confine biologico dell'essere umano.
```
+-----------------------------------------------------------------------+
|                       STADIO 1: RILEVAMENTO BIOLOGICO                 |
|  [PPG / HRV]           [GSR / Impedenza]       [Micro-Termoresistenze] |
+------------------------------------+----------------------------------+
                                     |
                                     v (Segnali Analogici)
+-----------------------------------------------------------------------+
|                       STADIO 2: SILICIO HARDENED                      |
|  [ nRF52840 SAADC ] -> [ EasyDMA Buffer ] -> [ Calcolo Matrice D ]     |
|         ^                       |                                     |
|         |                       v (Wipe Hardware ogni 120ms)          |
|  [ TRNG Noise Injection ] <-----+                                     |
+------------------------------------+----------------------------------+
                                     |
                                     v (Indice D + Nonce)
+-----------------------------------------------------------------------+
|                       STADIO 3: ATTREZZAGGIO CRITTOGRAFICO            |
|  [ CryptoCell-310 ] -> Firma Ed25519 (Chiave protetta da APPROTECT)   |
+------------------------------------+----------------------------------+
                                     |
                                     v (Payload Firmato e Cifrato)
                               [ Rete Mesh ]

```
## 2. Architettura Hardware Zero-Trust
Il cripto-sensore deve operare in un regime di totale isolamento logico. Non si ammettono interfacce di programmazione esposte, bus condivisi non protetti o linee dati intercettabili.
### 2.1 Principi di Progettazione del Circuito
 * **Integrazione Monolitica Analogico-Digitale:** Il condizionamento del segnale analogico (amplificazione e filtraggio delle risposte biologiche) e la conversione digitale devono avvenire sullo stesso PCB, racchiusi in una schermatura di Faraday metallica saldata direttamente sul circuito stampato. Questo impedisce l'iniezione di tensioni parassite tramite sonde esterne volte a simulare falsi stati di quiete.
 * **Alimentazione Isolata e Filtrata:** Il sottosistema di rilevamento biologico deve essere alimentato tramite regolatori di tensione a bassissimo rumore (LDO) dedicati, galvanicamente isolati dal circuito di trasmissione radio. Variazioni indotte nel consumo di corrente della radio non devono influenzare la precisione delle letture analogiche (prevenzione degli attacchi *Side-Channel* basati sulla potenza).
### 2.2 Requisiti Biometrici Multimodali (Prevenzione del Cloning)
Il sensore calcola il segnale di stress e coercizione combinando tre canali biologici indipendenti. Un'alterazione artificiale di un singolo parametro invalida l'intera lettura, azzerando l'anomalia.
 1. **Canale Elettrodermico (GSR - Galvanic Skin Response):** Misura la conduttanza cutanea indotta dall'attivazione del sistema nervoso simpatico, realizzato utilizzando un Analog Devices (Maxim) MAX30009 su bus I2C. Il circuito deve effettuare un rilevamento dell'impedenza dinamica a corrente alternata (AC) per accertarsi che il sensore sia a contatto con tessuto biologico vivo, scartando materiali sintetici o simulatori resistivi statici.
 2. **Canale Fotopletismografico (PPG / HRV):** Monitoraggio ottico del volume del sangue periferico tramite LED a doppia lunghezza d'onda (Infrarosso e Rosso), realizzato utilizzando un Analog Devices (Maxim) MAX30102 su bus I2C. Il firmware non analizza la semplice frequenza cardiaca bpt (*Beats Per Minute*), ma la **Variabilità della Frequenza Cardiaca (HRV)** nello spettro delle frequenze. Crolli repentini della componente HF (*High Frequency*) accoppiati a picchi della LF (*Low Frequency*) indicano uno stato di shock o minaccia imminente.
 3. **Gradiente Micro-Termico:** Monitoraggio accoppiato della temperatura cutanea superficiale e della temperatura ambientale, realizzato utilizzando due Texas Instruments TMP117 su bus I2C. Lo stress acuto indotto da coercizione genera vasocostrizione periferica con un calo termico localizzato immediato, impossibile da emulare senza alterare la temperatura interna del modulo.
## 3. Il Motore di Sintesi del Segnale D
Il calcolo dell'Indice D deve essere deterministico, matematicamente limitato e privo di euristiche aggiornabili o modelli statistici predittivi soggetti ad avvelenamento dei dati (*data poisoning*).
### 3.1 Funzione di Trasferimento Non Lineare
I tre vettori biologici grezzi – Conduttanza Cutanea (G), Variabilità Cardiaca normalizzata (V_{hrv}), e Gradiente Termico (\Delta T) – vengono mappati in matrici di rischio locali normalizzate nell'intervallo [0, 1].
L'Indice di Danno globale D è calcolato applicando una funzione di attivazione sigmoidale non lineare pesata, progettata per saturare rapidamente verso l'alto in presenza di anomalie simultanee su più canali:  

$$D = \frac{1}{1 + e^{-\left(\alpha \cdot G + \beta \cdot (1 - V_{hrv}) + \gamma \cdot \Delta T - \delta\right)}}$$
  
Dove \alpha, \beta, \gamma rappresentano i pesi hardware tarati in fase di fabbricazione del nodo, e \delta è il fattore di offset difensivo che impedisce ai falsi positivi microscopici (es. attività fisica moderata) di superare la soglia critica di innesco (0.7).
### 3.2 Meccanica dell'Oblio Biometrico (SPEC-003 Integration)
I dati biometrici grezzi necessari per calcolare G, V_{hrv} e \Delta T sono considerati **informazioni ad altissimo rischio di de-anonimizzazione**. Il sensore applica l'oblio radicale secondo le seguenti regole:
 * **Buffer Effimeri:** Le letture analogiche digitalizzate dall'ADC vengono scritte in una matrice circolare allocata in RAM.
 * **Finestra Temporale di Distruzione:** Il tempo di permanenza di una lettura grezza in memoria non può superare i **120 millisecondi**.
 * **Wipe Hardware Attivo:** Allo scadere di ogni ciclo di clock del timer hardware, il buffer viene letto dal Motore di Sintesi per aggiornare l'Indice D. Un istante dopo, il registro viene sovrascritto iniettando rumore bianco generato dal TRNG del chip. Nessuna traccia del battito cardiaco o della conduttanza cutanea passata sopravvive all'interno del silicio.
## 4. Attestazione Hardware e Cripto-Firmware
Un sensore non può limitarsi a trasmettere il valore di D in chiaro sulla rete mesh; deve dimostrare in modo matematico la propria integrità strutturale e l'autenticità del proprio silicio ad ogni singolo pacchetto inviato.
### 4.1 Isolamento delle Chiavi tramite Coprocessore Hardware
 * La chiave privata del cripto-sensore (SK_{sensor}) viene generata all'interno dell'ambiente protetto del coprocessore crittografico in fase di inizializzazione.
 * Il core principale della CPU non ha diritti di lettura sui registri fisici che contengono SK_{sensor}. Può soltanto inviare il digest del carico utile (il valore D unito a un timestamp e a un nonce) al coprocessore e ricevere indietro la firma digitale.
 * La chiave pubblica corrispondente (PK_{sensor}) è firmata digitalmente dalla chiave di attivazione del Quorum in fase di immissione del dispositivo nella rete mesh, creando un'ancora di fiducia distribuita.
### 4.2 Struttura del Pacchetto Dati (Payload)
Ogni trasmissione del sensore verso il nodo di rete mesh locale deve essere atomica e strutturata secondo il seguente schema a campi fissi, cifrato e firmato in modalità autenticata (AEAD):
| Dimensione (Byte) | Campo | Descrizione |
|---|---|---|
| **4** | TIMESTAMP | Epoca Unix aggiornata tramite clock hardware interno sincronizzato a maglia |
| **8** | NONCE | Numero casuale monouso generato dal TRNG per impedire attacchi di replica |
| **2** | DAMAGE_INDEX | Il valore D espresso come intero a 16-bit a virgola fissa (moltiplicato per 65535) |
| **64** | SIGNATURE | Firma Ed25519 calcolata su [TIMESTAMP + NONCE + DAMAGE_INDEX] |
## 5. Meccanismi di Anti-Manomissione (Tamper-Resistance)
Nel modello di minaccia di NELO, il sequestro fisico del sensore da parte di un avversario statale è un evento certo. Il dispositivo deve essere progettato per autodistruggersi crittograficamente prima che l'attaccante possa estrarre le chiavi o manipolare il firmware.
### 5.1 Protezione dei Canali di Debug (APPROTECT Permanente)
Le interfacce di programmazione e tracciamento hardware (JTAG / SWD) devono essere disabilitate in modo distruttivo e permanente. La configurazione dei registri di blocco dell'accesso alla memoria flash deve essere scritta nei settori non volatili del chip modificando i bit di protezione hardware (eFuse o registri UICR bloccati). Qualsiasi tentativo di forzare il debugger deve provocare il reset totale della memoria flash di sistema.
### 5.2 Rilevamento dell'Apertura del Casing (Involucro Attivo)
L'involucro protettivo del cripto-sensore è rivestito internamente da una maglia di vernice conduttiva percorsa da una micro-corrente continua monitorata da un pin GPIO con interrupt attivo.
 * **Innesco della Tabula Rasa:** L'interruzione fisica della maglia conduttiva (tentativo di apertura o perforazione del guscio) o la variazione repentina della capacità elettrica generano un interrupt non mascherabile (NMI) a priorità assoluta.
 * **Procedura di Emergenza:** Il firmware interrompe qualsiasi operazione in corso, invoca il coprocessore crittografico per sovrascrivere immediatamente il settore contenente SK_{sensor} con zeri logici, ed esegue un ciclo continuo di scrittura e cancellazione su tutta la SRAM, lasciando il chip permanentemente inutilizzabile e privo di materiale crittografico (Zeroizzazione Hardware).
A questo punto la struttura formale della SPEC-004 è definita in ogni suo vincolo logico e di sicurezza.

## Appendice Tecnica SPEC-004: Blindatura Hardware su Nordic nRF52840
[nelo_crypto_sensor_hardened.c](../src/nelo_crypto_sensor_hardened.c)
- Implementazione dei vincoli di sicurezza hardware SPEC-004 su nRF52840.
- Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
- version 3.1-HARDENED

[nelo_damage_synthesis.c](../src/nelo_damage_synthesis.c)
- Motore di sintesi deterministico in virgola fissa per l'Indice di Danno D.
- Ottimizzato per nRF52840 (Cortex-M4). Implementazione SPEC-004 Sezione 3.
- version 3.1-HARDENED

[nelo_transfer_function.c](../src/nelo_transfer_function.c)
- Implementazione Firmware (Virgola Fissa / Hardened) della Funzione di Trasferimento SPEC-004 su nRF52840.
- Target: Nordic Semiconductor nRF52840 (Cortex-M4 + CryptoCell-310)
- version 3.1-HARDENED

## Mappatura Finale dell'Integrazione Hardware (I^2C)
<pre>
+-------------------------------------------------------------------------+  
| SCHERMATURA DI FARADAY (Gabbia Metallica Saldata su PCB)                |  
|                                                                         |  
|  [MAX30009] ────► (GSR / Impedenza AC) ────┐                            |  
|  [MAX30102] ────► (PPG Rosso / IR)     ────┼──► Bus I2C Privato         |  
|  [TMP117-S] ────► (Temp. Cute)         ────┤    (Linee Schermate)       |  
|  [TMP117-E] ────► (Temp. Ambiente)     ────┘            │               |  
|                                                         ▼               |  
|  +-------------------------------------------------------------------+  |  
|  | HARDWARE NRF52840                                                 |  |  
|  |  [ TWIM0 EasyDMA ] ──► [ Matrice Effimera RAM ]                   |  |  
|  |                                │ (Wipe Attivo 120ms da TIMER1)    |  |  
|  |                                ▼                                  |  |  
|  |              [ TRNG Noise Injection / Zeroizzazione ]             |  |  
|  +-------------------------------------------------------------------+  |  
+-------------------------------------------------------------------------+  
</pre>

## Note di Audit di Sicurezza per lo Sviluppatore (SPEC-004-AUDIT):
1. Il Registro UICR: La funzione nelo_hw_enforce_approtect() scrive nella memoria non volatile speciale del chip. Questa operazione è irreversibile via software. Una volta eseguita sul sensore, il chip rifiuterà qualsiasi connessione J-Link o debugger esterno. Per riprogrammarlo sarà necessario eseguire un comando hardware di ERASEALL di intero chip, il quale azzera istantaneamente la flash, cancellando la chiave privata SK_{sensor}.
2. Uso degli Interrupt: Il TIMER1_IRQHandler è configurato con priorità 0. Questo garantisce che la distruzione del buffer della RAM non possa essere interrotta o ritardata da processi radio (BLE o Thread Mesh). L'oblio ha sempre la precedenza sulla comunicazione.
3. Istruzioni di Sincronizzazione (__DSB(), __ISB()): Inserite al termine del wipe hardware. Costringono la pipeline della CPU ARM a svuotare le sue cache interne e i registri interni dei passaggi intermedi, eliminando qualsiasi "fantasma energetico" o residuo dei dati biometrici grezzi prima di tornare all'esecuzione del codice ordinario.

