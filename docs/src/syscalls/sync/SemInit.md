# SemInit

`SemInit` - Crée et initialise un nouveau sémaphore

> Mettre a jour la doc

## SYNOPSIS
```c
#include "syscall.h"

int SemInit(int value);
```

## DESCRIPTION

`SemInit` crée un nouveau sémaphore avec une valeur initiale spécifiée et retourne un déscripteur permettant de l'identifier. Le sémaphore peut ensuite être utilisé pour la synchronisation entre threads du même processus.

Numéro d'appel système : `24`

### Comportement nominal

- Alloue un nouveau descripteur dans la table de sémaphores du processus
- Initialise le compteur interne du sémaphore à `value`
- Retourne un handle unique (entier entre 0 et 15)
- Le sémaphore est partagé entre tous les threads du processus
- Le sémaphore persiste jusqu'à destruction explicite ou terminaison du processus

### Cas particuliers

- **value < 0** : Retourne -1, `errno = E_INVAL`
- **Table pleine** : Retourne -1, `errno = E_FTABLE` (maximum 16 sémaphores)

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

**En cas de succès** : Handle du sémaphore (0 à 15)

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante   | Condition                      |
|-------|-------------|--------------------------------|
| 1     | `E_INVAL`   | `value < 0`                    |
| 10    | `E_FTABLE`  | Table de sémaphores pleine     |

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/exception.cc:handle_SC_SemInit()`
- **Implémentation** : `code/userprog/addrspace.cc:AddrSpace::SemaphoreCreate()`

### Architecture interne

**Table de descripteurs** :
- Tableau fixe de 16 entrées
- BitMap pour tracking des slots utilisés/libres
- Chaque descripteur contient :
  - `Semaphore* semaphore` : pointeur vers objet kernel
  - `bool valid` : flag de validité

**Allocation** :
1. Vérification de `value ≥ 0`
2. Recherche d'un slot libre via `BitMap::Find()` → O(1)
3. Création de `Semaphore("user_sem", value)`
4. Enregistrement dans la table
5. Marquage du slot comme utilisé

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

### Exemple 3 : Gestion d'erreur - table pleine

```c
#include "syscall.h"

int main() {
    int sems[20];
    int count = 0;
    
    // Tenter de créer 20 sémaphores
    for (int i = 0; i < 20; i++) {
        sems[i] = SemInit(0);
        
        if (sems[i] < 0) {
            PutString("Table pleine après ", 20);
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
Table pleine après 16 sémaphores
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Valeur initiale du sémaphore
- Table de sémaphores : peut contenir 0-16 descripteurs actifs
- BitMap : indique slots libres/occupés

**Pendant l'appel** :
- Recherche de slot libre : `BitMapThreadSafe::Find()`
- Allocation objet `Semaphore` sur le heap kernel
- Enregistrement dans `semaphoreTable[handle]`
- BitMap mis à jour

**Après l'appel** :
- `$2` : Handle du sémaphore (0-15) ou -1
- `errno` : 0 ou code d'erreur
- Table : nouveau descripteur valide enregistré

## NOTES

- **Handles réutilisables** : Après `SemDestroy(X)`, le handle X peut être réalloué
- **Pas de limite globale** : Chaque processus a sa propre table de 16 sémaphores
- **Nettoyage automatique** : Les sémaphores non détruits sont libérés à la terminaison du processus
- **Ordre d'allocation** : Dépend de `BitMap::Find()`, généralement ordre croissant puis réutilisation

## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## HISTORIQUE DES VERSIONS

- **v1.0** : Implémentation initiale avec adresses
- **v2.0** : Modification avec table de descripteurs

## VOIR AUSSI

- [SemP](SemWait.md) - Opération P (wait) sur un sémaphore
- [SemV](SemPost.md) - Opération V (signal) sur un sémaphore
- [SemDestroy](./SemDestroy.md) - Destruction d'un sémaphore
- [Vue d'ensemble](./README.md) - Guide complet des sémaphores

## AUTEURS

Antoine, 21 Dec 2025

## DERNIÈRE RÉVISION

21 Dec 2025 par Antoine
