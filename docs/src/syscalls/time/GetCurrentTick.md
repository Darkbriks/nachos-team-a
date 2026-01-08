# GetCurrentTick

`GetCurrentTick` - Obtient le nombre de ticks système écoulés depuis le démarrage

## Synopsis

```c
#include "syscall.h"

int GetCurrentTick(long long *tick);
```

## Description

`GetCurrentTick` récupère le nombre total de ticks système écoulés depuis le démarrage de Nachos. Cette valeur est stockée dans `stats->totalTicks` et s'incrémente à chaque interruption d'horloge.

**Numéro d'appel système** : `SC_GetCurrentTick` (28)

### Comportement nominal

1. Validation de l'adresse `tick` (doit être >= 0)
2. Lecture de `stats->totalTicks` (64 bits)
3. Décomposition en deux mots de 32 bits (low et high)
4. Écriture des 32 bits inférieurs à l'adresse `tick`
5. Écriture des 32 bits supérieurs à l'adresse `tick + 4`

<div class="callout callout-note">
    <div class="callout-title">Format 64 bits</div>
    <div class="callout-content">
        Les ticks sont stockés sur 64 bits pour éviter un débordement rapide.
        L'architecture MIPS32 nécessite de décomposer cette valeur en deux mots de 32 bits.
    </div>
</div>

## Paramètres

### `tick`

Pointeur vers une variable `long long` où sera stocké le nombre de ticks courant.

**Type** : `long long *`
**Direction** : OUT
**Registre** : `$4`
**Contraintes** : Doit être une adresse valide en espace utilisateur avec au moins 8 octets disponibles

<div class="callout callout-warning">
    <div class="callout-title">Alignement mémoire</div>
    <div class="callout-content">
        L'adresse doit pointer vers une zone mémoire d'au moins 8 octets consécutifs.
        Les 4 premiers octets reçoivent les bits inférieurs, les 4 suivants les bits supérieurs.
    </div>
</div>

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès, la valeur a été écrite à l'adresse `tick` |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 2 | `E_FAULT` | Adresse invalide ou écriture mémoire échouée |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:482`
- **Handler noyau** : `code/userprog/userSleep.cc:handle_SC_GetCurrentTick()`

### Flux d'exécution

```
GetCurrentTick(&tick)
    │
    ▼
start.S: GetCurrentTick
    │ charge $4 = adresse de tick
    │ charge $2 = SC_GetCurrentTick (28)
    ▼
syscall SC_GetCurrentTick
    │
    ▼
handle_SC_GetCurrentTick()
    │ ├─ lit adresse depuis $4
    │ ├─ valide adresse >= 0
    │ ├─ lit stats->totalTicks (64 bits)
    │ ├─ décompose en high (bits 32-63) et low (bits 0-31)
    │ ├─ écrit low à l'adresse
    │ └─ écrit high à l'adresse + 4
    ▼
[Variable utilisateur contient le tick courant]
```

## Exemples

### Exemple 1 : Obtenir le tick courant

```c
#include "syscall.h"

int main() {
    long long current_tick;

    if (GetCurrentTick(&current_tick) != 0) {
        PutString("Error getting current tick\n", 27);
        return 1;
    }

    PutString("Current tick: ", 14);
    PutInt((int)current_tick);
    PutChar('\n');

    return 0;
}
```

### Exemple 2 : Mesurer une durée

```c
#include "syscall.h"

int main() {
    long long start_tick, end_tick, elapsed;
    int i, sum = 0;

    GetCurrentTick(&start_tick);

    // Opération à mesurer
    for (i = 0; i < 1000; i++) {
        sum += i;
    }

    GetCurrentTick(&end_tick);
    elapsed = end_tick - start_tick;

    PutString("Operation took ", 15);
    PutInt((int)elapsed);
    PutString(" ticks\n", 7);

    PutString("Result: ", 8);
    PutInt(sum);
    PutChar('\n');

    return 0;
}
```

### Exemple 3 : Calculer un instant futur

```c
#include "syscall.h"

int main() {
    long long current_tick, future_tick;

    GetCurrentTick(&current_tick);
    future_tick = current_tick + 500;

    PutString("Current: ", 9);
    PutInt((int)current_tick);
    PutChar('\n');

    PutString("Sleeping until: ", 16);
    PutInt((int)future_tick);
    PutChar('\n');

    SleepUntil(future_tick);

    PutString("Woke up!\n", 9);
    return 0;
}
```

### Exemple 4 : Horodatage d'événements

```c
#include "syscall.h"

void log_event(char *message, int len) {
    long long timestamp;
    GetCurrentTick(&timestamp);

    PutChar('[');
    PutInt((int)timestamp);
    PutString("] ", 2);
    PutString(message, len);
}

int main() {
    log_event("Program started\n", 16);

    Sleep(100);
    log_event("After first sleep\n", 18);

    Sleep(200);
    log_event("After second sleep\n", 19);

    log_event("Program ending\n", 15);
    return 0;
}
```

**Sortie attendue** :
```
[0] Program started
[100] After first sleep
[300] After second sleep
[300] Program ending
```

### Exemple 5 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    long long tick;
    long long *invalid_ptr = (long long *)-1;

    // Tentative avec adresse invalide
    if (GetCurrentTick(invalid_ptr) != 0) {
        int err = GetLastError();
        PutString("GetCurrentTick failed with errno=", 33);
        PutInt(err);  // Affichera 2 (E_FAULT)
        PutChar('\n');
    }

    // Tentative avec adresse valide
    if (GetCurrentTick(&tick) == 0) {
        PutString("Success! Tick = ", 16);
        PutInt((int)tick);
        PutChar('\n');
    }

    return 0;
}
```

### Exemple 6 : Tâche périodique sans dérive

```c
#include "syscall.h"

#define PERIOD 100
#define ITERATIONS 10

int main() {
    long long next_wake, start, end;
    int i;

    GetCurrentTick(&start);
    next_wake = start;

    for (i = 0; i < ITERATIONS; i++) {
        PutString("Iteration ", 10);
        PutInt(i);
        PutChar('\n');

        next_wake += PERIOD;
        SleepUntil(next_wake);
    }

    GetCurrentTick(&end);

    PutString("Total elapsed: ", 15);
    PutInt((int)(end - start));
    PutString(" ticks\n", 7);

    PutString("Expected: ", 10);
    PutInt(PERIOD * ITERATIONS);
    PutString(" ticks\n", 7);

    return 0;
}
```

**Résultat** : Le temps total est très proche de l'attendu (PERIOD × ITERATIONS), sans dérive.

### Exemple 7 : Comparaison de performances

```c
#include "syscall.h"

void benchmark(char *name, int name_len, void (*func)(void)) {
    long long start, end;

    GetCurrentTick(&start);
    func();
    GetCurrentTick(&end);

    PutString(name, name_len);
    PutString(": ", 2);
    PutInt((int)(end - start));
    PutString(" ticks\n", 7);
}

void fast_operation(void) {
    int i, sum = 0;
    for (i = 0; i < 100; i++) sum += i;
}

void slow_operation(void) {
    int i, j, sum = 0;
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j++)
            sum += i * j;
}

int main() {
    benchmark("Fast operation", 14, fast_operation);
    benchmark("Slow operation", 14, slow_operation);
    return 0;
}
```

## Cas d'usage

### Horodatage

`GetCurrentTick` est idéal pour :
- Enregistrer l'instant d'un événement
- Logger avec timestamp
- Créer des traces d'exécution

### Mesure de performance

- Mesurer la durée d'opérations
- Comparer différentes implémentations
- Profiler du code

### Calculs temporels

- Déterminer des instants futurs pour `SleepUntil`
- Implémenter des timeouts
- Planifier des tâches périodiques

## Thread-safety

`GetCurrentTick` est thread-safe. Plusieurs threads peuvent lire simultanément `stats->totalTicks` sans conflit, car il s'agit d'une opération en lecture seule.

<div class="callout callout-note">
    <div class="callout-title">Cohérence</div>
    <div class="callout-content">
        La valeur retournée est cohérente au moment de l'appel, mais peut
        être légèrement obsolète au moment où le thread l'utilise en raison
        de la préemption.
    </div>
</div>

## Performance

- **Coût** : Très faible (lecture mémoire + deux écritures)
- **Blocage** : Non-bloquant
- **Atomicité** : La lecture de `totalTicks` est atomique

## Précision

La valeur retournée a la même précision que les interruptions d'horloge système. Entre deux interruptions, `totalTicks` ne change pas.

## Différences avec POSIX

| Aspect | POSIX clock_gettime() | Nachos GetCurrentTick() |
|--------|----------------------|-------------------------|
| Horloges | Multiples (REALTIME, MONOTONIC, etc.) | Une seule |
| Unité | Secondes + nanosecondes | Ticks système |
| Structure | `struct timespec` | `long long` |
| Précision | Nanoseconde (théorique) | Tick système |
| Retour | 0 ou -1 + errno global | 0 ou -1 + errno via registre |

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [Sleep](./Sleep.md) - Dormir pendant une durée
- [SleepUntil](./SleepUntil.md) - Dormir jusqu'à un instant précis

</div>
</div>

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
