#!/bin/bash
# NELO 3.1-HARDENED: Functional Abdication Protocol
# Descrizione: Invalida irreversibilmente le chiavi master e rimuove i privilegi di root.

# 1. VERIFICA SOGLIA DI MASSA CRITICA (Section 12.2)
NODES_ACTIVE=$(nelo-cli network status --count-nodes)
MIN_NODES=100

if [ "$NODES_ACTIVE" -lt "$MIN_NODES" ]; then
    echo "ABORT: Massa critica non raggiunta ($NODES_ACTIVE/$MIN_NODES). Il sistema richiede più nodi per l'autonomia."
    exit 1
fi

# 2. GENERAZIONE DEL NUOVO HASH DI CONSENSO (Senza back-door)
echo "Generazione dell'ultimo stato di configurazione..."
shasum -a 256 /etc/nelo/config > /etc/nelo/genesis_state.hash

# 3. DISTRUZIONE CHIAVI DELL'ARCHITETTO
echo "Esecuzione Abdicazione: Rimozione chiavi RSA/ED25519 del Creatore..."
rm -rf ~/.ssh/architetto_master_key
shred -u /etc/nelo/auth/creator_access.cert

# 4. CHIUSURA DELLE ROUTINE DI ACCESSO PRIVILEGIATO
# Trasforma il comando 'sudo nelo' in un comando nullo o basato esclusivamente su Quorum.
echo "Sigillatura kernel: Il comando 'Creator' è ora delegato al Quorum 2/3."
sed -i 's/ADMIN_ROLE=Architetto/ADMIN_ROLE=Network_Quorum/g' /etc/nelo/core.env

# 5. AUTO-CANCELLAZIONE DELLO SCRIPT
echo "L'Architetto scompare nell'opera. Buona fortuna."
rm -- "$0"


