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
#define SC_Halt                 0
#define SC_PutChar              1
#define SC_PutString            2
#define SC_GetChar              3
#define SC_GetString            4
#define SC_PutInt               5
#define SC_GetInt               6
#define SC_Sleep                7
#define SC_SleepUntil           8
#define SC_GetCurrentTick       9
#define SC_time                44
#define SC_SemInit             10
#define SC_SemWait             11
#define SC_SemPost             12
#define SC_SemDestroy          13
#define SC_SetMaxSemForProcess 14
#define SC_futex_wait          15
#define SC_futex_wake          16
#define SC_atomic_cmpxchg      17
#define SC_atomic_store        18
#define SC_atomic_load         19
#define SC_Sbrk                20
#define SC_mmap                21
#define SC_munmap              22
#define SC_thread_create       23
#define SC_thread_exit         24
#define SC_thread_self         25
#define SC_thread_yield        26
#define SC_ForkExec            27
#define SC_ForkJoin            28
#define SC_ForkSelf            29
#define SC_Exit                30
#define SC_connect             31
#define SC_listen              32
#define SC_accept              33
#define SC_sendto              34
#define SC_recvfrom            35
#define SC_close               36
#define SC_Create              37
#define SC_Open                38
#define SC_Read                39
#define SC_Write               40
#define SC_Close               41
#define SC_FileLen             42
#define SC_Seek                43
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

Antoine, 21 Jan 2026
Tommy, 08 Jan 2026

## Dernière révision

21 Jan 2026
