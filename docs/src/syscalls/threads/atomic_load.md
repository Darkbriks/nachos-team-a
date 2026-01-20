# atomic_load

`atomic_load` - Lecture atomique d'une valeur en mémoire.

## Synopsis

```c
#include "syscall.h"

int atomic_load(int* uaddr);
```

## Description

`atomic_load` lit de manière atomique la valeur à l'adresse `uaddr`. La lecture est garantie être indivisible : la valeur retournée est cohérente et ne peut pas être un mélange de deux écritures concurrentes.

### Comportement

L'opération est équivalente à :

```c
return *uaddr;
```

Mais avec la garantie que la lecture est atomique et reflète l'état le plus récent de la mémoire.

## Paramètres

### `uaddr`

Adresse de l'entier à lire.

**Type** : `int*`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : Adresse utilisateur valide, alignée sur 4 octets

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur          | Signification  |
|-----------------|----------------|
| Valeur lue      | Succès         |
| `-1` avec errno | Erreur         |

## Thread-safety

L'opération est atomique et thread-safe.

## Voir aussi

- [atomic_store](./atomic_store.md) - Écriture atomique
- [atomic_cmpxchg](./atomic_cmpxchg.md) - Compare-and-swap
- [futex_wait](./futex_wait.md) - Attente sur futex
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026