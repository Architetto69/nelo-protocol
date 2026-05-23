#!/bin/sh
# ==============================================================================
# NELO 3.1-HARDENED: Functional Abdication Protocol
# Descrizione: Invalida irreversibilmente le chiavi master, distrugge l'accesso 
#              del Creatore e delega permanentemente il controllo al Quorum 85/127.
# ==============================================================================

# Forza un PATH sicuro per impedire il dirottamento dei binari (Binary Hijacking)
export PATH="/usr/sbin:/usr/bin:/sbin:/bin"

# Intercetta i segnali di interruzione (Ctrl+C, terminazione) per impedire blocchi parziali
trap 'echo "ERRORE CRITICO: Il protocollo non può essere interrotto. Stato di emergenza."; exit 2' 1 2 3 15

echo "[+] Avvio Protocollo di Abdicazione Funzionale NELO v3.1..."

# 1. VERIFICA SOGLIA DI MASSA CRITICA (Allineato alla stabilità della rete)
NODES_ACTIVE=$(nelo-cli network status --count-nodes 2>/dev/null)
MIN_NODES=100

if [ -z "$NODES_ACTIVE" ] || [ "$NODES_ACTIVE" -lt "$MIN_NODES" ]; then
    echo "[-] ABORT: Massa critica non raggiunta ($NODES_ACTIVE/$MIN_NODES)."
    echo "    Il protocollo richiede un'infrastruttura mesh stabile per l'autonomia."
    exit 1
fi

# 2. GENERAZIONE DEL NUOVO HASH DI CONFIGURAZIONE NELLO STATO GENESI
echo "[+] Congelamento dello stato di configurazione corrente..."
shasum -a 256 /etc/nelo/config > /etc/nelo/genesis_state.hash
chmod 400 /etc/nelo/genesis_state.hash # Rende il file immutabile in sola lettura

# 3. SIGILLATURA DEL KERNEL (Prima la protezione logica, poi la distruzione)
# Il ruolo dell'Architetto decade. Il controllo passa al Quorum Bizantino (SPEC-002).
echo "[+] Riscrittura delle variabili d'ambiente del Secure Enclave..."
if [ -f /etc/nelo/core.env ]; then
    # Sostituzione atomica del ruolo amministrativo
    sed -i 's/ADMIN_ROLE=Architetto/ADMIN_ROLE=Network_Quorum/g' /etc/nelo/core.env
    # Forza il firmware a ricaricare le regole immutabili
    nelo-cli core reload-env || { echo "[-] ERRORE: Sigillatura fallita. Abort."; exit 1; }
else
    echo "[-] ERRORE: File core.env mancante. Struttura corrotta."; exit 1
fi

# 4. DISTRUZIONE CRITTOGRAFICA HARDWARE DELLE CHIAVI (Cancellazione Militare)
# Si usa una sovrascrittura a 7 passaggi con rumore TRNG prima della rimozione del puntatore.
echo "[+] Esecuzione Tabula Rasa: Distruzione fisica delle credenziali dell'Architetto..."

CREATOR_KEY="$HOME/.ssh/architetto_master_key"
CREATOR_CERT="/etc/nelo/auth/creator_access.cert"

for FILE in "$CREATOR_KEY" "$CREATOR_CERT"; do
    if [ -f "$FILE" ]; then
        # 7 passaggi di sovrascrittura con dati casuali, azzeramento finale e sincronizzazione hardware
        shred -n 7 -z -u "$FILE"
        # Forza il file system a svuotare i buffer fisici sui transistor delle memorie
        sync
    fi
done

# 5. RIMOZIONE DELLE SYSCALL DI ROOT (Abdicazione dei privilegi locali)
echo "[+] Disattivazione dei vettori di accesso manuale super-utente..."
nelo-cli core disable-privileged-syscalls --force

# 6. AUTO-CANCELLAZIONE IRREVERSIBILE DELLO SCRIPT DI DEPLOYMENT
echo "[+] L'Architetto scompare nell'opera. La sovranità è ora distribuita."
# Sovrascrive lo script stesso in RAM prima di rimuoverlo dal disco
shred -n 3 -z -u -- "$0"
