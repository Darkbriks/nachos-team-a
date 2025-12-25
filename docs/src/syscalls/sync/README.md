# Vue d'ensemble - Synchronisation par sémaphores

Les appels système de sémaphores permettent la synchronisation entre threads utilisateur au sein d'un même processus. NachOS fournit une implémentation sécurisée basée sur des descripteurs.

## Opérations disponibles

- **[SemInit](./SemInit.md)** : Crée et initialise un nouveau sémaphore
- **[SemP](./SemP.md)** : Opération P (wait/acquire) sur un sémaphore
- **[SemV](./SemV.md)** : Opération V (signal/release) sur un sémaphore
- **[SemDestroy](./SemDestroy.md)** : Détruit un sémaphore
- **[SetMaxSemForProcess](./SetMaxSemForProcess.md)** : Redimensionne la table de sémaphores

## Architecture

### Modèle de sécurité

Les sémaphores utilisent une table de descripteurs au niveau de chaque processus (`AddrSpace`). L'utilisateur manipule des descripteurs (entiers), jamais des pointeurs directs vers les structures kernel.

**Avantages** :
- **Isolation complète** : impossible d'accéder aux structures kernel
- **Validation stricte** : chaque descripteur est vérifié avant utilisation
- **Nettoyage automatique** : libération à la terminaison du processus

### Limites et dimensionnement

| Paramètre                   | Valeur   | Configurable       |
|-----------------------------|----------|--------------------|
| **Taille initiale table**   | 16       | Non (compile-time) |
| **Taille maximale table**   | 512      | Non (compile-time) |
| **Auto-expansion**          | Oui (×2) | -                  |
| **Partage inter-threads**   | Oui      | -                  |
| **Partage inter-processus** | Non      | -                  |

### Gestion dynamique de la table

La table de sémaphores s'adapte automatiquement aux besoins :

1. **Taille initiale** : 16 slots (défini par `INITIAL_SEMAPHORE_TABLE_SIZE`)
2. **Auto-expansion** : Si `SemInit()` est appelé et la table est pleine, elle double automatiquement
3. **Taille maximale** : 512 slots (défini par `MAX_SEMAPHORES_PER_PROCESS`)
4. **Pré-allocation** : `SetMaxSemForProcess()` permet de dimensionner manuellement

> **Note** : Le facteur d'expansion (×2) est susceptible d'évoluer dans les versions futures.

## Cycle de vie d'un sémaphore

```
┌─────────────┐
│  SemInit()  │  Retourne handle (0 à maxSemaphores-1)
└──────┬──────┘
       │
       ▼
┌─────────────────┐
│ Sémaphore actif │ ◄──┐
│  (utilisable)   │    │
└────┬────────┬───┘    │
     │        │        │
     │        │        │
  SemP()   SemV()      │ Utilisation répétée
     │        │        │
     │        │        │
     └────────┘────────┘
       │
       ▼
┌──────────────────┐
│  SemDestroy()    │  Libère le handle
└──────────────────┘
```

## Gestion des erreurs

Tous les appels système de sémaphores retournent un code d'erreur et définissent `errno` en cas d'échec.

| Appel                      | Succès      | Échec  | Codes errno possibles |
|----------------------------|-------------|--------|-----------------------|
| `SemInit(value)`           | handle (≥0) | -1     | `E_INVAL`, `E_FTABLE` |
| `SemP(sem_id)`             | 0           | -1     | `E_NOENT`             |
| `SemV(sem_id)`             | 0           | -1     | `E_NOENT`             |
| `SemDestroy(sem_id)`       | 0           | -1     | `E_NOENT`             |
| `SetMaxSemForProcess(max)` | 0           | -1     | `E_INVAL`             |

## Thread-safety

**Garanti** :
- Les opérations P() et V() sont atomiques et thread-safe
- Plusieurs threads peuvent utiliser le même sémaphore simultanément
- La table de descripteurs est protégée par un verrou interne

**Comportement** :
- `SemP()` bloque le thread appelant si le compteur est à 0
- `SemV()` réveille un thread bloqué en attente (aucune garentie sur le thread reveillé)


## Exemple complet

```c
#include "syscall.h"

// Variables partagées
int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemP(mutex_id);      // Entrer en section critique
        counter++;
        SemV(mutex_id);      // Sortir de section critique
    }
    ExitThread();
}

int main() {
    // Créer un mutex (sémaphore binaire)
    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        Exit(-1);
    }
    
    // Créer 2 threads
    int tid1 = CreateThread(increment_thread, 0);
    int tid2 = CreateThread(increment_thread, 0);
    
    // Attendre les threads
    JoinThread(tid1);
    JoinThread(tid2);
    
    // Vérifier résultat
    PutString("Counter final: ", 15);
    PutInt(counter);  // Devrait être 2000
    PutChar('\n');
    
    // Nettoyer
    SemDestroy(mutex_id);
    
    return 0;
}
```

## Limitations connues

### 1. Pas de timeout

`SemP()` peut bloquer indéfiniment si `SemV()` n'est jamais appelé.

**Impact** : Impossible d'implémenter des locks avec timeout.

### 2. Limite de 16 sémaphores

Maximum 16 sémaphores actifs par processus.

**Impact** : Applications complexes peuvent manquer de descripteurs.

**Note** : Configuration dynamique de la taille de la table envisagée.

### 3. Pas de partage inter-processus

Les sémaphores sont locaux à un processus.

**Impact** : Impossible de synchroniser des processus différents.

## Performances

| Opération                    | Complexité temporelle | Notes                     |
|------------------------------|-----------------------|---------------------------|
| `SemInit` (liste non pleine) | O(1)                  | Allocation O(1)           |
| `SemInit` (liste pleine)     | O(n)                  | Réallocation O(n)         |
| `SemP`                       | O(1)*                 | *Bloquant si compteur = 0 |
| `SemV`                       | O(1)                  | Réveil thread O(1)        |
| `SemDestroy`                 | O(1)                  | Libération bitmap         |

## Voir aussi

- [Gestion des erreurs](../errors.md) - Codes d'erreur système

## Auteurs

Antoine, 25 Dec 2025

## Dernière révision

25 Dec 2025 par Antoine
