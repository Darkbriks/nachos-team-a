# atomic_cmpxchg

`atomic_cmpxchg` - Compare-and-swap atomique.

## Synopsis

```c
#include "syscall.h"

int atomic_cmpxchg(int* uaddr, int expected, int desired);
```

## Description

`atomic_cmpxchg` effectue une opération atomique de type "compare-and-swap" (CAS). Elle compare la valeur à l'adresse `uaddr` avec `expected`, et si elles sont égales, remplace la valeur par `desired`. L'opération retourne toujours la valeur originale.

### Comportement

L'opération est équivalente au pseudo-code suivant, mais exécutée de manière atomique :

```c
int atomic_cmpxchg(int* uaddr, int expected, int desired) {
    int original = *uaddr;
    if (original == expected) {
        *uaddr = desired;
    }
    return original;
}
```

### Garanties d'atomicité

- La lecture, comparaison et écriture conditionnelle sont indivisibles
- Aucun autre thread ne peut observer un état intermédiaire
- L'opération est sérialisée par rapport aux autres opérations atomiques

## Paramètres

### `uaddr`

Adresse de l'entier sur lequel effectuer l'opération.

**Type** : `int*`  
**Direction** : IN/OUT  
**Registre** : `$4`  
**Contraintes** : Adresse utilisateur valide, alignée sur 4 octets

### `expected`

Valeur attendue pour que l'échange ait lieu.

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`

### `desired`

Nouvelle valeur à écrire si la comparaison réussit.

**Type** : `int`  
**Direction** : IN  
**Registre** : `$6`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur          | Signification                                         |
|-----------------|-------------------------------------------------------|
| `== expected`   | L'échange a eu lieu                                   |
| `!= expected`   | L'échange n'a pas eu lieu (valeur actuelle retournée) |
| `-1` avec errno | Erreur                                                |

## ABA Problem

<div class="callout callout-warning">
    <div class="callout-title">Problème ABA</div>
    <div class="callout-content">
        <p>Le CAS peut souffrir du problème ABA : si la valeur passe de A à B puis revient à A,
        le CAS ne détecte pas le changement intermédiaire.</p>
    </div>
</div>

## Thread-safety

L'opération est atomique et thread-safe par conception. C'est la primitive de base pour construire des structures de données thread-safe.

## Voir aussi

- [atomic_load](./atomic_load.md) - Lecture atomique
- [atomic_store](./atomic_store.md) - Écriture atomique
- [futex_wait](./futex_wait.md) - Attente sur futex
- [futex_wake](./futex_wake.md) - Réveil de threads
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026