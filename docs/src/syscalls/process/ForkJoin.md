# ForkJoin

`ForkJoin` - Attend la fin d'un autre process

## Synopsis

```c
#include "syscall.h"

int ForkJoin(int PID, int *exitcode);
```

## Description

`ForkJoin` Fait attendre au thread du process appelant la terminaison d'un process qu'il a crée.

**Numéro d'appel système** : `SC_ForkJoin` (35)

### Comportement nominal

1. Vérification du PID donné
2. Attente de la fin du processus
3. Retourne la valeur de l'exitcode du processus

## Paramètres

### `PID`

The process id for the process we want to wait

**Type** : `posix_process_t`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le process doit exister et être un enfant du process appelant

### `exitcode`

An adress to put the exitcode of the process

**Type** : `int *`  
**Direction** : OUT
**Registre** : `$5`  
**Contraintes** : L'adresse doit être valide et initialisée

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| ` 0` | Succès et le retour est dans l'exitcode|
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | `PID` invalid. Process try to wait himself. Or adress for exitcode is wrong |
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
        │ charge $5 = addr_result
        ▼
    syscall SC_ForkJoin
        │
        ▼
    handle_SC_ForkJoin()
        │ ├─ lit $4
        │ ├─ lit $5
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

### Exemple 1 : Création simple de deux processus

```c
#include "syscall.h"

int main(){
    int result = -1;
    ForkJoin(ForkExec("./userpages0"), &result);
    PutString("All processes are finished.\n", 27);
    return 0;
}
```


## Auteurs

Tommy, 8 Jan 2026

## Dernière révision

8 Jan 2026

