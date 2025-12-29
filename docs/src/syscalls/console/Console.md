# Vue d'ensemble - Console I/O

Les appels système de console permettent l'interaction entre les programmes utilisateur et le terminal. NachOS fournit deux catégories d'opérations :

## Opérations de base

### Sortie
- **[PutChar](./PutChar.md)** : Affiche un caractère unique
- **[PutString](./PutString.md)** : Affiche une chaîne de caractères
- **[PutInt](./PutInt.md)** : Affiche un entier signé

### Entrée
- **[GetChar](./GetChar.md)** : Lit un caractère
- **[GetString](./GetString.md)** : Lit une chaîne de caractères
- **[GetInt](./GetInt.md)** : Lit un entier signé

## Architecture

Tous les appels système de console utilisent la classe `SynchConsole` qui fournit :
- **Synchronisation** : Accès thread-safe via sémaphores
- **Blocking I/O** : Les appels bloquent jusqu'à complétion
- **Pas de buffering** : Communication directe avec le hardware

## Gestion des erreurs

| Appel     | Gestion errno | Valeur de retour         |
|-----------|---------------|--------------------------|
| PutChar   | Non           | void                     |
| GetChar   | Non           | char                     |
| PutString | Oui           | int (bytes écrits ou -1) |
| GetString | Oui           | int (bytes lus ou -1)    |
| PutInt    | Oui           | int (0 ou -1)            |
| GetInt    | Oui           | int (0 ou -1)            |

## Synchronisation

Le sémaphore `IO_Lock` garantit qu'un seul thread accède à la console à la fois, évitant l'entrelacement des caractères.

## Voir aussi

- [Gestion des erreurs](../errors.md)