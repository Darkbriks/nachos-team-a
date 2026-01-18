# Bibliothèques utilisateur

Cette section documente les bibliothèques disponibles en espace utilisateur pour les programmes NachOS.

## Vue d'ensemble

Les bibliothèques utilisateur fournissent une couche d'abstraction au-dessus des appels système bruts, offrant une API plus simple et des fonctionnalités supplémentaires.

## Bibliothèques disponibles

| Bibliothèque | Header         | Description                      |
|--------------|----------------|----------------------------------|
| errno        | `nos_errno.h`  | Gestion des erreurs thread-safe  |
| stdlib       | `nos_stdlib.h` | Fonctions utilitaires standard   |
| stdio        | `nos_stdio.h`  | Entrées/sorties formatées        |
| string       | `nos_string.h` | Manipulation de chaînes          |
| pthread      | `pthread.h`    | Threads POSIX-like (haut niveau) |

## Voir aussi

- [errno](./errno.md) - Gestion des erreurs
- [Appels système](../syscalls/Syscalls.md) - API bas niveau

## Auteurs

Antoine

## Dernière révision

18 Jan 2026