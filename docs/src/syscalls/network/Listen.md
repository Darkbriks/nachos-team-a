# Listen

`Listen` - Définit un port d'écoute


## Synopsis

```c
#include "syscall.h"

int listen(int port);
```

## Description

`Listen` permet de définir le port donné en argument comme port d'écoute

## Comportement nominal

- vérifie que le port soit valide
- trouve la connection par le port local
- alloue le listener

## Cas particuliers

- **port <= 0 || port > 65535** : Retourne -1, `errno = E_INVAL`

## Paramètres

### `port`

Numéro de port à écouter

**Type** : `uint16_t`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le port doit exister et être accessible.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `AllocateListener(port)` | Identifiant de connexion |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | Paramètre invalide |
| 102 | `E_ADDRINUSE` | Port déjà utilisé |

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_listen()`
- **Implémentation** : `code/network/connectionmanager.cc:Listen(...)`

### Flux d'exécution

```
listen(remoteAddr, remotePort, localPort)
        │
        ▼
    start.S: listen
        │ charge $4 = remoteAddr
        │ charge $5 = remotePort
        │ charge $6 = localPort
        ▼
    syscall SC_listen
        │
        ▼
    handle_SC_listen()
        │ ├─ lit $4
        │ └─ GetConnectionManager()
        ▼
    Listen(port)
        │ ├─ FindConnectionByLocalPort(port)
        │ └─ AllocateListener(port)
        ▼
```

## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## VOIR AUSSI

- [Accept](./Accept.md) - Accepter une connexion
- [Close](./Close.md) - Fermer une connexion
- [Connect](./Connect.md) - Connexion à une autre machine
- [sendto](./Send.md) - Envoyer des données
- [recvfrom](./Recv.md) - Recevoir des données

## Limitations

## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
