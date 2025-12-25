# SemDestroy

`SemDestroy` - Détruit un sémaphore et libère ses ressources

## SYNOPSIS
```c
#include "syscall.h"

int SemDestroy(int sem_id);
```

## DESCRIPTION

`SemDestroy` détruit le sémaphore identifié par `sem_id`, libère ses ressources kernel et invalide le descripteur.

Numéro d'appel système : `27`

### Comportement nominal

- Vérifie la validité du handle `sem_id`
- Marque le descripteur comme invalide
- Libère l'objet `Semaphore` kernel (destructeur appelé)
- Libère le slot dans la table de descripteurs (BitMap)
- Le handle peut être réutilisé par un futur `SemInit()`

### Cas particuliers

- **Descripteur invalide** : Retourne -1
- **Double-free** : Retourne -1
- **Threads bloqués** : **UNDEFINED BEHAVIOR** - Ne pas détruire un sémaphore avec threads en attente
- **Nettoyage automatique** : Si non détruit explicitement, libération automatique à la terminaison du processus

## PARAMÈTRES

### `sem_id`
Descripteur du sémaphore à détruire.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être un handle valide retourné par `SemInit()`
- Doit être dans l'intervalle [0, maxSemaphores-1] (max 511)
- Le sémaphore ne doit pas déjà être détruit
- **IMPORTANT** : Aucun thread ne doit être bloqué sur ce sémaphore

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : `0`

**En cas d'erreur** : `-1` et `errno` est défini

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userSem.cc:handle_SC_SemDestroy()`
- **Implémentation** : `code/userprog/addrspace.cc:AddrSpace::SemaphoreDestroy()`

### Thread-safety

**Garanties** :
- Destruction atomique : pas de race condition entre threads
- Double-free impossible : flag `valid` protège
- Pas de corruption de la table

**Limitation** : Aucune vérification que des threads sont bloqués sur le sémaphore. Responsabilité de l'appelant.

## DÉCISIONS DE CONCEPTION

### Pourquoi nettoyage automatique ?

**Robustesse** : Évite les fuites mémoire si l'utilisateur oublie de détruire.

**Pattern RAII** : Ressource liée à la durée de vie du processus (`AddrSpace`).

**Coût** : Néant - le destructeur `~AddrSpace()` est appelé de toute façon.

## EXEMPLES

### Exemple 1 : Utilisation normale

```c
#include "syscall.h"

int main() {
    int sem = SemInit(1);
    if (sem < 0) {
        Exit(-1);
    }
    
    // Utiliser le sémaphore
    SemP(sem);
    PutString("Section critique\n", 18);
    SemV(sem);
    
    // Nettoyer
    if (SemDestroy(sem) == 0) {
        PutString("Sémaphore détruit\n", 19);
    }
    
    return 0;
}
```

### Exemple 2 : Détection de double-free

```c
#include "syscall.h"

int main() {
    int sem = SemInit(0);
    
    // Première destruction : OK
    if (SemDestroy(sem) == 0) {
        PutString("Première destruction OK\n", 25);
    }
    
    // Deuxième destruction : ERREUR
    if (SemDestroy(sem) < 0) {
        PutString("Erreur: déjà détruit\n", 22);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
Première destruction OK
Erreur: déjà détruit
```

### Exemple 3 : INCORRECT - Destruction avec threads bloqués

```c
#include "syscall.h"

int sem;

void blocked_thread(void *arg) {
    PutString("Thread: attente sémaphore...\n", 30);
    SemP(sem);  // Bloque ici indéfiniment
    PutString("Thread: réveillé\n", 18);  // Jamais atteint
    ExitThread();
}

int main() {
    sem = SemInit(0);
    
    int tid = CreateThread(blocked_thread, 0);
    Sleep(50);  // Laisser le thread bloquer
    
    // ⚠️ DANGER : Détruire le sémaphore avec thread bloqué
    SemDestroy(sem);
    
    // Comportement indéfini : le thread bloqué peut crasher
    
    JoinThread(tid);  // Peut ne jamais retourner
    return 0;
}
```

**Comportement** : UNDEFINED - Le thread bloqué peut :
- Crasher (segfault en accédant au sémaphore libéré)
- Rester bloqué indéfiniment
- Corrompre la mémoire

**Solution** : Toujours signaler les threads bloqués avant destruction :
```c
SemV(sem);  // Réveiller le thread
JoinThread(tid);  // Attendre terminaison
SemDestroy(sem);  // Maintenant sûr
```

### Exemple 4 : Nettoyage automatique

```c
#include "syscall.h"

int main() {
    int sem = SemInit(1);
    
    SemP(sem);
    PutString("Travail...\n", 12);
    SemV(sem);
    
    // Terminer SANS détruire explicitement
    // → Nettoyage automatique dans ~AddrSpace()
    
    return 0;  // Pas de fuite mémoire
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Handle du sémaphore
- `semaphoreTable[handle].valid` : true
- `semaphoreBitmap->Test(handle)` : true
- Objet `Semaphore` existe en mémoire kernel

**Pendant l'appel** :
- Validations effectuées
- `valid` flag mis à false
- Destructeur `~Semaphore()` appelé
- BitMap mis à jour : slot marqué libre

**Après l'appel** :
- `$2` : 0 (succès) ou -1 (erreur)
- `semaphoreTable[handle].valid` : false
- `semaphoreTable[handle].semaphore` : nullptr
- `semaphoreBitmap->Test(handle)` : false
- Handle réutilisable par `SemInit()`

## NOTES

- **Ordre de destruction** : Pas d'ordre imposé, mais recommandé de détruire en ordre inverse de création (style LIFO/RAII)
- **Nettoyage automatique** : Même si non détruit, pas de fuite à la terminaison du processus
- **Handle réutilisable** : Après destruction, le handle peut être réalloué immédiatement
- **Pas de vérification threads bloqués** : Responsabilité de l'appelant

## FAILLES ET VULNÉRABILITÉS

Aucune vulnérabilité connue à ce jour.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale avec adresses
- **v2.0** : Modification avec table de descripteurs

## VOIR AUSSI

- [SemInit](./SemInit.md) - Création d'un sémaphore
- [SemP](./SemP.md) - Opération P (wait) sur un sémaphore
- [SemV](./SemV.md) - Opération V (signal) sur un sémaphore
- [SetMaxSemForProcess](./SetMaxSemForProcess.md) - Redimensionnement de la table
- [Vue d'ensemble](./README.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 25 Dec 2025

## DERNIÈRE RÉVISION

25 Dec 2025 par Antoine
