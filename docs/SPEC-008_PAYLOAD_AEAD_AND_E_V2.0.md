# **SPEC-008\_PAYLOAD\_AEAD\_AND\_E\_V2.0**

*Versione: 2.0 (Evolution) | Status: Operational | Mode: Authenticated Encryption*

## **1\. Panoramica del Payload (Concatenato)**

Nella versione 2.0, il pacchetto trasmesso alla rete Mesh non è più un'unità singola, ma un blocco concatenato composto da due segmenti distinti. Questo garantisce una catena di autenticazione che parte dal sensore e viene validata dal contesto mobile.

## **2\. Struttura del Pacchetto (Payload V2)**

Il payload complessivo è composto dalla concatenazione: **\[Segmento S\] \+ \[Segmento A\]**.

### **2.1 Segmento S (Source \- Sensore)**

Prodotto dal firmware (nRF52840) e firmato tramite *CryptoCell-310* (Ed25519).

* **Dimensione:** 83 byte (fisso).  
* **Struttura:** \[timestamp (4b)\]\[damage\_index (2b)\]\[temperature (2b)\]\[bpm (1b)\]\[spo2 (1b)\]\[battery (1b)\]\[eda\_data (8b)\]\[signature\_hw (64b)\].

### **2.2 Segmento A (Contextual \- Gateway)**

Aggiunto dallo Smartphone Gateway (App) prima dell'inoltro alla rete Mesh.

* **gw\_node\_id (16 byte):** UUID univoco del dispositivo mobile.  
* **context\_flags (4 byte):** Bitmask (Stato rete: WiFi, Roaming, Cell, Precisione GPS).  
* **app\_signature (64 byte):** Firma Ed25519 generata dall'App sul blocco \[Segmento S \+ Segmento A\].

## **3\. Integrità e Validazione**

Il sistema utilizza l'approccio AEAD (Authenticated Encryption with Associated Data):

* Il *Consensus Layer* esegue la verifica della **Firma HW** (integrità dato bio) e della **Firma App** (autenticità del contesto).  
* Qualsiasi tentativo di manipolazione del valore *D* (damage\_index) o dei flag di contesto invaliderà una delle due firme, causando lo scarto immediato del pacchetto da parte del quorum bizantino.  
* La decodifica e la validazione avvengono esclusivamente a livello di rete (Consensus Layer), garantendo che i nodi di transito (Mesh) restino *stateless* e ignorino il contenuto informativo del pacchetto.