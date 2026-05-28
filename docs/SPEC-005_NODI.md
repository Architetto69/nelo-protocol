I **Nodi di Transito** (chiamati anche Router di Maglia o *Mesh Relays*) costituiscono il tessuto connettivo e la spina dorsale infrastrutturale della rete NELO. A differenza dei Cripto-Sensori Antropici, che sono dispositivi ultraleggeri a bassissimo consumo focalizzati sul corpo umano, i nodi di transito sono stazioni di smistamento dati fisse o semi-mobili.
Nel modello di minaccia del protocollo, questi nodi sono considerati **hardware sacrificabile e costantemente esposto alla cattura o alla distruzione fisica**. Di conseguenza, la loro ingegnerizzazione segue logiche di anonimato costruttivo, ridondanza e indipendenza energetica.
## 1. Architettura Hardware: "Commodity-Off-The-Shelf" (COTS)
Per impedire la tracciabilità della catena di approvvigionamento e la de-anonimizzazione della rete, i nodi di transito non usano silicio proprietario o PCB custom riconoscibili. Sono realizzati modificando hardware commerciale standard o Single Board Computers (SBC) a basso costo di derivazione industriale.
L'equipaggiamento tipo si basa su moduli con processori ARM Cortex-A o System-on-Chip (SoC) tipici dei router domestici, scelti per la loro enorme diffusione e l'impossibilità di tracciarne l'acquisto di massa.
### Caratteristiche del Layout Hardware
 * **Assenza di Elementi Identificativi:** Tutte le serigrafie sul PCB, i loghi dei produttori e i codici di lotto vengono rimossi tramite abrasione chimica o meccanica prima del posizionamento sul campo.
 * **Involucro Camuffato (Stealth Enclosure):** I nodi non hanno l'aspetto di apparati di rete. Sono sigillati in scatole di derivazione stagne IP67 industriali, scatole elettriche standard o mimetizzati all'interno di oggetti urbani e rurali comuni (es. finti raccordi idraulici, intercapedini edilizie, cassette di derivazione grigie).
## 2. Lo Stack di Rete e lo Sdoppiamento Radio
Ogni nodo di transito monta due o più interfacce radio indipendenti, isolate fisicamente all'interno dello chassis per operare su domini distinti:
| Sottosistema Radio | Frequenza / Protocollo | Ruolo Operativo |
|---|---|---|
| **Ingresso (Biometric Link)** | 2.4 GHz / Bluetooth Low Energy (BLE) o IEEE 802.15.4 | Rileva ed estrae al volo i pacchetti broadcast AEAD emessi spontaneamente dai Cripto-Sensori a corto raggio. |
| **Dorsale (Backhaul Mesh)** | Sub-GHz (868 MHz / 915 MHz LoRa) o Wi-Fi ad alta potenza (2.4/5 GHz) | Propaga i dati a lungo raggio verso gli altri nodi di transito della rete mesh tramite algoritmi di routing a cammino minimo (es. OLSR, BATMAN-adv). |
## 3. Il Firmware: Stateless e Amnesico
I nodi eseguono una distribuzione linux minimale fortemente indurita (tipicamente derivata da **OpenWrt** o sistemi Yocto custom) privata di tutti i driver non essenziali, stack di debug e interfacce di configurazione (no SSH, no HTTP, no Telnet).
> **Principio dell'Invisibilità dei Dati:** I nodi di transito sono puri commutatori di pacchetti. Non conservano i dati.
> 
### Meccanica Operativa del Transito
 1. **Ricezione:** Il nodo capta il pacchetto pre-firmato dal sensore.
 2. **Validazione Epoca/Nonce:** Verifica la freschezza temporale e l'unicità del nonce nel suo buffer volatile locale di brevissimo termine per scartare istantaneamente attacchi di replica (*Replay Attacks*).
 3. **Instradamento:** Rimpacchetta il payload crittografato e lo spara sulla dorsale verso i nodi adiacenti.
 4. **Distruzione:** Il pacchetto viene rimosso immediatamente dalla RAM del router. Il nodo non ha hard disk, chip di memoria eMMC persistenti o schede SD montate in scrittura; l'intero file system è in sola lettura (*Read-Only RootFS*) allocato in una Flash SPI protetta da scrittura hardware.
## 4. Alimentazione e Autonomia Energetica
I nodi devono poter sopravvivere all'isolamento dalle reti elettriche civili. Sono ingegnerizzati per operare in regime di auto-sostentamento o parassitismo energetico:
 * **Sostentamento Solare Passivo:** Piccoli pannelli fotovoltaici a film sottile (spesso camuffati con pellicole texturizzate o vernici selettive) abbinati ad accumulatori al litio-ferro-fosfato (LiFePO_4), scelti per l'estrema stabilità termica e la tolleranza a climi ostili (da -20°C a +60°C).
 * **Parassitismo Energetico Induttivo:** Nei contesti urbani, alcuni nodi sfruttano micro-spire a induzione accoppiate parassitariamente ai cavi della linea elettrica pubblica o dei sistemi di illuminazione, prelevando la corrente minima necessaria per mantenere attivo il SoC.
Se un nodo viene scoperto e disconnesso dall'alimentazione, la RAM volatile decade in pochi secondi, cancellando le tabelle di routing attuali e l'elenco dei nonce temporanei. Il nodo diventa un pezzo di silicio muto e privo di informazioni strategiche sul resto della topologia NELO.
