# SemP

`SemP` - Opération P (wait/acquire) sur un sémaphore

## SYNOPSIS
```c
#include "syscall.h"

int SemWait(int sem_id);
```

## DESCRIPTION

`SemP` effectue l'opération P (Proberen) sur le sémaphore identifié par `sem_id`. Cette opération décrémente le compteur du sémaphore et bloque le thread appelant si le compteur devient négatif.

Numéro d'appel système : `25`

### Comportement nominal

- Vérifie la validité du déscripteur `sem_id`
- Si compteur > 0 : décrémente et retourne immédiatement
- Si compteur = 0 : bloque le thread en attente d'un `SemPost()`
- L'opération est atomique et thread-safe

### Cas particuliers

- **Handle invalide** : Retourne -1
- **Sémaphore détruit** : Retourne -1
- **Blocage indéfini** : Si aucun `SemPost()` n'est appelé, le thread reste bloqué (pas de timeout)
- **Interruptions** : Le blocage n'est pas interruptible

## PARAMÈTRES

### `sem_id`
Déscripteur du sémaphore sur lequel effectuer l'opération P.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être un déscripteur valide retourné par `SemInit()`
- Doit être dans l'intervalle [0, 15]
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
- **Handler noyau** : `code/userprog/exception.cc:handle_SC_SemWait()`
- **Implémentation** : 
  - `code/userprog/addrspace.cc:AddrSpace::SemaphoreP()`
  - `code/threads/synch.cc:Semaphore::P()`

### Thread-safety

**Sémaphore interne** : `Semaphore::P()` utilise son propre lock interne :
- Lock sémaphore acquis
- Si `value > 0` : décrémente et retourne
- Si `value == 0` : thread ajouté à la queue FIFO, bloqué sur condition variable

## DÉCISIONS DE CONCEPTION

## EXEMPLES

### Exemple 1 : Mutex basique

```c
#include "syscall.h"

int mutex;
int shared_counter = 0;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        // Entrer en section critique
        if (SemWait(mutex) < 0) {
            PutString("Erreur SemP\n", 13);
            ExitThread();
        }
        
        shared_counter++;
        
        // Sortir de section critique
        SemPost(mutex);
    }
    ExitThread();
}

int main() {
    mutex = SemInit(1);
    
    int tid1 = CreateThread(increment_thread, 0);
    int tid2 = CreateThread(increment_thread, 0);
    
    JoinThread(tid1);
    JoinThread(tid2);
    
    PutString("Counter: ", 9);
    PutInt(shared_counter);  // Devrait être 2000
    PutChar('\n');
    
    SemDestroy(mutex);
    return 0;
}
```

### Exemple 2 : Rendez-vous entre threads

```c
#include "syscall.h"

int rdv1, rdv2;

void thread_A(void *arg) {
    PutString("A: Phase 1\n", 12);
    Sleep(50);
    
    SemPost(rdv1);  // Signaler à B
    SemWait(rdv2);  // Attendre B
    
    PutString("A: Phase 2\n", 12);
    ExitThread();
}

void thread_B(void *arg) {
    PutString("B: Phase 1\n", 12);
    Sleep(100);
    
    SemPost(rdv2);  // Signaler à A
    SemWait(rdv1);  // Attendre A
    
    PutString("B: Phase 2\n", 12);
    ExitThread();
}

int main() {
    rdv1 = SemInit(0);
    rdv2 = SemInit(0);
    
    int ta = CreateThread(thread_A, 0);
    int tb = CreateThread(thread_B, 0);
    
    JoinThread(ta);
    JoinThread(tb);
    
    SemDestroy(rdv1);
    SemDestroy(rdv2);
    return 0;
}
```

**Sortie garantie** :
```
A: Phase 1
B: Phase 1
A: Phase 2   (ou B: Phase 2)
B: Phase 2   (ou A: Phase 2)
```

### Exemple 3 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    int sem = SemInit(1);
    
    // Utilisation normale
    if (SemWait(sem) == 0) {
        PutString("P réussi\n", 10);
        SemPost(sem);
    }
    
    // Détruire le sémaphore
    SemDestroy(sem);
    
    // Tenter d'utiliser un handle détruit
    if (SemWait(sem) < 0) {
        PutString("Erreur: sémaphore détruit\n", 27);
    }
    
    // Tenter d'utiliser un handle invalide
    if (SemWait(999) < 0) {
        PutString("Erreur: handle invalide\n", 25);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
P réussi
Erreur: sémaphore détruit
Erreur: handle invalide
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Handle du sémaphore
- Thread : état RUNNING
- Compteur sémaphore : valeur quelconque ≥ 0

**Pendant l'appel (cas non-bloquant)** :
- Validation du déscripteur
- Lock sémaphore acquis
- Compteur décrémenté
- Lock sémaphore libéré

**Pendant l'appel (cas bloquant)** :
- Validation du déscripteur
- Lock sémaphore acquis
- Thread ajouté à la queue d'attente
- Thread mis en état BLOCKED
- Lock sémaphore libéré
- Thread reste bloqué jusqu'à `SemPost()`

**Après l'appel** :
- `$2` : 0 (succès) ou -1 (erreur)
- `errno` : 0 ou E_NOENT

## NOTES

- **Atomicité** : P() est atomique, pas de race condition possible

## FAILLES ET VULNÉRABILITÉS
Aucune vulnérabilité connue a ce jour.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale avec adresses
- **v2.0** : Modification avec table de descripteurs

## VOIR AUSSI

- [SemInit](./SemInit.md) - Création d'un sémaphore
- [SemV](SemPost.md) - Opération V (signal) sur un sémaphore
- [SemDestroy](./SemDestroy.md) - Destruction d'un sémaphore
- [Vue d'ensemble](./README.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 21 Dec 2025

## DERNIÈRE RÉVISION

21 Dec 2025 par Antoine
