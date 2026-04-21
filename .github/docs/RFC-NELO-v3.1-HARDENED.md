## Network for Ethical Limitation of Outcomes (NELO)

### Versione: 3.1-HARDENED

---

# 1. Abstract

NELO è un’infrastruttura distribuita progettata per **ridurre la scalabilità della sofferenza non consensuale** attraverso meccanismi di interdizione sistemica.

NELO:

* NON governa
* NON ottimizza il comportamento umano
* NON prende decisioni normative complesse

NELO:

* limita eventi ad alto impatto
* aumenta il costo della coercizione sistemica
* riduce la propagazione del danno su larga scala

---

# 2. Terminologia

Le parole chiave **MUST, MUST NOT, SHOULD, SHOULD NOT, MAY** sono da interpretarsi come in RFC 2119.

---

### 2.1 Definizioni

* **Harm Event (HE):** evento che genera danno misurabile
* **Non-Consensual Harm (NCH):** danno senza consenso valido
* **Systemic Scale (SS):** capacità di propagazione su larga scala
* **Risk Index (RI):** metrica combinata di rischio
* **Interdiction:** blocco o limitazione di un evento

---

# 3. Obiettivi del sistema

NELO MUST:

1. Ridurre la probabilità di NCH su scala sistemica
2. Aumentare il costo computazionale/economico della coercizione
3. Mantenere decentralizzazione completa
4. Garantire auditabilità

NELO MUST NOT:

1. Centralizzare il potere decisionale
2. Ottimizzare comportamento umano generale
3. Mantenere identità persistenti non necessarie
4. Operare senza tracciabilità

Qui il [THREAT MODEL](https://github.com/Architetto69/nelo-protocol/blob/main/THREAT_MODEL.md),
qui il [BACKOFF LOGARITMICO](https://github.com/Architetto69/nelo-protocol/blob/main/.github/docs/SPEC-001_BACKOFF.md)

---

# 4. Architettura

## 4.1 Overview

```id="arch1"
[Edge Nodes] → [Risk Layer] → [Consensus Layer] → [Interdiction Layer]
                    ↑                 ↓
              [Audit Layer] ← [Ledger]
```

---

## 4.2 Componenti

### 4.2.1 Edge Nodes

* raccolgono dati locali
* eseguono inferenza iniziale
* MUST operare senza dipendenza da nodo centrale

---

### 4.2.2 Risk Evaluation Layer

Calcolo del rischio:

```id="risk1"
RI = P × S × NC × SS
```

Dove:

* P = probabilità evento
* S = severità
* NC = grado di non-consenso
* SS = scalabilità sistemica

---

### 4.2.3 Consensus Layer

* basato su protocollo distribuito (BFT o equivalente)
* richiede quorum minimo Q

```id="cons1"
IF approvals ≥ Q THEN escalate
```

Q SHOULD essere ≥ 2/3 dei nodi attivi

Qui il [Quorum Asincrono Probabilistico](https://github.com/Architetto69/nelo-protocol/blob/main/.github/docs/SPEC-002_QUORUM.md)

---

### 4.2.4 Interdiction Layer

Livelli:

1. **Level 0:** nessuna azione
2. **Level 1:** notifica
3. **Level 2:** limitazione
4. **Level 3:** blocco

---

### 4.2.5 Audit Layer

* logging immutabile
* accesso pubblico o verificabile
* MUST supportare explainability

---

# 5. Protocolli

## 5.1 Protocollo di rilevamento

```id="det1"
Input → Local Analysis → Risk Calculation → Broadcast
```

---

## 5.2 Protocollo di consenso

```id="cons2"
Receive Risk → Validate → Vote → Aggregate
```

---

## 5.3 Protocollo di interdizione

```id="int1"
IF RI > Threshold THEN
    execute_interdiction(level)
ENDIF
```

---

## 5.4 Protocollo di emergenza

Attivazione:

```id="em1"
IF Global_RI > Critical THEN Emergency_Mode = TRUE
```

Effetti:

* quorum ridotto
* latenza minima
* durata limitata (timeout obbligatorio)

---

# 6. Gestione identità e memoria

## 6.1 Principi

* minimizzazione dati
* separazione identità / evento
* eliminare radicalmente la possibilità di tracciamento individuale

Qui il [Protocollo di Oblio](https://github.com/Architetto69/nelo-protocol/blob/main/.github/docs/SPEC-003_OBLIO.md)

---

## 6.2 Implementazione

### PRIVATE_RAM

* volatile
* dati personali temporanei

### PUBLIC_LEDGER

* eventi aggregati
* anonimizzati

---

# 7. Sicurezza

## 7.1 Requisiti

NELO MUST:

* resistere a attacchi Byzantine
* evitare single point of failure
* supportare crittografia end-to-end

---

## 7.2 Protezioni

* Distributed Ledger
* Secure Enclaves
* Federated Learning

---

# 8. Hardware Constraints

* alcune regole MUST essere enforce a livello hardware
* es. limiti di esecuzione, accesso, firma

NOTA:

* non esiste sicurezza assoluta
* il sistema deve rendere l’attacco **economicamente proibitivo**

---

# 9. Metriche

## 9.1 Key Metrics

* **Harm Reduction Rate (HRR)**

```id="m1"
HRR = (baseline_harm - actual_harm) / baseline_harm
```

---

* **Interdiction Accuracy (IA)**

```id="m2"
IA = true_positive / (true_positive + false_positive)
```

---

* **System Latency (SL)**
* tempo tra rilevamento e azione

---

* **Decentralization Index (DI)**
* distribuzione dei nodi decisionali

---

* **Cost of Harm Scaling (CHS)**

```id="m3"
CHS = cost_with_NELO / cost_without_NELO
```

---

# 10. Limiti del sistema

NELO:

* MUST NOT garantire eliminazione totale del danno
* MUST accettare falsi positivi/negativi
* MUST operare su probabilità, non certezze

---

# 11. Compliance etica

Il sistema MUST:

* rispettare autonomia individuale
* evitare paternalismo
* limitarsi a danni sistemici

---

# 12. Deployment

## 12.1 Fasi

1. prototipo locale
2. rete federata
3. integrazione infrastrutturale
4. scaling globale

---

## 12.2 Requisiti minimi

* rete distribuita attiva
* nodi indipendenti ≥ 100
* infrastruttura crittografica

---

# 13. Conclusione

NELO è definito come:

> **un’infrastruttura distribuita che limita la scalabilità del danno senza centralizzare il potere**

Non è:

* governo
* intelligenza sovrana

È:

* **vincolo operativo sul sistema**

---

# Stato

```
STATUS: DRAFT → IMPLEMENTABLE
MODE: PASSIVE INTERDICTION
AUTHORITY: NONE
