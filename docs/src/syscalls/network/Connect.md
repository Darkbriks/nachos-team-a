# Connect

`Connect` - Etablit une connexion


## Synopsis

```c
#include "syscall.h"

int ConnectionManager::Connect(NetworkAddress remoteAddr, uint16_t remotePort, uint16_t localPort)
```

## Description

`Connect` crée un nouveau process qui va éxècuter l'éxècutable dont le nom est donné en paramètre

**Numéro d'appel système** : `SC_connect` (31)

### Comportement nominal

1. Allocation d'un PID unique via `processs_bitmap`
2. Création d'un objet `Process` noyau
3. Création de son `AddrSpace`
4. Création du thread principal du `Process` avec l'éxècutable donné comme paramétre
5. Retour immédiat (le nouveau process s'exécute de manière asynchrone)

## Paramètres

### `remoteAddr`

Argument passé à `start_routine`.

**Type** : `char *`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le fichier doit exister et être éxècutable.

### `remotePort`

Argument passé à `start_routine`.

**Type** : `char *`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Le fichier doit exister et être éxècutable.

### `localPort`

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
| 1 | `E_TIMEOUT` | `file_name` invalide ou Fichier pas trouvé |
| 7 | `E_REFUSED` | Plus de PID disponibles (limite atteinte) |
| 7 | `E_INVAL` | Plus de PID disponibles (limite atteinte) |
| 7 | `E_NOPORT` | Plus de PID disponibles (limite atteinte) |
| 7 | `E_NOTCONN` | Plus de PID disponibles (limite atteinte) |
| 7 | `E_ADDRINUSE` | Plus de PID disponibles (limite atteinte) |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_connect()`
- **Implémentation** : `code/network/connectionmanager.cc:Connect()`
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

8 Jan 2026
