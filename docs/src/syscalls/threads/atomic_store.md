# atomic_store

`atomic_store` - Écriture atomique d'une valeur en mémoire.

## Synopsis

```c
#include "syscall.h"

void atomic_store(int* uaddr, int value);
```

## Description

`atomic_store` écrit de manière atomique la valeur `value` à l'adresse `uaddr`. L'écriture est garantie être indivisible : aucun autre thread ne peut observer un état partiel de l'écriture.

### Comportement

L'opération est équivalente à :

```c
*uaddr = value;
```

Mais avec la garantie que l'écriture est atomique et visible immédiatement par tous les threads.

## Paramètres

### `uaddr`

Adresse de l'entier où écrire.

**Type** : `int*`  
**Direction** : OUT  
**Registre** : `$4`  
**Contraintes** : Adresse utilisateur valide, alignée sur 4 octets

### `value`

Valeur à écrire.

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`

## Valeur de retour

Aucune.

## Thread-safety

L'opération est atomique et thread-safe. L'écriture est garantie être visible par tous les threads immédiatement après le retour de l'appel.

## Voir aussi

- [atomic_load](./atomic_load.md) - Lecture atomique
- [atomic_cmpxchg](./atomic_cmpxchg.md) - Compare-and-swap
- [futex_wake](./futex_wake.md) - Réveiller des threads
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026