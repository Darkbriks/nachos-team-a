# Gestion mémoire - Appels système

Cette page documente les appels système pour la gestion de la mémoire dynamique dans NachOS.

## Vue d'ensemble

NachOS fournit des appels système permettant la gestion de la mémoire dynamique au niveau utilisateur. Ces syscalls permettent d'étendre la heap et d'allouer des zones mémoire supplémentaires.

### Organisation mémoire

```
Espace d'adressage du processus
┌────────────────────────────┐ ← Haut de mémoire (numPages * PageSize)
│      Stack thread N        │
├────────────────────────────┤
│      Stack thread 1        │
├────────────────────────────┤
│      Stack thread 0        │
├────────────────────────────┤
│    Zones mmap (stacks)     │
├────────────────────────────┤
│      Espace libre          │
├────────────────────────────┤ ← stackLimit
│          Heap              │ ← Grandit vers le haut (via sbrk)
│        (dynamique)         │
├────────────────────────────┤ ← heapStart (brk initial)
│      .bss (non init)       │
├────────────────────────────┤
│      .data (init)          │
├────────────────────────────┤
│      .text (code)          │
└────────────────────────────┘ ← 0
```

## API rapide

| Syscall                 | Description                         |
|-------------------------|-------------------------------------|
| [Sbrk](./Sbrk.md)       | Étendre/interroger le heap          |
| [mmap](./mmap.md)       | Allouer une zone mémoire            |
| [munmap](./munmap.md)   | Libérer une zone mémoire allouée    |

## Concepts

L'organisation de la mémoire utilisateur dans NachOS est fortement inspirée de Linux.

### Heap

La heap est la zone de mémoire dynamique utilisée pour les allocations (malloc). Elle débute juste après les segments statiques (.bss) et peut grandir vers le haut via `Sbrk`.

**Caractéristiques** :
- Adresse de départ : `heapStart` (alignée sur une page)
- Limite actuelle : `brk`
- Granularité : pages (128 octets dans NachOS)
- Direction de croissance : vers le haut (adresses croissantes)
- Limite maximale : `stackLimit` (limite inférieure de la zone de stack)

### Allocations mmap

Les allocations `mmap` permettent de réserver des zones mémoire en dehors du heap principal. Dans l'implémentation actuelle de NachOS, `mmap` alloue des zones dans la région des stacks.

**Caractéristiques** :
- Zone allouée : région de stack via `StackManager`
- Granularité : octets (arrondi a la page en interne)
- Utilisation principale : stacks de threads, zones temporaires

<div class="callout callout-warning">
    <div class="callout-title">Implémentation simplifiée</div>
    <div class="callout-content">
        Les syscalls <code>mmap</code> et <code>munmap</code> dans NachOS ne sont pas conformes
        aux véritables syscalls Linux. L'implémentation actuelle est simplifiée et alloue
        uniquement dans la zone de stack. Une implémentation complète est prévue mais non réalisée.
    </div>
</div>

## Gestion de la mémoire

### Extension du heap (Sbrk)

```
Avant Sbrk(2) :
┌──────────┐
│  .bss    │
├──────────┤ ← heapStart = 0x1000
│  Heap    │
│ (2 pages)│
├──────────┤ ← brk = 0x1100
│          │
│  Libre   │
│          │
└──────────┘

Après Sbrk(2) :
┌──────────┐
│  .bss    │
├──────────┤ ← heapStart = 0x1000
│  Heap    │
│ (4 pages)│
│          │
├──────────┤ ← brk = 0x1200
│          │
│  Libre   │
└──────────┘
```

### Limites

**Contraintes du heap** :
- Maximum : `MAX_HEAP_PAGES` (256 pages par défaut)
- Ne peut pas dépasser `stackLimit`
- Nécessite des frames physiques disponibles

**Contraintes mmap** :
- Dépend de l'espace disponible dans la zone de stack
- Limité par le nombre de stacks allouables

## Relation avec les threads

Les syscalls de gestion mémoire sont essentiels pour les threads :

- **thread_create** : utilise `mmap` pour allouer les stacks de threads
- **Bibliothèque pthread** : utilise `Sbrk` pour l'allocateur mémoire interne

Voir la [documentation des threads](../threads/Threads.md) pour plus de détails.

## Thread-safety

- `Sbrk` : **Thread-safe**. Protégé au niveau du `AddrSpace`.
- `mmap`/`munmap` : **Thread-safe**. Protégés par le `StackManager`.

Plusieurs threads peuvent allouer de la mémoire simultanément sans risque de corruption.

## Voir aussi

- [Sbrk](./Sbrk.md) - Étendre le heap
- [mmap](./mmap.md) - Allouer une zone mémoire
- [munmap](./munmap.md) - Libérer une zone mémoire
- [Threads](../threads/Threads.md) - Gestion des threads
- [errno](../../libs/errno.md) - Gestion des erreurs

## Auteurs

Antoine

## Dernière révision

21 Jan 2026