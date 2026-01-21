# close

`close` - Termine une connexion


## Synopsis

```c
#include "syscall.h"

int close(int id);
```

## Description

`close` permet de terminer une connexion ou stopper un listener

## Paramètres

### `id`

Identifiant de la connexion ou du listener

**Type** : `int`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : L'identifiant doit exister et être accessible.

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

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_close()`
- **Implémentation** : `code/network/connectionmanager.cc:Close(...)`

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

### Exemple : Fermeture simple d'une connexion

```c
#include "syscall.h"

int main(){
    ...
    close(connId);
}
```


## Limitations

## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
