# ForkExec

`ForkExec` - Crée un nouveau process utilisateur

<div class="callout-todo"> Adapter la page </div>

## Synopsis

```c
#include "syscall.h"

int ForkExec(char* file_name);
```

## Description

`ForkExec` crée un nouveau process qui exécute la fonction `start_routine` avec l'argument `arg`. Le nouveau process partage le même espace d'adressage que le process appelant.

**Numéro d'appel système** : `SC_ForkExec` (34)

### Comportement nominal

1. Allocation d'un TID unique via `processs_bitmap`
2. Création d'un objet `Thread` noyau
3. Configuration de l'état de détachement selon `attr`
4. Calcul de l'adresse de stack : `numPages * PageSize - (TID + 1) * stackSize`
5. Fork du process noyau avec `StartUserThread`
6. Écriture du TID à l'adresse `process`
7. Retour immédiat (le nouveau process s'exécute de manière asynchrone)

### Initialisation des registres

Le nouveau process démarre avec :

| Registre | Valeur |
|----------|--------|
| `$pc` | Adresse de `start_routine` |
| `$sp` | Sommet de la stack allouée |
| `$a0` ($4) | Valeur de `arg` |
| `$ra` | Adresse de `PprocessExit_wrapper` |

<div class="callout callout-note">
    <div class="callout-title">Terminaison automatique</div>
    <div class="callout-content">
        Le registre <code>$ra</code> est configuré pour que le retour de <code>start_routine</code> 
        appelle automatiquement <code>PprocessExit</code> avec la valeur de retour.
    </div>
</div>

## Paramètres

### `process`

Pointeur vers un `posix_process_t` où sera stocké l'identifiant du nouveau process.

**Type** : `posix_process_t *` (pointeur vers `unsigned int`)  
**Direction** : OUT  
**Registre** : `$4`  
**Contraintes** : Doit être une adresse valide en espace utilisateur

### `attr`

Attributs de création du process. Si `NULL` ou `0`, les attributs par défaut sont utilisés.

**Type** : `const pprocess_attr_t *`  
**Direction** : IN  
**Registre** : `$5`  
**Valeur par défaut** : `JOINABLE`, stack de 2 pages

### `start_routine`

Fonction à exécuter par le nouveau process.

**Type** : `void *(*)(void *)`  
**Direction** : IN  
**Registre** : `$6`  
**Contraintes** : Adresse valide de fonction utilisateur

### `arg`

Argument passé à `start_routine`.

**Type** : `void *`  
**Direction** : IN  
**Registre** : `$7`  
**Contraintes** : Aucune (peut être `NULL`)

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
- **Implémentation** : `code/userprog/userprocess.cc:do_ForkExec()`
- **Démarrage process** : `code/userprog/userprocess.cc:StartUserThread()`

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
        │ ├─ valide start_routine
        │ ├─ lit/initialise attr
        │ ├─ crée Thread via Process::CreateThread()
        │ ├─ configure detached selon attr
        │ ├─ crée Param(start_routine, arg, wrapper_addr)
        │ └─ process->Fork(StartUserThread, param)
        ▼
    [scheduler active le nouveau process]
        │
        ▼
    StartUserThread(param)
        │ ├─ calcule stack_addr
        │ ├─ InitRegisters()
        │ ├─ configure PC, SP, $a0, $ra
        │ └─ machine->Run()
        ▼
    [exécution de start_routine(arg)]
```

## Exemples

### Exemple 1 : Création simple

```c
#include "syscall.h"

void *hello_process(void *arg) {
    PutString("Hello from process!\n", 19);
    return 0;
}

int main() {
    posix_process_t tid;
    
    if (ForkExec(&tid, 0, hello_process, 0) != 0) {
        PutString("Error\n", 6);
        return 1;
    }
    
    PprocessJoin(tid, 0);
    return 0;
}
```

### Exemple 2 : Passage d'argument et récupération de retour

```c
#include "syscall.h"

void *compute(void *arg) {
    int value = (int)(long)arg;
    return (void *)(long)(value * value);
}

int main() {
    posix_process_t tid;
    void *result;
    
    ForkExec(&tid, 0, compute, (void *)7);
    PprocessJoin(tid, &result);
    
    PutString("7^2 = ", 6);
    PutInt((int)(long)result);  // Affiche 49
    PutChar('\n');
    
    return 0;
}
```

### Exemple 3 : Thread détaché

```c
#include "syscall.h"

void *background_task(void *arg) {
    PutString("Background task running\n", 24);
    // Pas besoin de join
    return 0;
}

int main() {
    posix_process_t tid;
    pprocess_attr_t attr;
    
    Pprocess_attr_init(&attr);
    Pprocess_attr_setdetachstate(&attr, DETACHED);
    
    ForkExec(&tid, &attr, background_task, 0);
    
    Pprocess_attr_destroy(&attr);
    
    // Le process principal ne se termine pas tant que le process détaché n'a pas fini
    
    return 0;
}
```

### Exemple 4 : Gestion d'erreur

```c
#include "syscall.h"

void *worker(void *arg) {
    return 0;
}

int main() {
    posix_process_t tids[20];
    int i;
    
    for (i = 0; i < 20; i++) {
        if (ForkExec(&tids[i], 0, worker, 0) != 0) {
            int err = GetLastError();
            PutString("Failed at process ", 17);
            PutInt(i);
            PutString(", errno=", 8);
            PutInt(err);
            PutChar('\n');
            break;  // Limite de processs atteinte
        }
    }
    
    // Joindre les processs créés
    while (--i >= 0) {
        PprocessJoin(tids[i], 0);
    }
    
    return 0;
}
```

## Limitations

<div class="callout callout-limitation">
    <div class="callout-title">Stack statique</div>
    <div class="callout-content">
        La stack est allouée statiquement. Impossible de spécifier une taille de stack personnalisée 
        ou d'utiliser une stack pré-allouée.
    </div>
</div>

<div class="callout callout-limitation">
    <div class="callout-title">Limite de processs</div>
    <div class="callout-content">
        Maximum <code>MAX_THREAD</code> processs par processus. Au-delà, <code>E_NOMEM</code> est retourné.
    </div>
</div>

## Thread-safety

L'appel est process-safe. Plusieurs processs peuvent appeler `ForkExec` simultanément grâce à la protection du `processs_bitmap` par `processNumberLock`.

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PprocessJoin](./PprocessJoin.md) - Attendre un process
- [PprocessExit](./PprocessExit.md) - Terminer un process
- [PprocessDetach](./PprocessDetach.md) - Détacher un process
- [Attributs](./attrs.md) - Configuration des attributs

</div>
</div>

## Auteurs

Antoine, 31 Dec 2025

## Dernière révision

31 Dec 2025
