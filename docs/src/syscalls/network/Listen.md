# Listen

`Listen` - Définit un port d'écoute


## Synopsis

```c
#include "syscall.h"

int listen(int port);
```

## Description

`Listen` permet de définir le port donné en argument comme port d'écoute

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
| 1 | `E_INVAL` |  |
| 102 | `E_ADDRINUSE` | Port déjà utilisé |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_listen()`
- **Implémentation** : `code/network/connectionmanager.cc:Listen(...)`

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

### Exemple : Ecoute d'un serveur

```c
#include "syscall.h"

int main(){
    ...
    int listenerId = listen(SERVER_PORT);
    if (listenerId < 0) {
        printf("[Server] ERROR: listen() failed with %d\n", listenerId);
        return 1;
    }
    printf("[Server] Listening on port %d (listenerId=%d)\n", SERVER_PORT, listenerId);
    ...
}
```


## Limitations

## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
