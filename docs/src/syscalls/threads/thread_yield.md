# thread_yield

`thread_yield` - Cède volontairement le CPU à un autre thread.

## Synopsis

```c
#include "syscall.h"

void thread_yield();
```

## Description

`thread_yield` permet au thread appelant de céder volontairement son temps CPU. Le thread est replacé dans la file des threads prêts et le scheduler choisit le prochain thread à exécuter.

### Comportement

1. Le thread courant est déplacé de l'état `RUNNING` à `READY`
2. Le thread est ajouté à la fin de la file des threads prêts
3. Le scheduler sélectionne le prochain thread à exécuter
4. Si aucun autre thread n'est prêt, le thread appelant reprend immédiatement

## Paramètres

Aucun paramètre.

## Valeur de retour

Aucune valeur de retour.

## Exemples

### Exemple : Yield simple

```c
#include "syscall.h"

void worker(int id) {
    for (int i = 0; i < 5; i++) {
        PutString("Thread ", 7);
        PutInt(id);
        PutString(" iteration ", 11);
        PutInt(i);
        PutChar('\n');
        
        thread_yield();  // Laisser les autres threads s'exécuter
    }
    
    thread_exit();
}
```

**Sortie possible** (entrelacement) :
```
Thread 1 iteration 0
Thread 2 iteration 0
Thread 1 iteration 1
Thread 2 iteration 1
...
```

## Thread-safety

L'appel est thread-safe. Chaque thread ne peut que se céder lui-même.

## Voir aussi

- [thread_create](./thread_create.md) - Créer un thread
- [thread_exit](./thread_exit.md) - Terminer un thread
- [Sleep](../time/Sleep.md) - Attendre un délai
- [futex_wait](./futex_wait.md) - Attente sur futex
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026