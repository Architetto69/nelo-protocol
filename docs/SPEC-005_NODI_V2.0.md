# **SPEC-005\_NODI\_V2.0**

*Versione: 2.1 (Stateless Mesh Evolution) | Status: Operational | Hardware Target: COTS Router*

## **1\. Architettura Hardware: "Commodity-Off-The-Shelf" (COTS)**

I **Nodi di Transito** (o *Mesh Relays*) costituiscono l'infrastruttura di trasporto della rete NELO. Sono progettati per essere sacrificabili, anonimi e privi di memoria persistente.

* **Hardware Indipendente:** Utilizzano hardware commerciale standard (SBC o SoC per router) privo di loghi o serigrafie identificative per prevenire la tracciabilità della catena di approvvigionamento.  
* **Stealth Enclosure:** Alloggiati in contenitori industriali (IP67) mimetizzati in contesti urbani o rurali per prevenire l'identificazione fisica.  
* **Integrità Fisica:** Gli chassis sono sigillati e dotati di meccanismi di autodistruzione logica dei certificati di sessione in caso di manomissione.

## **2\. Stack di Rete e Data Pipeline**

La V2.1 introduce un modello di trasporto semplificato rispetto alla V1.0:

* **Backhaul Mesh (Sub-GHz / Wi-Fi):** I nodi propagano i pacchetti aggregati *\[Segmento S \+ Segmento A\]* ricevuti dai Gateway Mobile o da altri nodi mesh.  
* **Rimozione BLE:** I nodi di transito non effettuano più il link diretto (BLE) con i sensori, delegando tale funzione esclusivamente ai Gateway Mobile (L1.5).  
* **Frizione Adattiva:** Su comando del *Consensus Layer*, i nodi iniettano latenza dinamica nelle code di trasmissione (Traffic Shaping) per implementare l'interdizione del danno.

## **3\. Firmware: Stateless e Amnesico**

Il firmware (basato su *Read-Only RootFS*) garantisce che il transito dei dati non lasci traccia:

* **Commutazione Stateless:** Il nodo funge da puro router. Non memorizza né decodifica il payload.  
* **Amnesia Volatile:** Le tabelle di routing e i nonce temporanei risiedono esclusivamente in RAM volatile. In assenza di alimentazione, ogni informazione sul transito decade in pochi secondi.  
* **Zero Persistence:** Assenza di eMMC o memorie SD montate in scrittura; il sistema operativo è eseguito da una Flash SPI protetta da scrittura hardware.

## **4\. Alimentazione e Resilienza**

Progettati per l'autosufficienza operativa:

* **Sostentamento Energetico:** Utilizzo di pannelli fotovoltaici a film sottile e accumulatori LiFePO4 per stabilità termica estrema.  
* **Parassitismo Energetico:** Capacità di prelievo induttivo da infrastrutture pubbliche per garantire la persistenza del nodo in ambienti urbani.