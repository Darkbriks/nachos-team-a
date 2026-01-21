# accept

`accept` - Accepte une connexion


## Synopsis

```c
#include "syscall.h"

int accept(int listenerId, int timeoutMs);
```

## Description

`accept` permet d'accepter une connexion sur un port d'écoute

## Comportement nominal



## Cas particuliers

## Paramètres

### `listenerId`

Identifiant renvoyé par listen

**Type** : `int`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le port doit exister et être accessible.

### `timeoutMS`

Timeout en millisecondes

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`  
**Contraintes** : -1 = infini, 0 = non-bloquant

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `connId` | Identifiant de connexion |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` |  |

## Implémentation

### Flux d'exécution

```
connect(remoteAddr, remotePort, localPort)
        │
        ▼
    start.S: ForkJoin
        │ charge $4 = remoteAddr
        │ charge $5 = remotePort
        │ charge $6 = localPort
        ▼
    syscall SC_connect
        │
        ▼
    handle_SC_connect()
        │ ├─ lit $4
        │ ├─ lit $5
        │ ├─ lit $6
        │ ├─ valide le PID
        │ └─ currentThread->getProcess()->WaitForChild(child)
        ▼
    WaitForChild(child, addr_result)
        │ ├─ valide le PID
        │ ├─ Attend le process 
        │ ├─ Met l'exitcode dans la mémoire 
        │ └─ Détruit le process finit
        ▼
```
## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
