# futex_wake

`futex_wake` - Réveille des threads en attente sur un futex.

## Synopsis

```c
#include "syscall.h"

int futex_wake(int* uaddr, int num_wake);
```

## Description

`futex_wake` réveille jusqu'à `num_wake` threads qui attendent sur le futex à l'adresse `uaddr`.

### Comportement

1. Désactive les interruptions
2. Recherche la file d'attente associée à `uaddr`
3. Si aucun thread n'attend : retourne 0
4. Réveille jusqu'à `num_wake` threads
5. Si la file devient vide : libère les ressources
6. Retourne le nombre de threads effectivement réveillés

## Paramètres

### `uaddr`

Adresse du futex sur lequel réveiller les threads.

**Type** : `int*`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Doit être une adresse utilisateur valide

### `num_wake`

Nombre maximum de threads à réveiller.

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`  
**Valeurs spéciales** : `INT_MAX` pour réveiller tous les threads en attente

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification               |
|--------|-----------------------------|
| `>= 0` | Nombre de threads réveillés |
| `-1`   | Erreur (consulter `errno`)  |

## Exemples

### Exemple 1 : Réveiller un seul thread

```c
#include "syscall.h"

int signal = 0;

void signaler(int arg) {
    // Préparer les données
    // ...
    
    // Signaler qu'un thread peut continuer
    atomic_store(&signal, 1);
    int woken = futex_wake(&signal, 1);
    
    PutString("Threads reveilles: ", 19);
    PutInt(woken);
    PutChar('\n');
    
    thread_exit();
}
```

### Exemple 2 : Broadcast (réveiller tous)

```c
#include "syscall.h"
#include "nos_stdlib.h"  // INT_MAX

int barrier = 0;

void broadcast_signal() {
    atomic_store(&barrier, 1);
    
    // Réveiller TOUS les threads en attente
    int woken = futex_wake(&barrier, INT_MAX);
    
    PutString("Broadcast: ", 11);
    PutInt(woken);
    PutString(" threads reveilles\n", 19);
}
```

## Thread-safety

L'appel est thread-safe. Les opérations sur la file d'attente sont protégées par la désactivation des interruptions.

## Voir aussi

- [futex_wait](./futex_wait.md) - Attendre sur un futex
- [atomic_store](./atomic_store.md) - Écriture atomique
- [Sémaphores](../sync/Sync.md) - Alternative haut niveau
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026