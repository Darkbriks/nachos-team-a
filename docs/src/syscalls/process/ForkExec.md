# ForkExec

`ForkExec` - Crée un nouveau process utilisateur


## Synopsis

```c
#include "syscall.h"

int ForkExec(char* file_name);
```

## Description

`ForkExec` crée un nouveau process qui va éxècuter l'éxècutable dont le nom est donné en paramètre

**Numéro d'appel système** : `SC_ForkExec` (34)

### Comportement nominal

1. Allocation d'un PID unique via `processs_bitmap`
2. Création d'un objet `Process` noyau
3. Création de son `AddrSpace`
4. Création du thread principal du `Process` avec l'éxècutable donné comme paramétre
5. Retour immédiat (le nouveau process s'exécute de manière asynchrone)

## Paramètres

### `file_name`

Argument passé à `start_routine`.

**Type** : `char *`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le fichier doit exister et être éxècutable.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | `start_routine` invalide ou écriture mémoire échouée |
| 2 | `E_FAULT` | Adresse `process` invalide |
| 7 | `E_NOMEM` | Plus de TID disponibles (limite atteinte) |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userprocess.cc:handle_SC_ForkExec()`
- **Implémentation** : `code/userprog/userprocess.cc:handle_SC_ForkExec()`
- **Démarrage process** : `code/userprog/userprocess.cc:StartProcess()`

### Flux d'exécution

```
ForkExec(&tid, attr, func, arg)
        │
        ▼
    start.S: ForkExec
        │ charge $8 = PprocessExit_wrapper
        ▼
    syscall SC_ForkExec
        │
        ▼
    handle_SC_ForkExec()
        │ lit $4, $5, $6, $7, $8
        ▼
    do_ForkExec()
        │ ├─ valide le fichier (éxècutable)
        │ ├─ crée Process via Process::createProcess(éxècutable)
        │ ├─ lit/initialise contexte d'éxècution
        │ └─ mainThread->Fork(StartProcess, newProcess)
        ▼
    [scheduler active le nouveau process]
        │
        ▼
    StartProcess(param)
        │ ├─ initialilse registres et adresses virtuelles
        │ └─ machine->Run()
        ▼
    [exécution de main()]
```

## Exemples

### Exemple 1 : Création simple de deux processus

```c
#include "syscall.h"

int main(){
    ForkExec("./userpages0");
    ForkExec("./userpages1");
    PutString("All processes launched.\n", 26);
    return 0;
}
```


## Limitations

<div class="callout callout-limitation">
    <div class="callout-title">Limite de processs</div>
    <div class="callout-content">
        Maximum <code>MAX_PROCESS</code> processs en même temps vivant sur la machine. Au-delà, <code>E_NOMEM</code> est retourné.
    </div>
</div>

## Auteurs

Tommy, 7 Jan 2026

## Dernière révision

7 Jan 2026
