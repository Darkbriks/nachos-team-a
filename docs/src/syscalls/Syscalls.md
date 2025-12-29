# Vue d'ensemble des appels système

Cette page fournit une vue d'ensemble complète de tous les appels système disponibles dans NachOS.

## Introduction

NachOS implémente un ensemble simplifié d'appels système inspiré de UNIX/POSIX. Ces appels permettent aux programmes utilisateur d'interagir avec le noyau pour :
- **I/O Console** : Lire et écrire sur la console
- **Gestion de processus** : Créer et gérer des processus
- **Gestion de threads** : Créer et synchroniser des threads utilisateur
- **Système de fichiers** : Manipuler des fichiers (partiellement implémenté)

## Organisation des appels système

### Par numéro (SC_*)

Les appels système sont identifiés par des numéros définis dans `syscall.h` :

```c
#define SC_Halt           0
#define SC_Exit           1
#define SC_Exec           2
#define SC_Join           3
#define SC_Create         4
#define SC_Open           5
#define SC_Read           6
#define SC_Write          7
#define SC_Close          8
#define SC_Fork           9
#define SC_Yield          10
#define SC_PutChar        11
#define SC_PutString      12
#define SC_GetChar        13
#define SC_GetString      14
#define SC_PutInt         15
#define SC_GetInt         16
#define SC_CreateThread   17
#define SC_ExitThread     18
#define SC_JoinThread     19
#define SC_Sleep          20
#define SC_SleepUntil     21
#define SC_GetCurrentTick 22
```

## Limitations connues

### Thread-safety partielle

⚠️ **Console I/O** : Thread-safe au niveau caractère mais pas au niveau message complet.

**Exemple de problème** :
```c
// Thread 1
PutString("AAAA", 4);

// Thread 2 (simultané)
PutString("BBBB", 4);

// Résultat possible: "AABBAABB" au lieu de "AAAABBBB"
```

**Solution** : Protéger les séquences d'I/O avec des sémaphores si nécessaire.

### errno global

⚠️ **Non thread-safe** : `errno` est une variable globale partagée.

**Problème** : Plusieurs threads peuvent écraser mutuellement leur `errno`.

**Correction prévue** : Thread-local errno sera implémenté prochainement.

Voir : [Gestion des erreurs](./errors.md#failles-et-vulnérabilités)

## Architecture d'un appel système

### Flux d'exécution

Le flux d'un appel système est complexe, et demande un changement de contexte du processeur. A ce titre, il est fortement conseillé de limiter le nombre d'appels système dans le code utilisateur pour minimiser la surcharge associée.

```
Programme utilisateur
        ↓
    Stub (start.S)
        ↓  syscall
    Exception
        ↓
ExceptionHandler (exception.cc)
        ↓
handler_SC_XXX()
        ↓
Implémentation (synchconsole.cc, userthread.cc, etc.)
        ↓
    Retour
        ↓
UpdatePC()
        ↓
Programme utilisateur
```

### Exemple détaillé : PutChar

1. **Programme** : `PutChar('A')`
2. **Stub** : Charge SC_PutChar (11) dans $2, 'A' dans $4
3. **syscall** : Déclenche exception
4. **ExceptionHandler** : Détecte SC_PutChar
5. **handler_SC_putChar** : Lit $4, appelle SynchPutChar
6. **SynchConsole** : Acquiert lock, écrit caractère
7. **UpdatePC** : Incrémente PC
8. **Retour** : Programme continue

## Voir aussi

- [Console I/O Overview](console/Console.md) - Détails des appels console
- [Gestion des erreurs](./errors.md) - Mécanisme errno
- [Liste des codes errno](./errno.md) - Tous les codes d'erreur

## Auteurs

Antoine, 20 Dec 2025

## Dernière révision

20 Dec 2025 par Antoine