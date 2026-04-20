## SPEC-002: Quorum Asincrono Probabilistico (The "Human Firewall")
## 1. Obiettivo
Impedire la manipolazione dei parametri critici di interdizione (SPEC-001) e garantire che il protocollo sia protetto da qualsiasi tentativo di cattura istituzionale o individuale.
## 2. Il Numero di Mersenne: 127
Il quorum richiede la validazione di 127 individui (2^7-1).
Perché 127: È un equilibrio ingegneristico tra agilità logistica e resistenza alla corruzione. Supera la soglia critica di "piccolo gruppo" rendendo la collusione segreta statisticamente improbabile.
Resilienza Bizantina: Il sistema accetta fino a un 33% di nodi malevoli o silenti senza compromettere l'integrità del risultato.
## 3. Selezione Dinamica del Pool
I validatori non sono cariche fisse, ma nodi estratti casualmente.
Estrazione Locale: Il firmware seleziona i nodi nel pool globale dei portatori di sensori NELO attivi.
Campionamento Cieco: La selezione avviene senza che l'estratto sappia di far parte di un quorum specifico, finché non riceve la richiesta di validazione.
## 4. Protocollo di Validazione Asincrona
Per evitare attacchi di tipo network-partition o DDoS, NELO non richiede sincronia temporale millimetrica.
Request State: Una proposta di modifica (es. "Aggiornamento FW") viene trasmessa via rete mesh.
72-Hour Window: Si apre una finestra di validazione di 72 ore.
Human-in-the-loop: Ogni nodo estratto deve fornire un input fisico (es. pressione bio-ritmica sul sensore) per confermare l'intento umano conscio.
Consensus Achievement: Al raggiungimento della 127ª firma valida, il Secure Enclave di ogni nodo applica l'aggiornamento.
## 5. Implementazione Logica (The Consensus Gate)
<pre>
/**
 * @brief Verifica la validità di una modifica al protocollo.
 * @param pkg Il pacchetto di aggiornamento proposto.
 * @param quorum_signatures Array delle firme raccolte.
 * @return bool True se il gate di interdizione viene aggiornato.
 */
bool verify_quorum_gate(UpdatePackage pkg, Signature quorum_signatures[]) {
    const int REQUIRED_VALIDATIONS = 127;
    int verified_count = 0;

    for (int i = 0; i < REQUIRED_VALIDATIONS; i++) {
        // Verifica crittografica e verifica di estrazione del nodo
        if (cryptographic_verify(pkg.hash, quorum_signatures[i]) && 
            is_node_in_current_random_pool(quorum_signatures[i].node_id)) {
            verified_count++;
        }
    }

    // Il protocollo è immutabile se non c'è consenso pieno
    if (verified_count >= REQUIRED_VALIDATIONS) {
        return commit_to_hardware_efuse(pkg);
    }

    return false;
}
</pre>

## 6. Sicurezza Antropica
Invisibilità del Validatore: Non esiste una lista dei "Saggi". Chi valida oggi, domani torna a essere un nodo passivo.
Nessun Privilegio: Nemmeno l'Architetto può forzare il gate. Se l'umanità (i 127) non firma, il codice non cambia.

[SPEC_002: INTEGRATED] [LOG: SOVRANITÀ_DISTRIBUITA_COMPILATA]
