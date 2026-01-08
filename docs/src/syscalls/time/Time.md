# Appels Système Temporels

Cette section documente les appels système liés à la gestion du temps dans Nachos, permettant aux threads de contrôler leur exécution temporelle.

## Vue d'ensemble

Les appels système temporels permettent de :
- Suspendre l'exécution d'un thread pendant une durée déterminée
- Synchroniser des threads à des instants précis
- Obtenir l'instant courant du système
- Implémenter des timeouts et des tâches périodiques

## Concepts fondamentaux

### Ticks système

Nachos mesure le temps en **ticks système**, qui représentent les unités de temps atomiques. Un tick correspond généralement à une interruption d'horloge.

La variable globale `stats->totalTicks` contient le nombre total de ticks écoulés depuis le démarrage du système.

### Temps absolu vs relatif

- **Temps relatif** : Durée à partir de maintenant (ex: "dans 100 ticks")
- **Temps absolu** : Instant précis sur l'horloge système (ex: "au tick 1500")

## Liste des appels système

### [Sleep](./Sleep.md)

Endort le thread courant pendant une durée spécifiée (temps relatif).

**Utilisation** :
```c
int Sleep(int sleep_time);
```

**Cas d'usage** :
- Délais simples
- Pause entre opérations
- Ralentir l'exécution

**Exemple** :
```c
Sleep(100);  // Dort pendant 100 ticks
```

### [SleepUntil](./SleepUntil.md)

Endort le thread courant jusqu'à un instant absolu spécifié.

**Utilisation** :
```c
int SleepUntil(long long wake_time);
```

**Cas d'usage** :
- Tâches périodiques sans dérive
- Barrières temporelles
- Synchronisation précise

**Exemple** :
```c
long long wake_time;
GetCurrentTick(&wake_time);
wake_time += 100;
SleepUntil(wake_time);  // Dort jusqu'au tick (current + 100)
```

### [GetCurrentTick](./GetCurrentTick.md)

Obtient le nombre de ticks système écoulés depuis le démarrage.

**Utilisation** :
```c
int GetCurrentTick(long long *tick);
```

**Cas d'usage** :
- Mesurer des durées
- Calculer des instants futurs
- Horodatage d'événements

**Exemple** :
```c
long long current_tick;
GetCurrentTick(&current_tick);
```

## Comparaison Sleep vs SleepUntil

| Aspect | Sleep | SleepUntil |
|--------|-------|------------|
| **Type de temps** | Relatif (durée) | Absolu (instant) |
| **Paramètre** | `int` durée | `long long` instant |
| **Dérive cumulative** | Oui | Non |
| **Simplicité** | Plus simple | Nécessite calculs |
| **Précision** | Bonne | Excellente |
| **Usage typique** | Délais simples | Synchronisation précise |

### Exemple de dérive avec Sleep

```c
// Méthode 1 : Sleep répété (avec dérive)
for (i = 0; i < 10; i++) {
    do_work();
    Sleep(100);  // Dérive = temps de do_work() * 10
}

// Méthode 2 : SleepUntil (sans dérive)
long long next_wake;
GetCurrentTick(&next_wake);
for (i = 0; i < 10; i++) {
    do_work();
    next_wake += 100;
    SleepUntil(next_wake);  // Pas de dérive
}
```

## Patterns courants

### Pattern 1 : Tâche périodique simple

```c
void periodic_simple() {
    while (1) {
        do_task();
        Sleep(PERIOD);  // Acceptable si do_task() est rapide
    }
}
```

### Pattern 2 : Tâche périodique précise

```c
void periodic_precise() {
    long long next_wake;
    GetCurrentTick(&next_wake);

    while (1) {
        do_task();
        next_wake += PERIOD;
        SleepUntil(next_wake);  // Compense le temps de do_task()
    }
}
```

### Pattern 3 : Timeout

```c
int wait_with_timeout(int condition, int timeout_ticks) {
    long long deadline;
    GetCurrentTick(&deadline);
    deadline += timeout_ticks;

    while (!condition) {
        long long now;
        GetCurrentTick(&now);

        if (now >= deadline) {
            return -1;  // Timeout
        }

        Sleep(10);  // Petite pause avant revérification
    }

    return 0;  // Succès
}
```

### Pattern 4 : Barrière temporelle

```c
long long barrier_time;

void thread_with_barrier() {
    // Phase 1 : travail
    do_work();

    // Phase 2 : attente à la barrière
    SleepUntil(barrier_time);

    // Phase 3 : tous les threads commencent ici ensemble
    synchronized_work();
}
```

### Pattern 5 : Mesure de performance

```c
long long start_tick, end_tick;

GetCurrentTick(&start_tick);
expensive_operation();
GetCurrentTick(&end_tick);

long long elapsed = end_tick - start_tick;
PutString("Operation took ", 15);
PutInt((int)elapsed);
PutString(" ticks\n", 7);
```

## Précision et garanties

### Garanties

- Les threads sont réveillés **dès que possible** après l'expiration du délai
- L'ordre de réveil respecte l'ordre des `wake_time` pour des threads endormis
- Aucun thread n'est réveillé **avant** son instant de réveil

### Limitations

- **Granularité** : Limitée par la fréquence d'interruption de l'horloge
- **Latence** : Délai possible entre le réveil et l'exécution effective si CPU occupé
- **Précision** : Plus ou moins quelques ticks selon la charge système

## Thread-safety

Tous les appels système temporels sont thread-safe :
- Plusieurs threads peuvent dormir simultanément
- Les structures de données internes sont protégées

## Performance

### Coût des opérations

| Opération | Coût | Blocage |
|-----------|------|---------|
| `Sleep` / `SleepUntil` | Changement de contexte | Oui |
| `GetCurrentTick` | Lecture mémoire | Non |

### Optimisations

- La file d'attente des threads endormis est triée par `wake_time`
- Le réveil est géré par interruption d'horloge (pas de polling)
- Les threads endormis ne consomment pas de CPU

## Différences avec POSIX

| Fonction Nachos | Équivalent POSIX | Différences principales |
|-----------------|------------------|------------------------|
| `Sleep(int)` | `sleep(unsigned)` / `usleep(useconds_t)` | Unité : ticks vs secondes/µs |
| `SleepUntil(long long)` | `clock_nanosleep(..., TIMER_ABSTIME, ...)` | Plus simple, une seule horloge |
| `GetCurrentTick(long long*)` | `clock_gettime(...)` | Retourne directement les ticks |

## Voir aussi

- [Threads](../threads/Threads.md) - Gestion des threads
- [PthreadExit](../threads/PthreadExit.md) - Terminer un thread

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
