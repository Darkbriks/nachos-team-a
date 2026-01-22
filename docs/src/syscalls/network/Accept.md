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

- récupère le listener
- vérifie que le timeout n'est pas dépassé
- vérifie que la connexion est acceptée

## Cas particuliers

- **listenerId < 0** : Retourne -1, `errno = E_INVAL`
- **mgrr == nullptr** : Retourne -1, `errno = E_NOSYS`

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
| 1 | `E_INVAL` | Paramètre invalide |

### Flux d'exécution

```
accept(listenerId, timeoutMS)
        │
        ▼
    start.S: ForkJacceptoin
        │ charge $4 = listenerId
        │ charge $5 = timeoutMS
        ▼
    syscall SC_accept
        │
        ▼
    handle_SC_accept()
        │ ├─ lit $4
        │ ├─ lit $5
        │ └─ GetConnectionManager()
        ▼
    Accept(listenerId, timeoutMS)
        │ ├─ GetListener()
        │ ├─ Wait()
        │ └─ FreeConnection()
        ▼
```
## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## VOIR AUSSI

- [Close](./Close.md) - Fermer une connexion
- [Connect](./Connect.md) - Connexion à une autre machine
- [Listen](./Listen.md) - Ecouter les demandes de connexions
- [sendto](./Send.md) - Envoyer des données
- [recvfrom](./Recv.md) - Recevoir des données

## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
