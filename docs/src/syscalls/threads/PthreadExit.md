# PthreadExit

`PthreadExit` - Termine le thread appelant

## Synopsis

```c
#include "syscall.h"

void PthreadExit(void *retval) __attribute__((noreturn));
```

## Description

`PthreadExit` termine le thread appelant et rend la valeur `retval` disponible pour un éventuel `PthreadJoin`. Cette fonction ne retourne jamais.

**Numéro d'appel système** : `SC_PthreadExit` (18)

### Comportement nominal

1. Stockage de `retval` dans le thread (`setReturnValue`)
2. Passage à l'état `TERMINATED`
3. Signal du sémaphore pour réveiller un éventuel joiner (`Joiner()`)
4. Notification du processus (`ThreadTerminated`)
5. Si détaché : suppression immédiate de la liste
6. Mise en sommeil définitive (`Sleep()`)

<div class="callout callout-warning">
    <div class="callout-title">Fonction noreturn</div>
    <div class="callout-content">
        Cette fonction ne retourne jamais. Tout code après l'appel est inaccessible.
    </div>
</div>

## Paramètres

### `retval`

Valeur de retour du thread, récupérable par `PthreadJoin`.

**Type** : `void *`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Aucune (peut être `NULL`, une valeur entière castée, ou un pointeur)

## Valeur de retour

Cette fonction ne retourne pas.

## Codes d'erreur

Aucun. Cette fonction ne peut pas échouer.

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userthread.cc:handle_SC_PthreadExit()`
- **Implémentation** : `code/userprog/userthread.cc:do_PthreadExit()`

### Flux d'exécution

```
PthreadExit(retval)
        │
        ▼
    start.S: PthreadExit
        │
        ▼
    syscall SC_PthreadExit
        │ [UpdatePC() NON appelé]
        ▼
    handle_SC_PthreadExit()
        │
        ▼
    do_PthreadExit(retval)
        │ ├─ thread->setReturnValue(retval)
        │ ├─ thread->setStatus(TERMINATED)
        │ ├─ thread->Joiner()  [sem->V()]
        │ ├─ process->ThreadTerminated(thread)
        │ ├─ if (detached) process->RemoveThread(thread)
        │ └─ thread->Sleep()
        ▼
    [thread ne s'exécute plus jamais]
```

<div class="callout callout-note">
    <div class="callout-title">Pas de UpdatePC</div>
    <div class="callout-content">
        Contrairement aux autres syscalls, <code>PthreadExit</code> n'appelle pas <code>UpdatePC()</code> 
        car le thread ne reprendra jamais son exécution.
    </div>
</div>

## Exemples

### Exemple 1 : Terminaison explicite

```c
#include "syscall.h"

void *worker(void *arg) {
    PutString("Working...\n", 11);
    
    // Terminaison explicite avec valeur
    PthreadExit((void *)42);
    
    // Code jamais exécuté
    PutString("NEVER PRINTED\n", 14);
}

int main() {
    posix_thread_t tid;
    void *result;
    
    PthreadCreate(&tid, 0, worker, 0);
    PthreadJoin(tid, &result);
    
    PutString("Result: ", 8);
    PutInt((int)(long)result);  // 42
    PutChar('\n');
    
    return 0;
}
```

### Exemple 2 : Terminaison implicite (return)

```c
#include "syscall.h"

void *worker(void *arg) {
    PutString("Working...\n", 11);
    
    // Return équivalent à PthreadExit
    return (void *)42;
}
```

<div class="callout callout-note">
    <div class="callout-title">Équivalence return / PthreadExit</div>
    <div class="callout-content">
        <p>Ces deux formes sont équivalentes :</p>
        <pre><code>return (void *)value;
PthreadExit((void *)value);</code></pre>
        <p>Le wrapper <code>PthreadExit_wrapper</code> appelle automatiquement 
        <code>PthreadExit</code> avec la valeur de <code>$v0</code>.</p>
    </div>
</div>

### Exemple 3 : Terminaison anticipée

```c
#include "syscall.h"

void *process_data(void *arg) {
    int *data = (int *)arg;
    int i;
    
    for (i = 0; i < 100; i++) {
        if (data[i] < 0) {
            // Erreur : terminer avec code d'erreur
            PthreadExit((void *)-1);
        }
        // Traitement...
    }
    
    return (void *)0;  // Succès
}
```

### Exemple 4 : Thread détaché

```c
#include "syscall.h"

void *daemon_thread(void *arg) {
    // Travail en arrière-plan
    PutString("Daemon running\n", 15);
    
    // La valeur de retour n'est pas récupérable
    // car le thread est détaché
    PthreadExit(0);
}

int main() {
    posix_thread_t tid;
    pthread_attr_t attr;
    
    Pthread_attr_init(&attr);
    Pthread_attr_setdetachstate(&attr, DETACHED);
    
    PthreadCreate(&tid, &attr, daemon_thread, 0);
    
    // Pas de join possible
    
    return 0;
}
```

## Libération des ressources

### Thread joinable

Les ressources (objet `Thread`) sont conservées jusqu'au `PthreadJoin`. La valeur de retour est stockée pour être récupérée.

### Thread détaché

Les ressources sont libérées immédiatement. La valeur de retour est perdue.

## Thread-safety

L'appel est thread-safe. Chaque thread ne peut appeler `PthreadExit` que pour lui-même.

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Créer un thread
- [PthreadJoin](./PthreadJoin.md) - Attendre un thread
- [PthreadDetach](./PthreadDetach.md) - Détacher un thread

</div>
</div>

## Auteurs

Antoine, 07 Jan 2026

## Dernière révision

07 Jan 2026