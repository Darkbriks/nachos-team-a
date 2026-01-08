# Sleep

`Sleep` - Endort le thread courant pendant un nombre de ticks

## Synopsis

```c
#include "syscall.h"

int Sleep(int sleep_time);
```

## Description

`Sleep` met en pause l'exécution du thread courant pendant un nombre spécifié de ticks système. Le thread est retiré de la file d'ordonnancement et sera réveillé automatiquement lorsque le nombre de ticks spécifié se sera écoulé.

**Numéro d'appel système** : `SC_Sleep` (26)

### Comportement nominal

1. Validation que `sleep_time >= 0`
2. Si `sleep_time == 0`, retour immédiat (no-op)
3. Calcul du temps de réveil : `wakeTime = stats->totalTicks + sleep_time`
4. Vérification de débordement arithmétique
5. Appel de `currentThread->SleepUntil(wakeTime)` pour endormir le thread
6. Le thread est réveillé automatiquement quand `stats->totalTicks >= wakeTime`

<div class="callout callout-note">
    <div class="callout-title">Temps relatif</div>
    <div class="callout-content">
        <code>Sleep</code> utilise un temps relatif (durée), tandis que <code>SleepUntil</code>
        utilise un temps absolu (instant précis). Utilisez <code>Sleep</code> pour des délais simples.
    </div>
</div>

## Paramètres

### `sleep_time`

Nombre de ticks système pendant lesquels le thread doit dormir.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** : Doit être >= 0

<div class="callout callout-note">
    <div class="callout-title">Ticks système</div>
    <div class="callout-content">
        Un tick système correspond à une unité de temps de base dans Nachos.
        La durée exacte dépend de la configuration, mais représente généralement
        l'intervalle entre deux interruptions de l'horloge.
    </div>
</div>

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès, le thread s'est réveillé normalement |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | `sleep_time` est négatif |
| 3 | `E_OVERFLOW` | Débordement arithmétique lors du calcul du temps de réveil |

<div class="callout callout-warning">
    <div class="callout-title">Débordement</div>
    <div class="callout-content">
        Si <code>currentTick + sleep_time</code> dépasse la capacité d'un <code>long long</code>,
        l'erreur <code>E_OVERFLOW</code> est retournée et le thread n'est pas endormi.
    </div>
</div>

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:448`
- **Handler noyau** : `code/userprog/userSleep.cc:handle_SC_Sleep()`
- **Implémentation** : `code/userprog/userSleep.cc:do_UserSleep()`
- **Mécanisme de réveil** : `code/threads/thread.cc:Thread::SleepUntil()`

### Flux d'exécution

```
Sleep(sleep_time)
    │
    ▼
start.S: Sleep
    │ charge $4 = sleep_time
    │ charge $2 = SC_Sleep (26)
    ▼
syscall SC_Sleep
    │
    ▼
handle_SC_Sleep()
    │ lit sleep_time depuis $4
    ▼
do_UserSleep(sleep_time)
    │ ├─ valide sleep_time >= 0
    │ ├─ si sleep_time == 0, retourne 0
    │ ├─ désactive les interruptions
    │ ├─ calcule wakeTime = totalTicks + sleep_time
    │ ├─ vérifie débordement
    │ └─ appelle currentThread->SleepUntil(wakeTime)
    ▼
Thread::SleepUntil(wakeTime)
    │ ├─ ajoute thread à la liste d'attente
    │ ├─ ordonne par wakeTime croissant
    │ └─ appelle Sleep() pour suspendre le thread
    ▼
[Thread suspendu jusqu'à wakeTime]
    │
    ▼
[Interruption horloge : stats->totalTicks >= wakeTime]
    │
    ▼
[Thread réveillé et replacé dans la file d'ordonnancement]
    │
    ▼
[Retour à l'utilisateur avec succès]
```

## Exemples

### Exemple 1 : Délai simple

```c
#include "syscall.h"

int main() {
    PutString("Starting task...\n", 17);

    Sleep(100);  // Dort pendant 100 ticks

    PutString("Task resumed after 100 ticks\n", 29);
    return 0;
}
```

### Exemple 2 : Boucle avec délai

```c
#include "syscall.h"

int main() {
    int i;

    for (i = 0; i < 5; i++) {
        PutString("Iteration ", 10);
        PutInt(i);
        PutChar('\n');

        Sleep(50);  // Pause entre les itérations
    }

    PutString("Done!\n", 6);
    return 0;
}
```

### Exemple 3 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    int sleep_time = -10;

    if (Sleep(sleep_time) != 0) {
        int err = GetLastError();
        PutString("Sleep failed with errno=", 24);
        PutInt(err);  // Affichera 1 (E_INVAL)
        PutChar('\n');
        return 1;
    }

    PutString("Sleep succeeded\n", 16);
    return 0;
}
```

### Exemple 4 : Sleep(0) comme yield

```c
#include "syscall.h"

void *worker(void *arg) {
    int id = (int)(long)arg;
    int i;

    for (i = 0; i < 10; i++) {
        PutString("Thread ", 7);
        PutInt(id);
        PutString(" working\n", 9);

        Sleep(0);  // Permet aux autres threads de s'exécuter
    }

    return 0;
}

int main() {
    posix_thread_t tid1, tid2;

    PthreadCreate(&tid1, 0, worker, (void *)1);
    PthreadCreate(&tid2, 0, worker, (void *)2);

    PthreadJoin(tid1, 0);
    PthreadJoin(tid2, 0);

    return 0;
}
```

### Exemple 5 : Synchronisation temporelle

```c
#include "syscall.h"

void *periodic_task(void *arg) {
    int period = (int)(long)arg;
    int i;

    for (i = 0; i < 5; i++) {
        PutString("Periodic task executing\n", 24);
        Sleep(period);
    }

    return 0;
}

int main() {
    posix_thread_t tid;

    PutString("Starting periodic task (period=100 ticks)\n", 42);
    PthreadCreate(&tid, 0, periodic_task, (void *)100);

    PthreadJoin(tid, 0);
    PutString("Periodic task completed\n", 24);

    return 0;
}
```

### Exemple 6 : Timeout avec vérification

```c
#include "syscall.h"

int wait_with_timeout(int timeout_ticks) {
    long long start_tick, end_tick;

    if (GetCurrentTick(&start_tick) != 0) {
        return -1;
    }

    Sleep(timeout_ticks);

    if (GetCurrentTick(&end_tick) != 0) {
        return -1;
    }

    // Vérifie que le délai s'est bien écoulé
    long long elapsed = end_tick - start_tick;
    PutString("Elapsed ticks: ", 15);
    PutInt((int)elapsed);
    PutChar('\n');

    return 0;
}

int main() {
    wait_with_timeout(200);
    return 0;
}
```

## Cas d'usage

### Délais et temporisation

`Sleep` est utilisé pour :
- Créer des délais entre opérations
- Implémenter des timeouts
- Ralentir l'exécution pour des démonstrations
- Simuler des opérations longues

### Ordonnancement coopératif

`Sleep(0)` peut être utilisé comme point de coopération pour permettre à d'autres threads de s'exécuter sans attente réelle.

## Thread-safety

`Sleep` est thread-safe. Chaque thread peut appeler `Sleep` indépendamment, et le système gère correctement le réveil de plusieurs threads simultanément.

<div class="callout callout-note">
    <div class="callout-title">Précision temporelle</div>
    <div class="callout-content">
        La précision de <code>Sleep</code> dépend de la fréquence des interruptions d'horloge.
        Le thread sera réveillé dès que possible après l'expiration du délai, mais peut
        y avoir un léger retard si d'autres threads sont en cours d'exécution.
    </div>
</div>

## Performance

- **Coût** : Très faible (changement de contexte)
- **Blocage** : Le thread est suspendu et ne consomme pas de CPU
- **Ordonnancement** : Les autres threads peuvent s'exécuter pendant le sommeil

## Différences avec POSIX

| Aspect | POSIX sleep() / usleep() | Nachos Sleep() |
|--------|-------------------------|----------------|
| Unité | Secondes / microsecondes | Ticks système |
| Type paramètre | `unsigned int` / `useconds_t` | `int` |
| Interruption | Peut être interrompu par signal | Non interruptible |
| Valeur de retour | Temps restant si interrompu | 0 ou -1 |
| Précision | Haute résolution | Dépend de l'horloge système |

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [SleepUntil](./SleepUntil.md) - Dormir jusqu'à un instant précis
- [GetCurrentTick](./GetCurrentTick.md) - Obtenir le tick courant

</div>
</div>

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
