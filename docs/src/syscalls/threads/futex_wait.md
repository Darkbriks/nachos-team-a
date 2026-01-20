# futex_wait

`futex_wait` - Attend sur un futex si sa valeur correspond à l'attendue.

## Synopsis

```c
#include "syscall.h"

int futex_wait(int* uaddr, int expected);
```

## Description

`futex_wait` vérifie atomiquement que la valeur à l'adresse `uaddr` est égale à `expected`, et si c'est le cas, met le thread appelant en sommeil jusqu'à ce qu'un autre thread appelle `futex_wake` sur la même adresse.

### Comportement

1. Désactive les interruptions
2. Lit la valeur à `uaddr`
3. Si la valeur diffère de `expected` : retourne immédiatement avec `E_AGAIN`
4. Sinon : ajoute le thread à la file d'attente du futex et l'endort
5. Au réveil : retourne 0

<div class="callout callout-note">
    <div class="callout-title">Atomicité</div>
    <div class="callout-content">
        La vérification de la valeur et la mise en sommeil sont atomiques.
    </div>
</div>

## Paramètres

### `uaddr`

Adresse de l'entier à surveiller (le futex).

**Type** : `int*`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Doit être une adresse utilisateur valide, alignée sur 4 octets

### `expected`

Valeur attendue à l'adresse `uaddr`.

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification                  |
|--------|--------------------------------|
| `0`    | Succès (thread a été réveillé) |
| `-1`   | Erreur (consulter `errno`)     |

## Exemples

### Exemple 1 : Attente simple

```c
#include "syscall.h"

int flag = 0;

void waiter(int arg) {
    PutString("Attente du signal...\n", 21);
    
    // Attendre que flag devienne != 0
    while (atomic_load(&flag) == 0) {
        futex_wait(&flag, 0);
    }
    
    PutString("Signal recu!\n", 13);
    thread_exit();
}

void signaler(int arg) {
    // Simuler du travail
    for (volatile int i = 0; i < 10000; i++);
    
    // Signaler
    atomic_store(&flag, 1);
    futex_wake(&flag, 1);
    
    thread_exit();
}
```

### Exemple 2 : Gestion du spurious wakeup

```c
#include "syscall.h"

int ready = 0;

void consumer(int arg) {
    // Boucle pour gérer les réveils intempestifs
    while (atomic_load(&ready) == 0) {
        int ret = futex_wait(&ready, 0);
        
        if (ret == -1 && errno == E_AGAIN) {
            // La valeur a changé, re-vérifier
            continue;
        }
    }
    
    PutString("Donnee prete\n", 13);
    thread_exit();
}
```

## Thread-safety

L'appel est thread-safe. La vérification et la mise en sommeil sont atomiques grâce à la désactivation des interruptions.

## Voir aussi

- [futex_wake](./futex_wake.md) - Réveiller des threads
- [atomic_cmpxchg](./atomic_cmpxchg.md) - Compare-and-swap atomique
- [atomic_load](./atomic_load.md) - Lecture atomique
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026