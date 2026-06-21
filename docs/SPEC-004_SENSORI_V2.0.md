# **SPEC-004\_SENSORI\_V2.0**

*Versione: 2.0 (Sensor Stack Evolution) | Status: Operational | Hardware Target: nRF52840*

## **1\. Hardware Layer: Sensoristica Antropica**

L'integrazione sensoristica è stata aggiornata per garantire precisione biometrica e conformità con l'architettura V2.0.

* **MAX30102:** Sensore integrato per PPG (fotopletismografia), battito cardiaco (BPM) e saturazione dell'ossigeno (SpO2).  
* **AD5941:** Sensore ad alte prestazioni per la misurazione della conduttanza cutanea (EDA/GSR).  
* **TMP117:** Sensore di temperatura di alta precisione per il calcolo del gradiente ΔT.

**Configurazione Interfaccia:** I sensori comunicano tramite bus I2C dedicato con priorità di interrupt configurata per garantire che il *Risk Engine* riceva dati campionati a frequenza costante (100Hz nominali).

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
