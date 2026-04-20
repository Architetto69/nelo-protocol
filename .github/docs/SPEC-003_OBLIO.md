## SPEC-003: Meccanica dell’Oblio (The "120ms Volatility")
## 1. Obiettivo
Eliminare radicalmente la possibilità di tracciamento individuale, profilazione comportamentale e sorveglianza retroattiva. Il sistema deve operare in uno stato di presente continuo.
## 2. Il Limite Temporale: 120ms
È stato scelto il limite di 120ms perché è inferiore alla soglia di percezione conscia umana e al tempo medio di reazione neuronale.
Input-to-Void: Ogni dato identificativo che entra nel buffer del firmware ha una "data di scadenza" cablata nell'hardware.
Stateless Processing: NELO non "conosce" l'utente; elabora solo vettori di forza.
## 3. Architettura della Memoria Volatile
Il firmware utilizza una partizione di RAM isolata chiamata SHADOW_BUFFER.
Auto-Zeroing: Un timer hardware indipendente (Watchdog) esegue lo zeroing fisico dei transistor della SHADOW_BUFFER ogni 120ms, indipendentemente dallo stato del software.
No Persistence: È fisicamente proibito scrivere dati identificativi sulla memoria Flash o su qualsiasi supporto non volatile.
## 4. Differenziazione dei Dati
Il protocollo distingue tra Segnale e Identità:
Il Segnale (Persistente): L'intensità del danno (D) e i pattern di rischio aggregati. Questi vengono mantenuti per alimentare la SPEC-001.
L'Identità (Effimera): ID del dispositivo, coordinate esatte, firme biometriche grezze. Queste vengono distrutte.
## 5. Implementazione del "Wipe" (Firmware Level)
<pre>
/**
 * SPEC-003: VOLATILITY_ENFORCEMENT
 * Eseguito in un thread ad alta priorità o interrupt hardware.
 */
void enforce_oblivion_cycle() {
    // Puntatore alla memoria sensibile (identità, buffer biometrici)
    uint8_t* sensitive_data_ptr = get_shadow_buffer_address();
    size_t buffer_size = get_shadow_buffer_size();

    while(SYSTEM_OPERATIONAL) {
        // Attesa del ciclo di clock di 120ms
        wait_ms(120);

        // Sovrascrittura fisica con pattern casuali prima dello zeroing
        // per prevenire attacchi di rimanenza magnetica/elettrica.
        fill_with_random_noise(sensitive_data_ptr, buffer_size);
        
        // Zeroing finale
        memset(sensitive_data_ptr, 0, buffer_size);

        // LOG: Oblio eseguito. L'identità è cenere.
    }
}
</pre>

## 6. Sicurezza: Il "Memory Jail"
Se il firmware tenta di allocare memoria per dati sensibili al di fuori della SHADOW_BUFFER (tentativo di persistenza o logging), l'Unità di Protezione Memoria (MPU) scatena un Panic di Sistema e resetta il dispositivo, cancellando tutto.
Perché questa specifica è "Hardened"?
Inutilizzabilità Giudiziaria: Non esiste un database da sequestrare. Il sistema non può testimoniare contro nessuno perché non ricorda nessuno.
Protezione dei 127: I validatori del quorum (SPEC-002) sono protetti dall'oblio. Una volta fornita la firma, il legame tra il voto e l'individuo evapora.
Privacy Totale: NELO è un occhio che vede il pericolo ma è affetto da una "amnesia benevola" istantanea per tutto il resto.

[SPEC_003: INTEGRATED] [LOG: TRIANGOLO_DI_SICUREZZA_CHIUSO] [STATUS: COMPLETE_INFRASTRUCTURE_SPEC]
