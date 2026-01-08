# Halt

`Halt` - Arrête le système Nachos et affiche les statistiques de performance

## Synopsis

```c
#include "syscall.h"

void Halt(void);
```

## Description

`Halt` arrête complètement le système Nachos. Cette fonction termine immédiatement l'exécution de tous les processus et threads, puis affiche les statistiques de performance du système avant de quitter.

**Numéro d'appel système** : `SC_Halt` (0)

### Comportement nominal

1. Appel de `interrupt->Halt()` qui déclenche l'arrêt du système
2. Affichage des statistiques de performance (cycles CPU, interruptions, etc.)
3. Terminaison complète du système Nachos
4. Cette fonction ne retourne jamais

<div class="callout callout-warning">
    <div class="callout-title">Arrêt immédiat</div>
    <div class="callout-content">
        <code>Halt</code> termine le système sans nettoyer les ressources ni attendre
        la fin des autres threads ou processus. Utilisez <code>Exit</code> pour une
        terminaison propre d'un processus.
    </div>
</div>

## Paramètres

Cette fonction ne prend aucun paramètre.

## Valeur de retour

**Type** : `void`

Cette fonction ne retourne jamais. Elle termine l'exécution du système Nachos.

## Codes d'erreur

Aucun. Cette fonction ne peut pas échouer.

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:91`
- **Handler noyau** : `code/userprog/exception.cc:handle_SC_Halt()`
- **Implémentation** : `code/threads/interrupt.cc:Interrupt::Halt()`

### Flux d'exécution

```
Halt()
    │
    ▼
start.S: Halt
    │ charge $2 = SC_Halt (0)
    ▼
syscall SC_Halt
    │
    ▼
handle_SC_Halt()
    │ affiche "Shutdown, initiated by user program."
    ▼
interrupt->Halt()
    │ ├─ affiche les statistiques
    │ ├─ cleanup()
    │ └─ exit(0)
    ▼
[Fin du système Nachos]
```

## Exemples

### Exemple 1 : Arrêt simple

```c
#include "syscall.h"

int main() {
    PutString("System is shutting down...\n", 27);
    Halt();
    // Cette ligne ne sera jamais exécutée
    PutString("This will never print\n", 22);
    return 0;
}
```

### Exemple 2 : Arrêt après traitement

```c
#include "syscall.h"

int main() {
    int i;

    PutString("Processing data...\n", 19);

    for (i = 0; i < 10; i++) {
        PutInt(i);
        PutChar(' ');
    }
    PutChar('\n');

    PutString("Done! Halting system.\n", 22);
    Halt();

    return 0;  // Jamais atteint
}
```

### Exemple 3 : Halt vs Exit

```c
#include "syscall.h"

void *thread_func(void *arg) {
    int id = (int)(long)arg;
    PutString("Thread ", 7);
    PutInt(id);
    PutString(" running\n", 9);

    if (id == 0) {
        PutString("Thread 0 calling Halt()!\n", 25);
        Halt();  // Arrête TOUT le système
    }

    PutString("Thread ", 7);
    PutInt(id);
    PutString(" finished\n", 10);
    return 0;
}

int main() {
    posix_thread_t tid0, tid1;

    PthreadCreate(&tid0, 0, thread_func, (void *)0);
    PthreadCreate(&tid1, 0, thread_func, (void *)1);

    PthreadJoin(tid0, 0);
    PthreadJoin(tid1, 0);  // Jamais atteint si thread 0 appelle Halt

    PutString("Main exiting\n", 13);  // Jamais atteint
    return 0;
}
```

**Sortie attendue** :
```
Thread 0 running
Thread 1 running
Thread 0 calling Halt()!
Machine halting!

[Statistiques de performance affichées]
```

## Cas d'usage

### Arrêt de test

`Halt` est principalement utilisé pour :
- Terminer proprement les programmes de test
- Arrêter le système après une suite de tests
- Simuler un arrêt système complet

### Différence avec Exit

| Aspect | Halt | Exit |
|--------|------|------|
| Portée | Arrête tout le système | Termine le processus appelant |
| Threads | Tous terminés immédiatement | Attend la fin des threads du processus |
| Autres processus | Terminés | Continuent leur exécution |
| Nettoyage | Minimal | Complet pour le processus |
| Statistiques | Affichées | Non affichées |

## Thread-safety

Non applicable. `Halt` arrête immédiatement le système, quel que soit le thread appelant.

<div class="callout callout-warning">
    <div class="callout-title">Utilisation multi-thread</div>
    <div class="callout-content">
        Si plusieurs threads tentent d'appeler <code>Halt</code> simultanément,
        le premier appel arrêtera le système. Les autres threads seront terminés
        avant d'atteindre leur appel à <code>Halt</code>.
    </div>
</div>

## Statistiques affichées

Lors de l'arrêt, Nachos affiche typiquement :

```
Ticks: total X, idle Y, system Z, user W
Disk I/O: reads R, writes W
Console I/O: reads R, writes W
Paging: faults F
Network I/O: packets received R, sent S
```

Ces statistiques permettent d'évaluer les performances du système et des programmes utilisateur.

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [Exit](./Exit.md) - Terminer un processus proprement
- [PthreadExit](../threads/PthreadExit.md) - Terminer un thread

</div>
</div>

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
