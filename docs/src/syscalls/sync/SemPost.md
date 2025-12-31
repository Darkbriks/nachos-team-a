# SemPost

`SemPost` - Opération V (signal/release) sur un sémaphore

## SYNOPSIS
```c
#include "syscall.h"

int SemPost(int sem_id);
```

## DESCRIPTION

`SemPost` effectue l'opération V (Verhogen) sur le sémaphore identifié par `sem_id`. Cette opération incrémente le compteur du sémaphore et réveille un thread bloqué en attente sur `SemWait()` si la queue n'est pas vide.

Numéro d'appel système : `26`

### Comportement nominal

- Vérifie la validité du handle `sem_id`
- Incrémente le compteur du sémaphore de manière atomique
- Si des threads sont bloqués en `SemWait()` : en réveille un
- Retourne immédiatement (opération non-bloquante)
- L'opération est atomique et thread-safe

### Cas particuliers

- **Handle invalide** : Retourne -1
- **Sémaphore détruit** : Retourne -1
- **Queue vide** : Compteur simplement incrémenté, pas de réveil
- **Overflow** : Compteur peut croître indéfiniment (pas de limite supérieure)

## PARAMÈTRES

### `sem_id`
Descripteur du sémaphore sur lequel effectuer l'opération V.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être un descripteur valide retourné par `SemInit()`
- Doit être dans l'intervalle [0, maxSemaphores-1] (max 511)
- Le sémaphore ne doit pas avoir été détruit

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : `0`

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante | Condition                            |
|-------|-----------|--------------------------------------|
| 10    | `E_NOENT` | Handle invalide ou sémaphore détruit |

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userSem.cc:handle_SC_SemPost()`
- **Implémentation** :
    - `code/userprog/addrspace.cc:AddrSpace::SemaphorePost()`
    - `code/threads/synch.cc:Semaphore::V()`

### Thread-safety

**Sémaphore interne** : Désactivation des interruptions assure atomicité :
- Pas de race condition entre V() concurrent
- Pas de corruption de la queue d'attente
- Thread réveillé de manière atomique

## DÉCISIONS DE CONCEPTION

*TODO*

## EXEMPLES

### Exemple 1 : Libération de mutex

```c
#include "syscall.h"

int mutex;

void critical_section() {
    SemWait(mutex);
    
    PutString("Dans section critique\n", 23);
    
    SemPost(mutex);  // Libérer le mutex
}

int main() {
    mutex = SemInit(1);
    
    critical_section();
    critical_section();
    
    SemDestroy(mutex);
    return 0;
}
```

### Exemple 2 : Signaler terminaison

```c
#include "syscall.h"

int done;

void worker_thread(void *arg) {
    PutString("Worker: calcul en cours...\n", 28);
    
    // Simuler travail intensif
    for (int i = 0; i < 1000000; i++) {
        // computation
    }
    
    PutString("Worker: terminé\n", 17);
    
    SemPost(done);  // Signaler terminaison
    ExitThread();
}

int main() {
    done = SemInit(0);
    
    int tid = CreateThread(worker_thread, 0);
    
    PutString("Main: attente worker...\n", 25);
    SemWait(done);  // Bloque jusqu'au signal
    PutString("Main: worker terminé!\n", 23);
    
    JoinThread(tid);
    SemDestroy(done);
    return 0;
}
```

### Exemple 3 : Barrière de synchronisation

```c
#include "syscall.h"

#define NUM_THREADS 5

int barrier;
int count = 0;
int mutex;

void barrier_thread(void *arg) {
    int id = (int)arg;
    
    PutString("Thread ", 7);
    PutInt(id);
    PutString(": phase 1\n", 11);
    
    // Arriver à la barrière
    SemWait(mutex);
    count++;
    if (count == NUM_THREADS) {
        // Dernier thread : réveiller tous les autres
        for (int i = 0; i < NUM_THREADS - 1; i++) {
            SemPost(barrier);
        }
    }
    SemPost(mutex);
    
    if (count < NUM_THREADS) {
        SemWait(barrier);  // Attendre les autres
    }
    
    PutString("Thread ", 7);
    PutInt(id);
    PutString(": phase 2\n", 11);
    
    ExitThread();
}

int main() {
    barrier = SemInit(0);
    mutex = SemInit(1);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        CreateThread(barrier_thread, (void*)i);
    }
    
    Sleep(500);
    
    SemDestroy(barrier);
    SemDestroy(mutex);
    return 0;
}
```

**Sortie** (ordre des threads peut varier) :
```
Thread 0: phase 1
Thread 1: phase 1
Thread 2: phase 1
Thread 3: phase 1
Thread 4: phase 1
Thread 0: phase 2
Thread 1: phase 2
Thread 2: phase 2
Thread 3: phase 2
Thread 4: phase 2
```

### Exemple 4 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    int sem = SemInit(0);
    
    // V() sur sémaphore valide
    if (SemPost(sem) == 0) {
        PutString("V réussi\n", 10);
    }
    
    // Détruire le sémaphore
    SemDestroy(sem);
    
    // Tenter V() sur sémaphore détruit
    if (SemPost(sem) < 0) {
        PutString("Erreur: sémaphore détruit\n", 27);
    }
    
    // Tenter V() sur handle invalide
    if (SemPost(999) < 0) {
        PutString("Erreur: handle invalide\n", 25);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
V réussi
Erreur: sémaphore détruit
Erreur: handle invalide
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Descripteur du sémaphore
- Compteur sémaphore : valeur ≥ 0
- Queue d'attente : 0+ threads bloqués

**Pendant l'appel (queue vide)** :
- Validation du handle
- Interruptions désactivées
- Compteur incrémenté
- Interruptions réactivées

**Pendant l'appel (queue non-vide)** :
- Validation du handle
- Interruptions désactivées
- Thread retiré de la queue
- Thread mis en état READY
- Compteur incrémenté
- Interruptions réactivées

**Après l'appel** :
- `$2` : 0 (succès) ou -1 (erreur)
- Thread appelant continue son exécution
- Thread réveillé (si applicable) : en état READY, sera schedulé plus tard

## NOTES

- **Toujours retourne immédiatement** : `SemPost()` ne bloque jamais
- **Pas de limite supérieure** : Compteur peut croître indéfiniment
- **Sémaphore détruit avec threads en attente** : Undefined behavior

## FAILLES ET VULNÉRABILITÉS

Aucune vulnérabilité connue à ce jour.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale avec adresses
- **v2.0** : Modification avec table de descripteurs

## VOIR AUSSI

- [SemInit](./SemInit.md) - Création d'un sémaphore
- [SemWait](SemWait.md) - Opération P (wait) sur un sémaphore
- [SemDestroy](./SemDestroy.md) - Destruction d'un sémaphore
- [SetMaxSemForProcess](./SetMaxSemForProcess.md) - Redimensionnement de la table
- [Vue d'ensemble](Sync.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 31 Dec 2025

## DERNIÈRE RÉVISION

31 Dec 2025
