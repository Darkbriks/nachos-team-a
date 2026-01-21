# mmap

## Synopsis

```c
#include "syscall.h"

int mmap(void* addr, int length);
```

## Description

`mmap` alloue une zone de mémoire dans l'espace d'adressage du processus.

<div class="callout callout-warning">
    <div class="callout-title">Implémentation simplifiée</div>
    <div class="callout-content">
        <strong>IMPORTANT</strong> : Cette implémentation de <code>mmap</code> est simplifiée
        et <strong>non conforme</strong> au véritable syscall Linux. Elle n'est pas terminée
        et ne gère que l'allocation basique dans la zone de stack via <code>StackManager</code>.
        <br><br>
        Limitations principales :
        <ul>
            <li>Le paramètre <code>addr</code> est ignoré (allocation automatique)</li>
            <li>Pas de support des flags (PROT_*, MAP_*)</li>
            <li>Pas de mapping de fichiers</li>
            <li>Allocation uniquement dans la région de stack</li>
        </ul>
    </div>
</div>

### Comportement actuel

Dans NachOS, `mmap` effectue les opérations suivantes :

1. Ignore le paramètre `addr` (allocation automatique)
2. Alloue une zone de `length` octets via `StackManager::AllocateStack()`
3. Retourne l'adresse de **base** (limite inférieure) de la zone allouée

La zone allouée se trouve dans la région des stacks, entre `stackLimit` et le haut de la mémoire.

### Utilisation principale

Dans l'implémentation actuelle, `mmap` est principalement utilisé par la bibliothèque pthread pour allouer les stacks des threads :

```c
// Dans nos_pthread.c
t->stack_base = (void*)mmap(NULL, t->stack_size);
```

## Paramètres

### `addr`

Adresse souhaitée pour le mapping (actuellement **ignoré**).

**Type** : `void*` (registre `$4`)  
**Direction** : IN  
**Valeur** : Ignorée dans l'implémentation actuelle (passer `NULL`)

### `length`

Taille de la zone à allouer en octets.

**Type** : `int` (registre `$5`)  
**Direction** : IN  
**Contraintes** :
- Doit être `> 0`
- Arrondi a la page en interne (128 octets)

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur  | Signification                                    |
|---------|--------------------------------------------------|
| `> 0`   | Adresse de base (limite) de la zone allouée      |
| `-1`    | Erreur (consulter `errno`)                       |

## Erreurs

| errno     | Condition                                        |
|-----------|--------------------------------------------------|
| `E_FAULT` | Processus ou `AddrSpace` invalide                |
| `E_NOMEM` | Pas assez d'espace disponible dans la zone stack |

## Implementation

### Allocation dans StackManager

Le `StackManager` maintient une liste de zones allouées et trouve un emplacement libre dans la région `[stackLimit, numPages * PageSize]`.

## Limitations

<div class="callout callout-warning">
    <div class="callout-title">Implémentation incomplète</div>
    <div class="callout-content">
        <ul>
            <li><strong>Pas de contrôle sur l'adresse</strong> : le paramètre <code>addr</code> est ignoré</li>
            <li><strong>Pas de flags</strong> : pas de <code>PROT_READ</code>, <code>PROT_WRITE</code>, <code>MAP_PRIVATE</code>, etc.</li>
            <li><strong>Pas de mapping de fichiers</strong> : uniquement de la mémoire anonyme</li>
            <li><strong>Pas de MAP_SHARED</strong> : pas de partage entre processus</li>
            <li><strong>Allocation dans la stack</strong> : limite l'espace disponible pour les vraies stacks</li>
        </ul>
    </div>
</div>

## Thread-safety

`mmap` est **thread-safe**. Les allocations sont protégées par le `StackManager` qui gère les accès concurrents.

## Relation avec munmap

Toute zone allouée par `mmap` **doit** être libérée avec `munmap` pour éviter les fuites mémoire :

```c
void* zone = (void*)mmap(NULL, 1024);
// ... utiliser la zone ...
munmap(zone);  // Libération obligatoire
```

Voir [munmap](./munmap.md) pour plus de détails.

## Voir aussi

- [munmap](./munmap.md) - Libérer une zone mmap
- [Memory Overview](./Memory.md) - Vue d'ensemble de la gestion mémoire
- [Sbrk](./Sbrk.md) - Extension du heap
- [Threads](../threads/Threads.md) - Utilisation pour les stacks de threads
- [errno](../../libs/errno.md) - Gestion des erreurs

## Auteurs

Antoine

## Dernière révision

21 Jan 2026