# SetMaxSemForProcess

`SetMaxSemForProcess` - Définit la taille maximale de la table de sémaphores du processus

## SYNOPSIS
```c
#include "syscall.h"

int SetMaxSemForProcess(unsigned int maxSemaphores);
```

## DESCRIPTION

`SetMaxSemForProcess` permet de redimensionner explicitement la table de sémaphores du processus courant. Cette fonction est utile pour pré-allouer une table de grande taille si le nombre de sémaphores nécessaires est connu à l'avance, évitant ainsi les réallocations dynamiques.

Numéro d'appel système : `33`

### Comportement nominal

- Réalloue la table de sémaphores à la taille spécifiée
- Préserve les sémaphores existants en cas d'agrandissement (aucune garantie en cas de réduction)
- Initialise les nouveaux slots comme invalides
- Met à jour le BitMap de tracking

### Cas particuliers

- **maxSemaphores ≤ 0** : Retourne -1, `errno = E_INVAL`
- **maxSemaphores > MAX_SEMAPHORES_PER_PROCESS (512)** : Retourne -1, `errno = E_INVAL`
- **Réduction de taille** : Les sémaphores au-delà de la nouvelle limite sont perdus (non recommandé)

## PARAMÈTRES

### `maxSemaphores`
Nouvelle taille de la table de sémaphores.

**Type** : `unsigned int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être > 0
- Doit être ≤ 512 (`MAX_SEMAPHORES_PER_PROCESS`)

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : `0`

**En cas d'erreur** : `-1` et `errno` est défini

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userSem.cc:handle_SC_SetMaxSemForProcess()`
- **Implémentation** : `code/userprog/addrspace.cc:AddrSpace::AllocateSemaphoreTable()`

### Thread-safety

*TODO*

## DÉCISIONS DE CONCEPTION

### Pourquoi permettre le redimensionnement manuel ?

**Optimisation** : Évite les réallocations multiples si le nombre de sémaphores est connu à l'avance.

**Contrôle mémoire** : Permet de limiter l'utilisation mémoire en spécifiant une taille exacte.

## EXEMPLES

### Exemple 1 : Pré-allocation pour application intensive

```c
#include "syscall.h"

int main() {
    // Pré-allouer 100 sémaphores pour éviter les réallocations
    if (SetMaxSemForProcess(100) < 0) {
        PutString("Erreur: impossible de redimensionner la table\n", 47);
        Exit(-1);
    }
    
    PutString("Table redimensionnée à 100 slots\n", 34);
    
    // Créer les sémaphores nécessaires...
    int sems[50];
    for (int i = 0; i < 50; i++) {
        sems[i] = SemInit(1);
    }
    
    // Utilisation...
    
    // Nettoyage
    for (int i = 0; i < 50; i++) {
        SemDestroy(sems[i]);
    }
    
    return 0;
}
```

### Exemple 2 : Gestion d'erreur

```c
#include "syscall.h"

int main() {
    // Tenter une taille invalide (0)
    if (SetMaxSemForProcess(0) < 0) {
        PutString("Erreur attendue: taille 0 invalide\n", 36);
    }
    
    // Tenter une taille trop grande
    if (SetMaxSemForProcess(1000) < 0) {
        PutString("Erreur attendue: taille > 512\n", 31);
    }
    
    // Taille valide
    if (SetMaxSemForProcess(256) == 0) {
        PutString("Redimensionnement à 256 réussi\n", 32);
    }
    
    return 0;
}
```

**Sortie attendue** :
```
Erreur attendue: taille 0 invalide
Erreur attendue: taille > 512
Redimensionnement à 256 réussi
```

### Exemple 3 : DÉCONSEILLÉ - Réduction de la table

```c
#include "syscall.h"

int main() {
    // Créer des sémaphores
    int sem1 = SemInit(1);  // handle 0
    int sem2 = SemInit(1);  // handle 1
    
    PutString("Créé sémaphores 0 et 1\n", 24);
    
    // ⚠️ DANGER : Réduire la table à 1 slot
    SetMaxSemForProcess(1);
    
    // sem1 (handle 0) est toujours valide
    if (SemWait(sem1) == 0) {
        PutString("sem1 OK\n", 9);
        SemPost(sem1);
    }
    
    // sem2 (handle 1) est PERDU
    if (SemWait(sem2) < 0) {
        PutString("sem2 perdu!\n", 13);
    }
    
    return 0;
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Nouvelle taille souhaitée
- `maxSemaphores` : Taille actuelle de la table
- Table : peut contenir 0-N sémaphores actifs

**Pendant l'appel** :
- Allocation nouvelle table
- Copie des entrées (min(ancienne_taille, nouvelle_taille))
- Mise à jour du BitMap
- Libération ancienne table

**Après l'appel** :
- `$2` : 0 (succès) ou -1 (erreur)
- `maxSemaphores` : Nouvelle taille
- Table : redimensionnée, sémaphores existants préservés (si agrandissement)

## NOTES

- **Taille initiale** : 16 sémaphores par défaut (défini par `INITIAL_SEMAPHORE_TABLE_SIZE`)
- **Taille maximale** : 512 sémaphores (défini par `MAX_SEMAPHORES_PER_PROCESS`)
- **Auto-expansion** : `SemInit()` agrandit automatiquement la table si nécessaire (doublement)
- **Cas d'usage principal** : Pré-allocation pour éviter les réallocations dynamiques

## FAILLES ET VULNÉRABILITÉS

Aucune vulnérabilité connue à ce jour.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale

## VOIR AUSSI

- [SemInit](./SemInit.md) - Création d'un sémaphore (déclenche auto-expansion si nécessaire)
- [SemDestroy](./SemDestroy.md) - Destruction d'un sémaphore
- [Vue d'ensemble](Sync.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 07 Jan 2026
Tommy, 05 Jan 2026

## DERNIÈRE RÉVISION

07 Jan 2026
