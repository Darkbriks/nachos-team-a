# connect

`connect` - Etablit une connexion


## Synopsis

```c
#include "syscall.h"

int connect(int remoteAddr, int remotePort, int localPort);
```

## Description

`connect` permet d'établir une connexion à un serveur distant

## Comportement nominal

- vérifie si les ports sont valides ou déjà utilisés
- alloue une connexion
- récupère la connexion
- remplit la structure de connexion
- envoie un message de contrôle
- vérifie que la connexion soit établie

## Cas particuliers

- **remoteAddr < 0 || remoteAddr > 9** : Retourne -1, `errno = E_INVAL`
- **remotePort <= 0 || remotePort > 65535** : Retourne -1, `errno = E_INVAL`
- **localPort <= 0 || localPort > 65535** : Retourne -1, `errno = E_INVAL`
- **mgrr == nullptr** : Retourne -1, `errno = E_NOSYS`
- **mgr->IsValidConnection(result) == false** : Retourne -1, `errno = E_NOTCONN`

## Paramètres

### `remoteAddr`

**Type** : `int`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : L'adresse doit exister et être accessible.

### `remotePort`

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`  
**Contraintes** : Le port doit exister et être accessible.

### `localPort`

**Type** : `int`  
**Direction** : IN  
**Registre** : `$6`  
**Contraintes** : Le port doit exister et être accessible.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `connId` | Identifiant de connexion |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 105 | `E_TIMEOUT` | Temps écoulé |
| 100 | `E_REFUSED` | Connexion refusée |
| 1 | `E_INVAL` | Paramètre invalide |
| 107 | `E_NOPORT` | Port non défini |
| 101 | `E_NOTCONN` | Echec de connexion |
| 102 | `E_ADDRINUSE` | Port déjà utilisé |

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_connect()`
- **Implémentation** : `code/network/connectionmanager.cc:Connect(...)`

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
        │ └─ GetConnectionManager()
        ▼
    Connect(remoteAddr, remotePort, localPort)
        │ ├─ AllocateEphemeralPort()
        │ ├─ connId = AllocateConnection()
        │ ├─ GetConnection(connId)
        │ └─ SendControlMessage(MSG_SYN,        
        |                       initialSeqNum, 0)
        | └─ FreeConnection(connId) 
        ▼
```

## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## VOIR AUSSI

- [Accept](./Accept.md) - Accepter une connexion
- [Close](./Close.md) - Fermer une connexion
- [Listen](./Listen.md) - Ecouter les demandes de connexions
- [sendto](./Send.md) - Envoyer des données
- [recvfrom](./Recv.md) - Recevoir des données


## Auteurs

Victor, 21 Jan 2026

## Dernière révision

22 Jan 2026
