# PthreadSelf

`PthreadSelf` - Obtient l'identifiant du thread courant

## Synopsis

```c
#include "syscall.h"

posix_thread_t PthreadSelf(void);
```

## Description

`PthreadSelf` retourne l'identifiant unique (TID) du thread appelant. Chaque thread dans un processus possède un TID unique attribué lors de sa création.

**Numéro d'appel système** : `SC_PthreadSelf` (21)

### Comportement nominal

1. Récupération du pointeur vers le thread courant (`currentThread`)
2. Extraction du TID via `currentThread->getTID()`
3. Retour immédiat du TID dans le registre `$2`

<div class="callout callout-note">
    <div class="callout-title">Thread principal</div>
    <div class="callout-content">
        Le thread principal d'un processus possède toujours le TID 0.
    </div>
</div>

## Paramètres

Cette fonction ne prend aucun paramètre.

## Valeur de retour

**Type** : `posix_thread_t` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `>= 0` | TID du thread courant (succès) |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 9 | `E_NOSPC` | Pointeur de thread courant invalide (ne devrait jamais arriver) |

<div class="callout callout-note">
    <div class="callout-title">Fiabilité</div>
    <div class="callout-content">
        En pratique, <code>PthreadSelf</code> ne devrait jamais échouer dans un système correctement configuré,
        car <code>currentThread</code> est toujours valide pendant l'exécution d'un thread.
    </div>
</div>

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:351`
- **Handler noyau** : `code/userprog/userthread.cc:handle_SC_PthreadSelf()`
- **Implémentation** : `code/userprog/userthread.cc:do_PthreadSelf()`

### Flux d'exécution

```
PthreadSelf()
    │
    ▼
start.S: PthreadSelf
    │ charge $2 = SC_PthreadSelf
    ▼
syscall SC_PthreadSelf
    │
    ▼
handle_SC_PthreadSelf()
    │
    ▼
do_PthreadSelf()
    │ ├─ récupère currentThread
    │ ├─ vérifie validité (protection)
    │ └─ retourne currentThread->getTID()
    ▼
[registre $2 contient le TID]
```

## Exemples

### Exemple 1 : Affichage du TID

```c
#include "syscall.h"

void *worker(void *arg) {
    posix_thread_t my_tid = PthreadSelf();

    PutString("Thread TID=", 11);
    PutInt(my_tid);
    PutString(" is running\n", 12);

    return 0;
}

int main() {
    posix_thread_t tid1, tid2;
    posix_thread_t main_tid = PthreadSelf();

    PutString("Main thread TID=", 16);
    PutInt(main_tid);
    PutChar('\n');

    PthreadCreate(&tid1, 0, worker, 0);
    PthreadCreate(&tid2, 0, worker, 0);

    PthreadJoin(tid1, 0);
    PthreadJoin(tid2, 0);

    return 0;
}
```

**Sortie attendue** :
```
Main thread TID=0
Thread TID=1 is running
Thread TID=2 is running
```

### Exemple 2 : Comparaison de TID

```c
#include "syscall.h"

posix_thread_t global_tid;

void *compare_thread(void *arg) {
    posix_thread_t my_tid = PthreadSelf();

    if (my_tid == global_tid) {
        PutString("TID matches!\n", 13);
    } else {
        PutString("TID mismatch!\n", 14);
    }

    return 0;
}

int main() {
    PthreadCreate(&global_tid, 0, compare_thread, 0);
    PthreadJoin(global_tid, 0);
    return 0;
}
```

### Exemple 3 : Thread-local storage simulé

```c
#include "syscall.h"

#define MAX_THREADS 10

int thread_data[MAX_THREADS];

void set_thread_data(int value) {
    posix_thread_t tid = PthreadSelf();
    if (tid >= 0 && tid < MAX_THREADS) {
        thread_data[tid] = value;
    }
}

int get_thread_data(void) {
    posix_thread_t tid = PthreadSelf();
    if (tid >= 0 && tid < MAX_THREADS) {
        return thread_data[tid];
    }
    return -1;
}

void *worker(void *arg) {
    int value = (int)(long)arg;
    set_thread_data(value);

    PutString("Thread ", 7);
    PutInt(PthreadSelf());
    PutString(" stored value: ", 15);
    PutInt(get_thread_data());
    PutChar('\n');

    return 0;
}

int main() {
    posix_thread_t tid1, tid2;

    PthreadCreate(&tid1, 0, worker, (void *)42);
    PthreadCreate(&tid2, 0, worker, (void *)99);

    PthreadJoin(tid1, 0);
    PthreadJoin(tid2, 0);

    return 0;
}
```

**Sortie attendue** :
```
Thread 1 stored value: 42
Thread 2 stored value: 99
```

### Exemple 4 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    posix_thread_t tid = PthreadSelf();

    if (tid < 0) {
        int err = GetLastError();
        PutString("PthreadSelf failed with errno=", 30);
        PutInt(err);
        PutChar('\n');
        return 1;
    }

    PutString("Current TID: ", 13);
    PutInt(tid);
    PutChar('\n');

    return 0;
}
```

## Cas d'usage

### Identification de thread

`PthreadSelf` est particulièrement utile pour :
- Distinguer le thread principal des threads secondaires
- Implémenter un système de logging par thread
- Créer des structures de données thread-local
- Déboguer des problèmes de concurrence

### Comparaison avec pthread_equal

Contrairement à POSIX qui fournit `pthread_equal()` pour comparer des TID, Nachos utilise des entiers simples, donc la comparaison directe avec `==` est possible.

## Thread-safety

L'appel est thread-safe. Chaque thread obtient son propre TID unique, et `currentThread` est toujours valide dans le contexte d'un thread en cours d'exécution.

<div class="callout callout-note">
    <div class="callout-title">Macro RETURN</div>
    <div class="callout-content">
        L'implémentation utilise la macro <code>RETURN()</code> qui écrit directement
        dans le registre <code>$2</code> et gère la convention d'appel système.
    </div>
</div>


## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Créer un nouveau thread
- [PthreadExit](./PthreadExit.md) - Terminer un thread
- [PthreadJoin](./PthreadJoin.md) - Attendre un thread
- [PthreadDetach](./PthreadDetach.md) - Détacher un thread

</div>
</div>

## Auteurs

Alioune Badara DIENE, 7 Jan 2026

## Dernière révision
7 Jan 2026
