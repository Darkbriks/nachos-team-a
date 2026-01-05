# SemInit

`SemInit` - Crée et initialise un nouveau sémaphore

## SYNOPSIS
```c
#include "syscall.h"

int SemInit(int value);
```

## DESCRIPTION

`SemInit` crée un nouveau sémaphore avec une valeur initiale spécifiée et retourne un descripteur permettant de l'identifier. Le sémaphore peut ensuite être utilisé pour la synchronisation entre threads du même processus.

Numéro d'appel système : `29`

### Comportement nominal

- Alloue un nouveau descripteur dans la table de sémaphores du processus
- Initialise le compteur interne du sémaphore à `value`
- Retourne un handle unique (entier entre 0 et `maxSemaphores - 1`)
- Le sémaphore est partagé entre tous les threads du processus
- Le sémaphore persiste jusqu'à destruction explicite ou terminaison du processus

### Auto-expansion de la table

Si la table de sémaphores est pleine lors d'un appel à `SemInit()`, le système tente automatiquement de l'agrandir (doublement de la taille). Cette expansion est transparente pour l'utilisateur.

L'expansion échoue uniquement si la nouvelle taille dépasserait `MAX_SEMAPHORES_PER_PROCESS` (512).

### Cas particuliers

- **value < 0** : Retourne -1, `errno = E_INVAL`
- **Table pleine et expansion impossible** : Retourne -1, `errno = E_FTABLE` (maximum 512 sémaphores atteint)

## PARAMÈTRES

### `value`
Valeur initiale du compteur du sémaphore.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être ≥ 0
- Représente le nombre de threads pouvant exécuter `SemWait()` simultanément sans bloquer

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : Handle du sémaphore (0 à `maxSemaphores - 1`)

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante   | Condition                                             |
|-------|-------------|-------------------------------------------------------|
| 1     | `E_INVAL`   | `value < 0`                                           |
| 10    | `E_FTABLE`  | Table pleine (512 sémaphores) et expansion impossible |

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userSem.cc:handle_SC_SemInit()`
- **Implémentation** : `code/userprog/addrspace.cc:AddrSpace::SemaphoreCreate()`

### Architecture interne

**Table de descripteurs** :
- Taille initiale : 16 entrées (`INITIAL_SEMAPHORE_TABLE_SIZE`)
- Taille maximale : 512 entrées (`MAX_SEMAPHORES_PER_PROCESS`)
- Auto-expansion : doublement automatique si table pleine
- BitMap pour tracking des slots utilisés/libres
- Chaque descripteur contient :
  - `Semaphore* semaphore` : pointeur vers objet kernel
  - `bool valid` : flag de validité

**Allocation** :
1. Vérification de `value ≥ 0`
2. Recherche d'un slot libre via `BitMap::Find()` → O(1)
3. Si aucun slot libre : tentative d'expansion (×2)
4. Création de `Semaphore("UserSemaphore", value)`
5. Enregistrement dans la table
6. Marquage du slot comme utilisé

### Thread-safety

**Garanties** :
- Plusieurs threads peuvent appeler `SemInit` simultanément
- Les descripteurs sont alloués de manière atomique
- Pas de race condition sur le BitMap

## DÉCISIONS DE CONCEPTION

### Pourquoi des descripteurs plutôt que des pointeurs ?

**Sécurité** : Empêche l'utilisateur de manipuler directement la mémoire kernel. Sans descripteurs, un utilisateur malveillant pourrait :
- Forger un pointeur vers n'importe quelle structure kernel
- Provoquer des crashs ou corruption mémoire
- Contourner les mécanismes de protection

### Pourquoi l'auto-expansion ?

*TODO*

## EXEMPLES

### Exemple 1 : Créer un mutex

```c
#include "syscall.h"

int main() {
    // Créer un sémaphore binaire (mutex)
    int mutex = SemInit(1);
    
    if (mutex < 0) {
        PutString("Erreur création mutex\n", 23);
        Exit(-1);
    }
    
    PutString("Mutex créé avec handle: ", 25);
    PutInt(mutex);
    PutChar('\n');
    
    // Utiliser le mutex...
    
    SemDestroy(mutex);
    return 0;
}
```

**Sortie attendue** :
```
Mutex créé avec handle: 0
```

### Exemple 2 : Sémaphore pour rendez-vous

```c
#include "syscall.h"

int rdv;

void worker_thread(void *arg) {
    PutString("Thread: travail en cours...\n", 29);
    Sleep(100);  // Simuler du travail
    PutString("Thread: travail terminé\n", 25);
    
    SemPost(rdv);  // Signaler terminaison
    ExitThread();
}

int main() {
    // Sémaphore initialement bloqué
    rdv = SemInit(0);
    if (rdv < 0) {
        PutString("Erreur création sémaphore\n", 27);
        Exit(-1);
    }
    
    int tid = CreateThread(worker_thread, 0);
    
    PutString("Main: attente du worker...\n", 28);
    SemWait(rdv);  // Bloquer jusqu'au signal
    PutString("Main: worker terminé!\n", 23);
    
    JoinThread(tid);
    SemDestroy(rdv);
    return 0;
}
```

### Exemple 3 : Auto-expansion de la table

```c
#include "syscall.h"

int main() {
    int sems[100];
    int count = 0;
    
    // Créer 100 sémaphores (dépasse la taille initiale de 16)
    for (int i = 0; i < 100; i++) {
        sems[i] = SemInit(0);
        
        if (sems[i] < 0) {
            PutString("Échec après ", 13);
            PutInt(count);
            PutString(" sémaphores\n", 13);
            break;
        }
        
        count++;
    }
    
    PutString("Créé ", 6);
    PutInt(count);
    PutString(" sémaphores avec succès\n", 25);
    
    // Nettoyer
    for (int i = 0; i < count; i++) {
        SemDestroy(sems[i]);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
Créé 100 sémaphores avec succès
```

### Exemple 4 : Limite maximale atteinte

```c
#include "syscall.h"

int main() {
    int sems[600];
    int count = 0;
    
    // Tenter de créer 600 sémaphores (> MAX_SEMAPHORES_PER_PROCESS)
    for (int i = 0; i < 600; i++) {
        sems[i] = SemInit(0);
        
        if (sems[i] < 0) {
            PutString("Limite atteinte après ", 23);
            PutInt(count);
            PutString(" sémaphores\n", 13);
            break;
        }
        
        count++;
    }
    
    // Nettoyer
    for (int i = 0; i < count; i++) {
        SemDestroy(sems[i]);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
Limite atteinte après 512 sémaphores
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Valeur initiale du sémaphore
- Table de sémaphores : peut contenir 0 à `maxSemaphores` descripteurs actifs
- BitMap : indique slots libres/occupés

**Pendant l'appel** :
- Recherche de slot libre : `BitMapThreadSafe::Find()`
- Si aucun slot : tentative d'expansion via `AllocateSemaphoreTable(maxSemaphores * 2)`
- Allocation objet `Semaphore` sur le heap kernel
- Enregistrement dans `semaphoreTable[handle]`
- BitMap mis à jour

**Après l'appel** :
- `$2` : Handle du sémaphore (0 à maxSemaphores-1) ou -1
- `errno` : 0 ou code d'erreur
- Table : nouveau descripteur valide enregistré (potentiellement agrandie)

## NOTES

- **Handles réutilisables** : Après `SemDestroy(X)`, le handle X peut être réalloué
- **Pas de limite globale** : Chaque processus a sa propre table (max 512 sémaphores)
- **Nettoyage automatique** : Les sémaphores non détruits sont libérés à la terminaison du processus
- **Ordre d'allocation** : Dépend de `BitMap::Find()`, généralement ordre croissant puis réutilisation
- **Pré-allocation** : Utiliser `SetMaxSemForProcess()` pour éviter les réallocations si le nombre est connu

## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale avec adresses
- **v2.0** : Modification avec table de descripteurs (taille fixe 16)
- **v2.1** : Auto-expansion de la table (taille initiale 16, max 512)

## VOIR AUSSI

- [SemWait](SemWait.md) - Opération P (wait) sur un sémaphore
- [SemPost](SemPost.md) - Opération V (signal) sur un sémaphore
- [SemDestroy](./SemDestroy.md) - Destruction d'un sémaphore
- [SetMaxSemForProcess](./SetMaxSemForProcess.md) - Redimensionnement manuel de la table
- [Vue d'ensemble](Sync.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 25 Dec 2025

## DERNIÈRE RÉVISION

5 Jan 2026
