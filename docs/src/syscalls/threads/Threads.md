# Threads - Appels système

Cette page documente les appels système pour la gestion des threads dans NachOS.

## Vue d'ensemble

NachOS fournit un ensemble d'appels système permettant la création et la gestion de threads utilisateur. Ces syscalls constituent la couche basse sur laquelle des bibliothèques de plus haut niveau (comme pthread) peuvent être construites.

### Modèle d'exécution

NachOS utilise un modèle **1:1** où chaque thread utilisateur correspond à un thread noyau :

```
┌────────────────────────────────────────┐
│           Espace utilisateur           │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│  │Thread 0 │ │Thread 1 │ │Thread 2 │   │
│  └────┬────┘ └────┬────┘ └────┬────┘   │
├───────┼───────────┼───────────┼────────┤
│       │           │           │        │
│  ┌────▼────┐ ┌────▼────┐ ┌────▼────┐   │
│  │KThread 0│ │KThread 1│ │KThread 2│   │
│  └─────────┘ └─────────┘ └─────────┘   │
│             Espace noyau               │
└────────────────────────────────────────┘
```

## API rapide

| Syscall                             | Description                      |
|-------------------------------------|----------------------------------|
| [thread_create](./thread_create.md) | Créer un nouveau thread          |
| [thread_exit](./thread_exit.md)     | Terminer le thread courant       |
| [thread_self](./thread_self.md)     | Obtenir le TID du thread courant |
| [thread_yield](./thread_yield.md)   | Céder le CPU volontairement      |

### Primitives de synchronisation

| Syscall                       | Description                      |
|-------------------------------|----------------------------------|
| [futex_wait](./futex_wait.md) | Attendre sur un futex            |
| [futex_wake](./futex_wake.md) | Réveiller des threads en attente |

### Opérations atomiques

| Syscall                               | Description               |
|---------------------------------------|---------------------------|
| [atomic_cmpxchg](./atomic_cmpxchg.md) | Compare-and-swap atomique |
| [atomic_store](./atomic_store.md)     | Écriture atomique         |
| [atomic_load](./atomic_load.md)       | Lecture atomique          |

## Architecture

### Création de thread

Le syscall `thread_create` reçoit une structure `user_thread_args` contenant :

```c
typedef struct {
    ptr_32 entry;      // Point d'entrée (fonction)
    ptr_32 arg;        // Argument à passer
    ptr_32 user_sp;    // Stack pointer (0 = allocation auto)
    ptr_32 tls_base;   // Base du TLS (0 = pas de TLS)
} user_thread_args;
```

### Thread-Local Storage (TLS)

Chaque thread peut avoir une zone TLS pointée par le registre `$gp`. La structure TLS contient :

```c
typedef struct _tls {
    struct _tls* self;         // Auto-référence
    int errno_val;             // errno thread-local
    int pthread_ptr;           // Pointeur lib pthread
    void* tsd[TLS_MAX_KEYS];   // Données thread-specific
} tls_t;
```

### Identifiants de thread (TID)

Chaque thread possède un TID unique au sein de son processus :

- **TID 0** : Thread principal (créé automatiquement)
- **TID 1..N** : Threads créés via `thread_create`

Les TID sont alloués via un bitmap et recyclés à la terminaison du thread.

## Cycle de vie d'un thread

### États possibles

| État           | Description                            |
|----------------|----------------------------------------|
| `JUST_CREATED` | Thread créé mais pas encore démarré    |
| `READY`        | Prêt à s'exécuter, attend le scheduler |
| `RUNNING`      | En cours d'exécution                   |
| `BLOCKED`      | Bloqué (futex, sémaphore, SleepUntil)  |
| `SLEEP`        | En sommeil (Sleep)                     |
| `TERMINATED`   | Terminé, en attente de nettoyage       |

## Layout mémoire

```
Espace d'adressage du processus
┌────────────────────────────┐ ← Haut de mémoire
│      Stack thread 0        │
├────────────────────────────┤
│      Stack thread 1        │
├────────────────────────────┤
│      Stack thread 2        │
├────────────────────────────┤
│           ...              │
├────────────────────────────┤
│      Espace libre          │
├────────────────────────────┤
│          Heap              │
├────────────────────────────┤
│          .bss              │
├────────────────────────────┤
│         .data              │
├────────────────────────────┤
│         .code              │
└────────────────────────────┘ ← 0
```

## Exemple complet

```c
#include "syscall.h"
#include "nos_errno.h"

// Structure pour passer les arguments au thread
typedef struct {
    unsigned int entry;
    unsigned int arg;
    unsigned int user_sp;
    unsigned int tls_base;
} thread_args;

void thread_func(int arg) {
    PutString("Thread ", 7);
    PutInt(thread_self());
    PutString(" avec arg=", 10);
    PutInt(arg);
    PutChar('\n');
    
    thread_exit();
}

int main() {
    thread_args args;
    args.entry = (unsigned int)thread_func;
    args.arg = 42;
    args.user_sp = 0;    // Stack auto
    args.tls_base = 0;   // Pas de TLS custom
    
    int tid = thread_create(&args);
    
    if (tid == -1) {
        PutString("Erreur creation thread\n", 23);
        return 1;
    }
    
    PutString("Thread cree, TID=", 17);
    PutInt(tid);
    PutChar('\n');
    
    // Attendre un peu
    for (int i = 0; i < 100000; i++) {
        // Simuler du travail
    }
    
    return 0;
}
```

<div class="callout callout-note">
    <div class="callout-title">Bibliothèque pthread</div>
    <div class="callout-content">
        Pour une API de plus haut niveau avec join, detach, et gestion automatique des ressources,
        utilisez la bibliothèque pthread qui encapsule ces syscalls.
    </div>
</div>

## Voir aussi

- [thread_create](./thread_create.md) - Créer un thread
- [thread_exit](./thread_exit.md) - Terminer un thread
- [errno](../libs/errno.md) - Gestion des erreurs
- [Sémaphores](../sync/Sync.md) - Synchronisation haut niveau

## Auteurs

Antoine

## Dernière révision

18 Jan 2026