# **ARCHITETTURA DEL PROTOCOLLO NELO \- SPEC V2.0**

*Versione: 2.0 (Evolution) | Status: Operational Friction | Mode: Systemic Inertia*

## **1\. Evoluzione Architetturale**

Il protocollo NELO evolve dalla versione 1.0 (Mesh pura) alla **versione 2.0 (Hybrid Gateway-Stateless)**. Il sensore (L1) non agisce più come nodo terminale autonomo di iniezione, ma come sorgente di dati firmati che necessitano del contesto mobile (Gateway) per completare la catena di autenticazione.

## **2\. Payload di Comunicazione (Payload AEAD V2)**

Il pacchetto è ora suddiviso in due segmenti interdipendenti, garantendo una firma a due livelli:

### **2.1 Segmento S (Sensore \- Source)**

Prodotto dal firmware (nRF52840) e firmato tramite *CryptoCell-310* (Ed25519).

* **Dimensione:** 83 byte (fisso).  
* **Struttura:** \[timestamp (4b)\]\[damage\_index (2b)\]\[temperature (2b)\]\[bpm (1b)\]\[spo2 (1b)\]\[battery (1b)\]\[eda\_data (8b)\]\[signature\_hw (64b)\].

### **2.2 Segmento A (App \- Contextual)**

Aggiunto dal Gateway Mobile (App) prima dell'inoltro alla rete Mesh.

* **gw\_node\_id:** UUID univoco del dispositivo mobile.  
* **context\_flags:** Bitmask (WiFi, Roaming, Cell, Location\_Precision).  
* **app\_signature:** Firma Ed25519 del blocco concatenato \[Segmento S \+ Segmento A\].

## **3\. Schema di Flusso Aggiornato (V2.0)**

`graph TD`  
    `S1[Sensore nRF52840] -->|Segmento S: Dati Firmati| G[Smartphone Gateway (App)]`  
    `G -->|Segmento S + A: Con testo Firmato| M[Rete Mesh COTS]`  
    `M -->|Validazione| CL[Consensus Layer]`  
    `CL -->|Latenza L(D)| M`

## **4\. Risk Engine V2 (Integrazione Biometrica)**

Il calcolo di *D* (Indice di Danno) è stato aggiornato per includere la variabilità dell'ossigenazione sanguigna (SpO2) misurata dal MAX30102:  
*Dv2 \= σ(α·G \+ β·(1 \- HRVnorm) \+ γ·ΔT \+ δ·(1 \- SpO2norm))*  
Il fattore δ (SpO2) normalizza l'indice di sofferenza, rendendo il quorum più sensibile a cali di saturazione ossigeno improvvisi.
