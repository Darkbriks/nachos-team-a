# thread_self

`thread_self` - Retourne l'identifiant du thread courant.

## Synopsis

```c
#include "syscall.h"

int thread_self();
```

## Description

`thread_self` retourne le TID (Thread IDentifier) du thread appelant. Cet identifiant est unique au sein du processus.

### Comportement

L'appel retourne immédiatement le TID stocké dans la structure du thread courant.

## Paramètres

Aucun paramètre.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification                   |
|--------|---------------------------------|
| `0`    | Thread principal du processus   |
| `1..N` | Thread créé via `thread_create` |

<div class="callout callout-note">
    <div class="callout-title">Pas d'erreur possible</div>
    <div class="callout-content">
        <code>thread_self</code> réussit toujours. Il n'y a pas de cas d'erreur possible
        car le thread courant existe nécessairement s'il peut exécuter cet appel.
    </div>
</div>

## Exemples

### Exemple : Identification simple

```c
#include "syscall.h"

void worker(int arg) {
    int tid = thread_self();
    
    PutString("Je suis le thread ", 18);
    PutInt(tid);
    PutChar('\n');
    
    thread_exit();
}

int main() {
    PutString("Thread principal: TID=", 22);
    PutInt(thread_self());  // Affiche 0
    PutChar('\n');
    
    // Créer des threads...
    
    return 0;
}
```

## Propriétés du TID

### Unicité

- Un TID est unique **au sein d'un processus**
- Deux processus différents peuvent avoir des threads avec le même TID
- Le TID 0 est toujours le thread principal

### Recyclage

- Les TID sont recyclés après la terminaison d'un thread
- Un nouveau thread peut recevoir le TID d'un thread précédemment terminé

## Thread-safety

L'appel est intrinsèquement thread-safe :
- Chaque thread lit son propre TID
- Aucune donnée partagée n'est modifiée
- Pas de race condition possible

## Voir aussi

- [thread_create](./thread_create.md) - Créer un thread
- [thread_exit](./thread_exit.md) - Terminer un thread
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026