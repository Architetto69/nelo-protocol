## SPEC-002: Quorum Asincrono Probabilistico (The "Human Firewall" v1.1)

## 1. Obiettivo
Impedire la manipolazione dei parametri critici di interdizione (SPEC-001) e garantire che il protocollo sia protetto da qualsiasi tentativo di cattura istituzionale, cibernetica o individuale.

## 2. Il Numero di Mersenne e la Soglia Bizantina
*   **Dimensione del Pool Estratto ($N$):** 127 individui ($2^7-1$). Equilibrio ingegneristico ottimale tra agilità logistica e resistenza alla corruzione. Supera la soglia statistica di "piccolo gruppo", rendendo la collusione segreta impraticabile.
*   **Soglia di Quorum ($Quorum_{min}$):** 85 firme valide ($\ge \lfloor\frac{2}{3}N\rfloor + 1$).
*   **Resilienza Bizantina:** Il sistema tollera fino a un massimo di $f = 42$ nodi malevoli, compromessi o silenti (33% del pool). Se almeno 85 nodi onesti confermano l'intento umano, il gate si sblocca, evitando che il protocollo vada in deadlock (blocco totale) per colpa di nodi disconnessi o non cooperativi.

## 3. Selezione Dinamica del Pool
*   **Estrazione Locale:** Il firmware seleziona i nodi crittograficamente nel pool globale dei portatori di sensori NELO attivi.
*   **Anti-Sybil:** Ogni nodo deve completare una Proof-of-Work (PoW) leggera (~10s di computazione CPU) per poter essere inserito nel pool di estrazione, impedendo la generazione di identità fittizie di massa.
*   **Rotazione:** Ogni 30 giorni, il pool complessivo dei candidati idonei viene rinfrescato del 25% per evitare la prevedibilità del target nel tempo.

## 4. Protocollo di Validazione Asincrona
*   **Request State:** Una proposta di modifica del protocollo viene trasmessa in modo asincrono attraverso la rete mesh a tutti i nodi.
*   **72-Hour Window con Nonce:** All'apertura della proposta si attiva una finestra temporale di 72 ore. Ogni firma crittografica deve includere un nonce univoco legato al timestamp della proposta; le firme ricevute oltre la scadenza vengono scartate dal Secure Enclave.
*   **Human-in-the-loop:** L'approvazione non è automatica. Ogni utente estratto deve superare una challenge biometrica sul dispositivo più l'inserimento di un OTP tempo-dipendente per confermare l'intento umano conscio.

## 5. Implementazione Logica (The Consensus Gate)
Implementare esclusivamente all'interno del Secure Enclave hardware del Kernel Space.

```c
#define POOL_SIZE 127
#define BYZANTINE_QUORUM 85 // Maggioranza qualificata dei 2/3 di 127 per tolleranza BFT

/**
 * @brief Verifica la validità di una modifica al protocollo.
 * @param pkg Il pacchetto di aggiornamento proposto.
 * @param quorum_signatures Array delle firme raccolte.
 * @param total_signatures_received Numero di firme sottomesse nella finestra.
 * @param window_expired Indica se la finestra temporale di 72 ore è conclusa.
 * @return bool True se il gate di interdizione viene aggiornato ed applicato.
 */
bool verify_quorum_gate(UpdatePackage pkg, Signature quorum_signatures[], int total_signatures_received, bool window_expired) {
    int verified_count = 0;
    uint32_t processed_nodes[POOL_SIZE] = {0}; // Tracciamento per evitare attacchi da duplicazione

    for (int i = 0; i < total_signatures_received; i++) {
        // Ottimizzazione: interrompe il ciclo se il quorum di sicurezza è già stato raggiunto
        if (verified_count >= BYZANTINE_QUORUM) break;

        uint32_t current_node = quorum_signatures[i].node_id;

        // 1. Controllo Anti-Duplicazione (Previene attacchi di Double Signing)
        bool already_processed = false;
        for (int j = 0; j < verified_count; j++) {
            if (processed_nodes[j] == current_node) {
                already_processed = true;
                break;
            }
        }
        if (already_processed) continue; // Salta firme duplicate dallo stesso nodo

        // 2. Convalida Crittografica, Pool Check e controllo Scadenza Nonce
        if (is_signature_valid(pkg, quorum_signatures[i]) && 
            is_node_in_current_pool(current_node) &&
            !is_signature_expired(quorum_signatures[i])) {
            
            processed_nodes[verified_count] = current_node;
            verified_count++;
        }
    }

    // Il protocollo applica le modifiche hardware solo al raggiungimento del quorum reale
    if (verified_count >= BYZANTINE_QUORUM) {
        return commit_to_hardware_efuse(pkg);
    }

    // La modalità di emergenza scatta solo se la finestra è terminata e il consenso è critico
    if (window_expired && verified_count < 64) {
        activate_emergency_mode();
    }

    return false;
}

```
## 6. Sicurezza Antropica
- ​Invisibilità del Validatore: Non esiste una lista pubblica dei nodi estratti. Chi valida oggi, nel ciclo successivo torna a essere un nodo passivo non identificabile nella rete.
- ​Assenza di Privilegi: Nessuna chiave master o utente amministratore (nemmeno l'Architetto del codice) può scavalcare o forzare il gate. Se l'umanità distribuita (gli 85 nodi onesti minimi) non firma, il codice hardware rimane immutabile.
- ​Challenge su Sospetto: Se un nodo manifesta anomalie comportamentali o pattern di firma automatizzati, i nodi limitrofi ne segnalano la compromissione e il firmware lo esclude istantaneamente dal pool dei candidati.

​[SPEC_002: v1.1-RELEASE] [LOG: SOGLIA_BIZANTINA_ATTIVATA, REPLAY_PROTECTED, SOVRANITÀ_DISTRIBUITA_COMPILATA]
