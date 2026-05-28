Il firmware dei router mesh (i nodi di transito) è il vero cuore logistico del protocollo. Se il braccialetto è il sensore biologico passivo, il router è il guardiano dell'infrastruttura.

Dato che i nodi saranno gestiti dai maker della community e installati in contesti ostili o instabili, il software non può permettersi il lusso di essere un normale sistema operativo di rete. Deve essere progettato seguendo i principi della **ridondanza estrema**, dell'**immutabilità** e dell'**assenza di stato** (*stateless*).

Ecco i tre pilastri ingegneristici su cui dobbiamo poggiare lo sviluppo del software per i nodi Alpha:

---

### 1. Architettura Stateless e Memoria Volatile (`SPEC-010`)

La regola d'oro del router NELO è: **ciò che non esiste non può essere sequestrato o forzato**.

* **Esecuzione solo in RAM:** Il sistema operativo (una build minimalista basata su un kernel Linux hardened o un RTOS come Zephyr, a seconda dell'hardware) risiede compresso in una partizione della memoria Flash protetta da scrittura hardware (ROM). All'avvio, l'intero sistema viene caricato in RAM e la Flash viene isolata.
* **Zero Logging:** Il software non scrive un singolo byte di log su supporti persistenti. Non esistono file come `syslog`, non ci sono cronologie di connessione, né tabelle di routing salvate.
* **Il buffer circolare a sovrascrittura rapida:** I pacchetti biometrici cifrati da 96 byte che transitano nel nodo vengono allocati in un buffer circolare in RAM gestito da puntatori volatili. Non appena un pacchetto viene ritrasmesso ai nodi adiacenti, lo spazio di memoria viene immediatamente sovrascritto con stringhe di zeri o rumore stocastico.

---

### 2. Stack Radio Dual-Layer Dinamico

Il software deve gestire contemporaneamente due interfacce radio operanti su logiche opposte:

```
                  +-----------------------------------+
                  |      CORE ROUTER SOFTWARE         |
                  |  (Kernel in RAM / Zero-Storage)   |
                  +-----------------------------------+
                                    |
            +-----------------------+-----------------------+
            |                                               |
            v                                               v
+-----------------------+                       +-----------------------+
|   INTERFACCIA LOCAL   |                       |  INTERFACCIA BACKHAUL |
| (Sub-GHz / LoRa 868)  |                       |  (HF / SDR / Mesh)    |
+-----------------------+                       +-----------------------+
| Rilevamento passivo   |                       | Instradamento a lungo |
| braccialetti (Edge)   |                       | raggio verso i nodi   |
| e mesh di prossimità  |                       | di uscita o dorsale   |
+-----------------------+                       +-----------------------+

```

* **Livello Locale (Ascolto e Mesh di Prossimità):** Utilizza il chip Sub-GHz (es. SX1262) per mantenere la maglia a corto/medio raggio con gli altri braccialetti e router vicini. Questo livello deve essere ultra-ottimizzato per la gestione delle collisioni radio in ambienti ad alta densità (algoritmo CSMA modificato), impedendo che la rete si saturi se migliaia di braccialetti si attivano contemporaneamente.
* **Livello Backhaul (Esfiltrazione):** Il software deve includere driver flessibili per interfacciarsi con moduli radio a onde corte (HF) o schede SDR. Se il nodo rileva l'isolamento locale, deve essere in grado di riconfigurare al volo i parametri di trasmissione (cambio frequenza, bitrate, potenza) per cercare un varco radio oltre le linee di sbarramento.

---

### 3. Tolleranza ai Guasti ed Autoguarigione (Watchdog Hardware)

I router saranno installati in luoghi di difficile accesso (tetti, pali della luce, alberi) e dovranno operare per mesi senza manutenzione umana.

* **Anti-Brick Firmware:** Il software utilizzerà un sistema a doppia partizione (Slot A e Slot B). Se un aggiornamento firmware via mesh fallisce o viene corrotto da interferenze, il Watchdog hardware della CPU rileva il mancato avvio e ripristina istantaneamente la versione precedente funzionante nello Slot specchio.
* **Dynamic Routing stocastico:** Il software non calcola la rotta ottimale statica (stile tabelle IP classiche), poiché i nodi vicini potrebbero accendersi o spegnersi continuamente a causa di guasti o attacchi. Il software applica un algoritmo di instradamento probabilistico: invia il pacchetto nella direzione geografica stimata migliore, sfruttando il rimbalzo dinamico. Se un nodo intermedio muore, la rete guarisce da sola deviando il flusso istantaneamente.

---

Questo livello di accuratezza nel software è ciò che trasforma dei semplici componenti elettronici da pochi euro in un'infrastruttura di grado militare.

