# **SPEC-004\_SENSORI\_V2.0**

*Versione: 2.0 (Sensor Stack Evolution) | Status: Operational | Hardware Target: nRF52840*

## **1\. Hardware Layer: Sensoristica Antropica**

L'integrazione sensoristica è stata raffinata per riflettere le specifiche di comunicazione di ciascun modulo:

* **MAX30102 (I2C):** Sensore integrato per PPG (fotopletismografia), battito cardiaco (BPM) e saturazione dell'ossigeno (SpO2).  
* **TMP117 (I2C):** Sensore di temperatura di alta precisione per il calcolo del gradiente ΔT.  
* **AD5941 (SPI):** Sensore per la misurazione della conduttanza cutanea (EDA/GSR). L'interfaccia SPI è necessaria per la corretta gestione dei dati ad alta velocità richiesti per le analisi di bioimpedenza.

## **2\. Firmware Logic: Data Pipeline**

Il firmware è stato riprogettato per operare esclusivamente come sorgente di dati firmati, delegando le funzioni di rete al Gateway Mobile.

* **Shadow Buffer:** I dati grezzi vengono bufferizzati in RAM volatile.  
* **Pre-elaborazione:** Il firmware normalizza i valori di SpO2 e BPM prima di alimentare il *Risk Engine* (Q16.16).  
* **Ruolo Firmware:** Il firmware esegue la lettura dei sensori, l'elaborazione locale del danno (D) e la firma crittografica del *Segmento S*. Lo stack di rete Mesh è stato rimosso; la comunicazione avviene tramite BLE (GATT Service) verso lo Smartphone Gateway.

## **3\. Struttura Payload (Segmento S)**

Il pacchetto è fisso a **83 byte** per garantire la compatibilità con la firma Ed25519 del CryptoCell-310.  
`[timestamp (4b)]`  
`[damage_index (2b)]`  
`[temperature (2b)]`  
`[bpm (1b)]`  
`[spo2 (1b)]`  
`[battery (1b)]`  
`[eda_data (8b)]`  
`[signature_hw (64b)]`  
