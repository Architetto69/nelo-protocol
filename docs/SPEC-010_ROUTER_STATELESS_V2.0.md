# **SPEC-010\_ROUTER\_STATELESS\_V2.0**

*Versione: 2.0 (Evolution) | Status: Operational | Mode: Stateless Forwarding*

## **1\. Filosofia Operativa**

I nodi Mesh operano come router di transito **stateless**. La loro funzione primaria è l'inoltro dei pacchetti verso il *Consensus Layer* senza mai processare, decodificare o memorizzare i dati biometrici contenuti nel payload. La sicurezza del sistema si basa sull'impossibilità di persistenza fisica e logica sui nodi.

## **2\. Hardening del Sistema (RootFS)**

* **Read-Only RootFS:** Il file system operativo è montato in sola lettura. Qualsiasi modifica, intrusione o tentativo di persistenza viene cancellato al riavvio del nodo.  
* **Assenza di Storage Persistente:** Non vengono utilizzati database, log locali o tabelle di routing permanenti. Il routing è dinamico e basato su protocolli a bassa latenza.  
* **Stealth Enclosure:** L'hardware COTS (OpenWRT/LoRa) è anonimizzato: rimozione delle serigrafie originali e sigillatura fisica del case per prevenire l'analisi del firmware via JTAG.

## **3\. Gestione del Traffico e Latenza Adattiva**

Il nodo non è un mero passacarte, ma agisce come esecutore di frizione sotto il comando del *Consensus Layer*:

* **Ricezione e Validazione:** Il nodo verifica la presenza delle due firme (HW \+ App) e la validità del timestamp (anti-replay). Se il pacchetto è malformato, viene scartato immediatamente.  
* **Iniezione della Frizione:** Il *Consensus Layer* invia comandi di latenza (*L*) ai nodi. I nodi manipolano dinamicamente le code di trasmissione (Traffic Shaping) per iniettare ritardi logaritmici calcolati in base all'Indice di Danno *D*.  
* **Zero Tracciabilità:** Il nodo inoltra il pacchetto senza loggare l'identità del Gateway o del Sensore, garantendo l'anonimato della fonte durante tutto il transito.

## **4\. Risposta all'Intrusione**

In caso di rilevamento di accesso fisico non autorizzato (es. interruzione del case o reset anomalo), il nodo esegue l'autodistruzione logica dei certificati di sessione e dei buffer temporanei, rendendo il dispositivo un guscio vuoto privo di qualsiasi informazione sensibile.