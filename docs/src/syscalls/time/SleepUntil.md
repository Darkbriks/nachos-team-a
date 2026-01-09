# SleepUntil

`SleepUntil` - Endort le thread courant jusqu'à un instant précis

## Synopsis

```c
#include "syscall.h"

int SleepUntil(long long wake_time);
```

## Description

`SleepUntil` met en pause l'exécution du thread courant jusqu'à ce qu'un instant précis (exprimé en ticks système absolus) soit atteint. Le thread est retiré de la file d'ordonnancement et sera réveillé automatiquement lorsque `stats->totalTicks >= wake_time`.

**Numéro d'appel système** : `SC_SleepUntil` (27)

### Comportement nominal

1. Validation que `wake_time >= 0`
2. Si `wake_time <= stats->totalTicks` (déjà passé), retour immédiat
3. Appel de `currentThread->SleepUntil(wake_time)` pour endormir le thread
4. Le thread est réveillé automatiquement quand `stats->totalTicks >= wake_time`

<div class="callout callout-note">
    <div class="callout-title">Temps absolu</div>
    <div class="callout-content">
        <code>SleepUntil</code> utilise un temps absolu (instant précis), tandis que <code>Sleep</code>
        utilise un temps relatif (durée). Utilisez <code>SleepUntil</code> pour des synchronisations
        temporelles précises sans dérive cumulative.
    </div>
</div>

## Paramètres

### `wake_time`

Instant absolu (en ticks système) auquel le thread doit se réveiller.

**Type** : `long long`
**Direction** : IN
**Registre** : `$4` (32 bits inférieurs seulement dans cette implémentation simplifiée)
**Contraintes** : Doit être >= 0

<div class="callout callout-warning">
    <div class="callout-title">Réveil immédiat</div>
    <div class="callout-content">
        Si <code>wake_time</code> est dans le passé ou égal au tick courant,
        <code>SleepUntil</code> retourne immédiatement sans endormir le thread.
    </div>
</div>

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès, le thread s'est réveillé au bon moment |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | `wake_time` est négatif |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:467`
- **Handler noyau** : `code/userprog/userSleep.cc:handle_SC_SleepUntil()`
- **Implémentation** : `code/userprog/userSleep.cc:do_UserSleepUntil()`
- **Mécanisme de réveil** : `code/threads/thread.cc:Thread::SleepUntil()`

### Flux d'exécution

```
SleepUntil(wake_time)
    │
    ▼
start.S: SleepUntil
    │ charge $4 = wake_time
    │ charge $2 = SC_SleepUntil (27)
    ▼
syscall SC_SleepUntil
    │
    ▼
handle_SC_SleepUntil()
    │ lit wake_time depuis $4
    ▼
do_UserSleepUntil(wake_time)
    │ ├─ valide wake_time >= 0
    │ ├─ désactive les interruptions
    │ ├─ si wake_time <= totalTicks, retourne 0
    │ └─ appelle currentThread->SleepUntil(wake_time)
    ▼
Thread::SleepUntil(wake_time)
    │ ├─ ajoute thread à la liste d'attente
    │ ├─ ordonne par wake_time croissant
    │ └─ appelle Sleep() pour suspendre le thread
    ▼
[Thread suspendu jusqu'à wake_time]
    │
    ▼
[Interruption horloge : stats->totalTicks >= wake_time]
    │
    ▼
[Thread réveillé et replacé dans la file d'ordonnancement]
    │
    ▼
[Retour à l'utilisateur avec succès]
```

## Exemples

### Exemple 1 : Réveil à un instant précis

```c
#include "syscall.h"

int main() {
    long long wake_time;

    if (GetCurrentTick(&wake_time) != 0) {
        PutString("Error getting current tick\n", 27);
        return 1;
    }

    wake_time += 200;  // Réveil dans 200 ticks

    PutString("Sleeping until tick ", 20);
    PutInt((int)wake_time);
    PutChar('\n');

    SleepUntil(wake_time);

    PutString("Woke up!\n", 9);
    return 0;
}
```

### Exemple 2 : Synchronisation périodique sans dérive

```c
#include "syscall.h"

void *periodic_task_precise(void *arg) {
    long long next_wake;
    int period = (int)(long)arg;
    int i;

    // Obtenir l'instant de départ
    if (GetCurrentTick(&next_wake) != 0) {
        return (void *)-1;
    }

    for (i = 0; i < 10; i++) {
        PutString("Tick ", 5);
        PutInt(i);
        PutChar('\n');

        // Calculer le prochain instant de réveil
        next_wake += period;

        // Dormir jusqu'à cet instant précis
        SleepUntil(next_wake);
    }

    return 0;
}

int main() {
    posix_thread_t tid;

    PutString("Starting precise periodic task (period=100)\n", 44);
    PthreadCreate(&tid, 0, periodic_task_precise, (void *)100);

    PthreadJoin(tid, 0);
    PutString("Task completed\n", 15);

    return 0;
}
```

**Avantage** : Pas de dérive cumulative, contrairement à une boucle utilisant `Sleep(period)`.

### Exemple 3 : Gestion du cas où wake_time est dans le passé

```c
#include "syscall.h"

int main() {
    long long current_tick;
    long long past_time;

    GetCurrentTick(&current_tick);
    past_time = current_tick - 100;  // 100 ticks dans le passé

    PutString("Current tick: ", 14);
    PutInt((int)current_tick);
    PutChar('\n');

    PutString("Trying to sleep until: ", 23);
    PutInt((int)past_time);
    PutChar('\n');

    // Retourne immédiatement
    if (SleepUntil(past_time) == 0) {
        PutString("SleepUntil returned immediately (expected)\n", 43);
    }

    return 0;
}
```

### Exemple 4 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    long long invalid_time = -100;

    if (SleepUntil(invalid_time) != 0) {
        int err = GetLastError();
        PutString("SleepUntil failed with errno=", 29);
        PutInt(err);  // Affichera 1 (E_INVAL)
        PutChar('\n');
        return 1;
    }

    PutString("This should not print\n", 22);
    return 0;
}
```

### Exemple 5 : Barrière temporelle pour threads

```c
#include "syscall.h"

long long barrier_time;

void *worker(void *arg) {
    int id = (int)(long)arg;

    PutString("Thread ", 7);
    PutInt(id);
    PutString(" working...\n", 12);

    // Simule un travail variable
    Sleep(id * 50);

    PutString("Thread ", 7);
    PutInt(id);
    PutString(" waiting at barrier\n", 20);

    // Tous les threads se réveillent au même instant
    SleepUntil(barrier_time);

    PutString("Thread ", 7);
    PutInt(id);
    PutString(" passed barrier!\n", 17);

    return 0;
}

int main() {
    posix_thread_t tid1, tid2, tid3;
    long long current;

    GetCurrentTick(&current);
    barrier_time = current + 500;  // Barrière dans 500 ticks

    PutString("Barrier time set to tick ", 25);
    PutInt((int)barrier_time);
    PutChar('\n');

    PthreadCreate(&tid1, 0, worker, (void *)1);
    PthreadCreate(&tid2, 0, worker, (void *)2);
    PthreadCreate(&tid3, 0, worker, (void *)3);

    PthreadJoin(tid1, 0);
    PthreadJoin(tid2, 0);
    PthreadJoin(tid3, 0);

    PutString("All threads completed\n", 22);
    return 0;
}
```

### Exemple 6 : Comparaison Sleep vs SleepUntil

```c
#include "syscall.h"

int main() {
    long long start, end, target;
    int i;

    // Méthode 1 : Sleep répété (avec dérive)
    GetCurrentTick(&start);
    for (i = 0; i < 5; i++) {
        Sleep(100);
    }
    GetCurrentTick(&end);
    PutString("Sleep x5: elapsed = ", 20);
    PutInt((int)(end - start));
    PutChar('\n');

    // Méthode 2 : SleepUntil (sans dérive)
    GetCurrentTick(&start);
    target = start;
    for (i = 0; i < 5; i++) {
        target += 100;
        SleepUntil(target);
    }
    GetCurrentTick(&end);
    PutString("SleepUntil x5: elapsed = ", 25);
    PutInt((int)(end - start));
    PutChar('\n');

    return 0;
}
```

**Résultat attendu** : `SleepUntil` est plus précis car il compense automatiquement les délais d'ordonnancement.

## Cas d'usage

### Synchronisation temporelle précise

`SleepUntil` est idéal pour :
- Tâches périodiques sans dérive cumulative
- Synchronisation de threads à un instant précis
- Barrières temporelles
- Ordonnancement temps réel

### Avantages par rapport à Sleep

| Aspect | Sleep | SleepUntil |
|--------|-------|------------|
| Dérive | Cumulative sur plusieurs itérations | Aucune |
| Utilisation | Délais simples | Synchronisation précise |
| Complexité | Plus simple | Nécessite gestion des ticks absolus |

## Thread-safety

`SleepUntil` est thread-safe. Plusieurs threads peuvent dormir jusqu'à des instants différents ou identiques sans conflit.

## Performance

- **Coût** : Très faible (changement de contexte)
- **Blocage** : Le thread est suspendu et ne consomme pas de CPU
- **Ordonnancement** : File d'attente ordonnée par `wake_time` pour un réveil efficace

## Différences avec POSIX

| Aspect | POSIX clock_nanosleep() | Nachos SleepUntil() |
|--------|------------------------|---------------------|
| Temps absolu | Supporte TIMER_ABSTIME | Par défaut |
| Unité | Nanosecondes | Ticks système |
| Horloges | Multiple (CLOCK_REALTIME, etc.) | Une seule horloge |
| Interruption | Peut être interrompu | Non interruptible |

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [Sleep](./Sleep.md) - Dormir pendant une durée relative
- [GetCurrentTick](./GetCurrentTick.md) - Obtenir le tick courant

</div>
</div>

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
