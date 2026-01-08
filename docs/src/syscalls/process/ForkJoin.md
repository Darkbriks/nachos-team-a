# ForkJoin

`ForkJoin` - Attend la fin d'un autre process

## Synopsis

```c
#include "syscall.h"

int ForkJoin(int PID);
```

## Description

`ForkJoin` Fait attendre au thread du process appelant la terminaison d'un process qu'il a crée.

**Numéro d'appel système** : `SC_ForkJoin` (35)

### Comportement nominal

1. Vérification du PID donné
2. Attente de la fin du processus
5. Retour

## Paramètres

### `PID`

The process id for the process we want to wait

**Type** : `posix_process_t`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le process doit exister et être un enfant du process appelant

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | `PID` invalid. Process try to wait himself |
| 9 | `E_NOSPC` |`PID` process is not found  |
| 12 | `E_NOCPC` | `PID` is not a child of the current process |


## Implémentation


### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userprocess.cc:handle_SC_ForkJoin()`
- **Implémentation** : `code/userprog/userprocess.cc:handle_SC_ForkJoin()`
- **Attente** : `code/userprog/process.cc:WaitForChild(Process* child)`

### Flux d'exécution

```
ForkJoin(PID)
        │
        ▼
    start.S: ForkJoin
        │ charge $4 = PID
        ▼
    syscall SC_ForkJoin
        │
        ▼
    handle_SC_ForkJoin()
        │ ├─ lit $4
        │ ├─ valide le PID
        │ └─ currentThread->getProcess()->WaitForChild(child)
        ▼
    WaitForChild(child)
        │ └─ Attend le process
        ▼
```

## Exemples

### Exemple 1 : Création simple de deux processus

```c
#include "syscall.h"

int main(){
    ForkJoin(ForkJoin("./userpages0"));
    PutString("All processes are finished.\n", 27);
    return 0;
}
```


## Auteurs

Tommy, 8 Jan 2026

## Dernière révision

8 Jan 2026

