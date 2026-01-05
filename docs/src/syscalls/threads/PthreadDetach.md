# PthreadDetach

`PthreadDetach` - Détache un thread

## Synopsis

```c
#include "syscall.h"

int PthreadDetach(posix_thread_t thread);
```

## Description

`PthreadDetach` marque le thread spécifié comme détaché. Un thread détaché ne peut plus être joint, et ses ressources sont automatiquement libérées à sa terminaison.

**Numéro d'appel système** : `SC_PthreadDetach` (20)

### Comportement nominal

1. Recherche du thread via `Process::FindThread()`
2. Vérification que le thread n'est pas déjà détaché
3. Vérification qu'aucun thread n'attend déjà un join
4. Marquage du thread comme détaché
5. Si déjà terminé : suppression immédiate de la liste

<div class="callout callout-note">
    <div class="callout-title">Détachement ≠ Annulation</div>
    <div class="callout-content">
        Détacher un thread ne l'arrête pas. Le thread continue son exécution normalement.
        Seule la gestion des ressources à la terminaison change.
    </div>
</div>

## Paramètres

### `thread`

Identifiant du thread à détacher.

**Type** : `posix_thread_t` (`unsigned int`)  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Doit être un TID valide d'un thread non-détaché

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification              |
|--------|----------------------------|
| `0`    | Succès                     |
| `-1`   | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition                               |
|-------|-----------|-----------------------------------------|
| 1     | `E_INVAL` | Thread déjà détaché ou a déjà un joiner |
| 9     | `E_NOSPC` | Thread non trouvé (TID invalide)        |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userthread.cc:handle_SC_PthreadDetach()`
- **Implémentation** : `code/userprog/userthread.cc:do_PthreadDetach()`

### Flux d'exécution

```
PthreadDetach(tid)
        │
        ▼
    syscall SC_PthreadDetach
        │
        ▼
    do_PthreadDetach(tid)
        │ ├─ thread = FindThread(tid)
        │ ├─ if (!thread) return -E_NOSPC
        │ ├─ if (thread->isDetached()) return -E_INVAL
        │ ├─ if (thread->hasJoiner()) return -E_INVAL
        │ ├─ thread->setDetached(true)
        │ └─ if (thread->isTerminated())
        │        process->RemoveThread(thread)
        ▼
    return 0
```

## Exemples

### Exemple 1 : Détachement après création

```c
#include "syscall.h"

void *background_task(void *arg) {
    PutString("Background task running\n", 24);
    return 0;
}

int main() {
    posix_thread_t tid;
    
    // Créer un thread joinable
    PthreadCreate(&tid, 0, background_task, 0);
    
    // Le détacher
    if (PthreadDetach(tid) != 0) {
        PutString("Detach failed\n", 14);
    }
    
    // Pas besoin de join
    // Le thread libérera ses ressources automatiquement
    
    return 0;
}
```

### Exemple 2 : Création détachée (alternative)

```c
#include "syscall.h"

void *background_task(void *arg) {
    return 0;
}

int main() {
    posix_thread_t tid;
    pthread_attr_t attr;
    
    // Créer directement en mode détaché
    Pthread_attr_init(&attr);
    Pthread_attr_setdetachstate(&attr, DETACHED);
    
    PthreadCreate(&tid, &attr, background_task, 0);
    
    // Pas besoin de PthreadDetach ni de PthreadJoin
    
    Pthread_attr_destroy(&attr);
    return 0;
}
```

<div class="callout callout-tip">
    <div class="callout-title">Préférer les attributs</div>
    <div class="callout-content">
        Si vous savez dès la création que le thread sera détaché, 
        utilisez les attributs plutôt que <code>PthreadDetach</code>.
    </div>
</div>

### Exemple 3 : Gestion des erreurs

```c
#include "syscall.h"

void *worker(void *arg) {
    return 0;
}

int main() {
    posix_thread_t tid;
    
    PthreadCreate(&tid, 0, worker, 0);
    
    // Premier détachement : OK
    if (PthreadDetach(tid) != 0) {
        PutString("First detach failed\n", 20);
    } else {
        PutString("First detach OK\n", 16);
    }
    
    // Deuxième détachement : ERREUR
    if (PthreadDetach(tid) != 0) {
        PutString("Second detach failed (expected)\n", 32);
    }
    
    // Join : ERREUR (thread détaché)
    if (PthreadJoin(tid, 0) != 0) {
        PutString("Join failed (expected)\n", 23);
    }
    
    return 0;
}
```

## Cas particuliers

### Thread déjà terminé

Si le thread est déjà terminé au moment du détachement, ses ressources sont libérées immédiatement.

### Thread avec joiner en attente

<div class="callout callout-danger">
    <div class="callout-title">Conflit Detach/Join</div>
    <div class="callout-content">
        Si un autre thread attend déjà avec <code>PthreadJoin</code>, le détachement échoue avec <code>E_INVAL</code>.
    </div>
</div>

## Comparaison : Joinable vs Détaché

| Aspect           | Thread Joinable          | Thread Détaché            |
|------------------|--------------------------|---------------------------|
| Ressources       | Conservées jusqu'au join | Libérées à la terminaison |
| Valeur de retour | Récupérable via join     | Perdue                    |
| PthreadJoin      | Obligatoire              | Interdit                  |

## Thread-safety

L'appel est thread-safe. L'état de détachement est protégé par les flags du thread.

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Créer un thread
- [PthreadJoin](./PthreadJoin.md) - Attendre un thread
- [Attributs](./attrs.md) - Création détachée via attributs

</div>
</div>

## Auteurs

Antoine, 31 Dec 2025

## Dernière révision

31 Dec 2025