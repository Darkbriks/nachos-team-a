# PthreadJoin

`PthreadJoin` - Attend la terminaison d'un thread

## Synopsis

```c
#include "syscall.h"

int PthreadJoin(posix_thread_t thread, void **retval);
```

## Description

`PthreadJoin` bloque le thread appelant jusqu'à ce que le thread spécifié se termine. Si le thread cible est déjà terminé, la fonction retourne immédiatement.

**Numéro d'appel système** : `SC_PthreadJoin` (19)

### Comportement nominal

1. Recherche du thread cible via `Process::FindThread()`
2. Vérification des conditions (non-soi, non-détaché, non-déjà-joint)
3. Configuration de la relation thread joiner ↔ thread join
4. Appel à `thread->Join()` (bloquant sur sémaphore)
5. Récupération de la valeur de retour
6. Suppression du thread de la liste du processus
7. Retour au thread appelant

### Récupération de la valeur de retour

Si `retval` n'est pas `NULL`, la valeur passée à `PthreadExit()` (ou retournée par `start_routine`) est écrite à cette adresse.

<div class="callout callout-note">
    <div class="callout-title">Thread terminé</div>
    <div class="callout-content">
        Un thread terminé reste dans la liste des threads du processus jusqu'à ce qu'il soit 
        joint ou détaché. C'est le <code>PthreadJoin</code> qui libère les ressources.
    </div>
</div>

## Paramètres

### `thread`

Identifiant du thread à attendre.

**Type** : `posix_thread_t` (`unsigned int`)  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Doit être un TID valide d'un thread joinable

### `retval`

Pointeur où stocker la valeur de retour du thread. Peut être `NULL` si la valeur n'est pas nécessaire.

**Type** : `void **`  
**Direction** : OUT  
**Registre** : `$5`  
**Contraintes** : Si non-NULL, doit être une adresse valide en espace utilisateur

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification              |
|--------|----------------------------|
| `0`    | Succès                     |
| `-1`   | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition                                    |
|-------|-----------|----------------------------------------------|
| 1     | `E_INVAL` | Self-join, thread détaché, ou déjà un joiner |
| 2     | `E_FAULT` | Écriture de `retval` échouée                 |
| 9     | `E_NOSPC` | Thread non trouvé (TID invalide)             |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userthread.cc:handle_SC_PthreadJoin()`
- **Implémentation** : `code/userprog/userthread.cc:do_PthreadJoin()`

## Exemples

### Exemple 1 : Join basique

```c
#include "syscall.h"

void *worker(void *arg) {
    PutString("Worker done\n", 12);
    return (void *)42;
}

int main() {
    posix_thread_t tid;
    void *result;
    
    PthreadCreate(&tid, 0, worker, 0);
    
    PutString("Waiting for worker...\n", 22);
    PthreadJoin(tid, &result);
    
    PutString("Worker returned: ", 17);
    PutInt((int)(long)result);
    PutChar('\n');
    
    return 0;
}
```

### Exemple 2 : Join sans récupérer la valeur

```c
#include "syscall.h"

void *task(void *arg) {
    // Travail sans valeur de retour importante
    return 0;
}

int main() {
    posix_thread_t tid;
    
    PthreadCreate(&tid, 0, task, 0);
    
    // On attend juste la fin, pas besoin de la valeur
    PthreadJoin(tid, 0);
    
    return 0;
}
```

### Exemple 3 : Attendre plusieurs threads

```c
#include "syscall.h"

#define NUM_THREADS 5

void *compute(void *arg) {
    int id = (int)(long)arg;
    return (void *)(long)(id * id);
}

int main() {
    posix_thread_t tids[NUM_THREADS];
    void *results[NUM_THREADS];
    int i, sum = 0;
    
    // Créer les threads
    for (i = 0; i < NUM_THREADS; i++) {
        PthreadCreate(&tids[i], 0, compute, (void *)(long)i);
    }
    
    // Attendre tous les threads
    for (i = 0; i < NUM_THREADS; i++) {
        PthreadJoin(tids[i], &results[i]);
        sum += (int)(long)results[i];
    }
    
    PutString("Sum of squares: ", 16);
    PutInt(sum);  // 0 + 1 + 4 + 9 + 16 = 30
    PutChar('\n');
    
    return 0;
}
```

### Exemple 4 : Gestion des erreurs

```c
#include "syscall.h"

void *worker(void *arg) {
    return 0;
}

int main() {
    posix_thread_t tid;
    
    // Test 1: Join sur soi-même
    if (PthreadJoin(0, 0) != 0) {
        PutString("Cannot self-join (expected)\n", 28);
    }
    
    // Test 2: Join sur thread inexistant
    if (PthreadJoin(9999, 0) != 0) {
        PutString("Cannot join non-existent (expected)\n", 36);
    }
    
    // Test 3: Double join
    PthreadCreate(&tid, 0, worker, 0);
    PthreadJoin(tid, 0);
    
    if (PthreadJoin(tid, 0) != 0) {
        PutString("Cannot double join (expected)\n", 30);
    }
    
    return 0;
}
```

## Cas particuliers

### Thread déjà terminé

Si le thread cible a déjà terminé (état `TERMINATED`), `PthreadJoin` retourne immédiatement avec la valeur de retour.

### Double join

Un thread ne peut être joint qu'une seule fois. Après un join réussi, le thread est supprimé de la liste et tout join subséquent échoue avec `E_NOSPC`.

### Join sur thread détaché

<div class="callout callout-warning">
    <div class="callout-title">Threads détachés</div>
    <div class="callout-content">
        Impossible de joindre un thread détaché. L'erreur <code>E_INVAL</code> est retournée.
    </div>
</div>

## Thread-safety

L'appel est thread-safe. Un seul thread peut joindre un thread donné grâce au flag `hasJoiner`.

<div class="callout callout-warning">
    <div class="callout-title">Un seul joiner</div>
    <div class="callout-content">
        Si deux threads tentent de joindre le même thread, le second échoue avec <code>E_INVAL</code>.
    </div>
</div>

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Créer un thread
- [PthreadExit](./PthreadExit.md) - Terminer un thread
- [PthreadDetach](./PthreadDetach.md) - Détacher un thread

</div>
</div>

## Auteurs

Antoine, 31 Dec 2025

## Dernière révision

31 Dec 2025