# connect

`connect` - Etablit une connexion


## Synopsis

```c
#include "syscall.h"

int ConnectionManager::Connect(int remoteAddr, int remotePort, int localPort);
```

## Description

`connect` permet d'établir une connexion à un serveur distant

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
| 1 | `E_INVAL` |  |
| 107 | `E_NOPORT` | Port non défini |
| 101 | `E_NOTCONN` | Echec de connexion |
| 102 | `E_ADDRINUSE` | Port déjà utilisé |

## Implémentation

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

## Exemples

### Exemple : Connexion d'un client

```c
#include "syscall.h"

int main() {
    ...
    int connId = connect(SERVER_ADDR, SERVER_PORT, 0);
    if (connId < 0) {
        printf("[Client] ERROR: connect() failed with %d\n", connId);
        return 1;
    }
    printf("[Client] Connected! (connId=%d)\n", connId);
    ...
}
```

## Limitations


## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
