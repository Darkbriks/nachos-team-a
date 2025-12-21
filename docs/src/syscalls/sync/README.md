# Vue d'ensemble - Synchronisation par sémaphores

> Mettre a jour la doc

Les appels système de sémaphores permettent la synchronisation entre threads utilisateur au sein d'un même processus. NachOS fournit une implémentation sécurisée basée sur des descripteurs.

## Opérations disponibles

- **[SemInit](./SemInit.md)** : Crée et initialise un nouveau sémaphore
- **[SemP](./SemP.md)** : Opération P (wait/acquire) sur un sémaphore
- **[SemV](./SemV.md)** : Opération V (signal/release) sur un sémaphore
- **[SemDestroy](./SemDestroy.md)** : Détruit un sémaphore

## Architecture

### Modèle de sécurité

Les sémaphores utilisent une table de descripteurs au niveau de chaque processus (`AddrSpace`). L'utilisateur manipule des descripteurs (entiers), jamais des pointeurs directs vers les structures kernel.

**Avantages** :
- **Isolation complète** : impossible d'accéder aux structures kernel
- **Validation stricte** : chaque descripteur est vérifié avant utilisation
- **Nettoyage automatique** : libération à la terminaison du processus

### Limites actuelles

| Paramètre | Valeur | Configurable |
|-----------|--------|--------------|
| **Sémaphores par processus** | 16 | Non (actuellement) |
| **Partage inter-threads** | Oui | - |
| **Partage inter-processus** | Non | - |

> **Note** : Il devrait être possible dans le futur de spécifier la taille de la table de sémaphores lors de la création du processus, permettant d'éliminer l'overhead si aucune synchronisation n'est requise.

## Cycle de vie d'un sémaphore

```
┌─────────────┐
│  SemInit()  │  Retourne handle (0-15)
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

| Appel        | Retour succès      | Retour erreur | Définit errno |
|--------------|--------------------|---------------|---------------|
| SemInit      | descripteur (0-15) | -1            | Oui           |
| SemP         | 0                  | -1            | Oui           |
| SemV         | 0                  | -1            | Oui           |
| SemDestroy   | 0                  | -1            | Oui           |

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

| Opération    | Complexité temporelle | Notes                          |
|--------------|----------------------|--------------------------------|
| `SemInit`    | O(1)                 | Allocation bitmap              |
| `SemP`       | O(1)*                | *Bloquant si compteur = 0      |
| `SemV`       | O(1)                 | Réveil thread O(1)             |
| `SemDestroy` | O(1)                 | Libération bitmap              |

## Voir aussi

- [Gestion des erreurs](../errors.md) - Codes d'erreur système

## Auteurs

Antoine, 21 Dec 2025

## Dernière révision

21 Dec 2025 par Antoine
