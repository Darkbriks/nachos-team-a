# ForkSelf

`ForkSelf` - Obtient le PID du processus courant

## Synopsis

```c
#include "syscall.h"

int ForkSelf(void);
```

## Description

`ForkSelf` retourne l'identifiant unique (PID) du processus appelant.

**Numéro d'appel système** : `SC_ForkSelf` (29)

### Comportement nominal

1. Récupère le processus courant via `currentThread->getProcess()`
2. Retourne le PID du processus

## Paramètres

Aucun paramètre.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `>= 0` | PID du processus courant |


## Implémentation


### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userprocess.cc:handle_SC_ForkSelf()`

### Flux d'exécution

```
ForkSelf()
        │
        ▼
    start.S: ForkSelf
        │
        ▼
    syscall SC_ForkSelf
        │
        ▼
    handle_SC_ForkSelf()
        │ └─ currentThread->getProcess()->getPId()
        ▼
    [retourne PID dans $2]
```

## Exemples

### Exemple 1 : Afficher son propre PID

```c
#include "syscall.h"

int main() {
    int myPid = ForkSelf();
    PutString("Mon PID est: ", 13);
    PutInt(myPid);
    PutChar('\n');
    return 0;
}
```

### Exemple 2 : Distinguer parent et enfant

```c
#include "syscall.h"

int main() {
    int parentPid = ForkSelf();
    int childPid = ForkExec("./child_program");

    PutString("Je suis le processus ", 22);
    PutInt(parentPid);
    PutString(", j'ai lance le processus ", 28);
    PutInt(childPid);
    PutChar('\n');

    return 0;
}
```


## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026

