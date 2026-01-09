# Appels Système de Contrôle

Cette section documente les appels système de contrôle du système Nachos, qui permettent de gérer le cycle de vie du système et des processus.

## Vue d'ensemble

Les appels système de contrôle permettent de :
- Arrêter complètement le système Nachos
- Terminer proprement un processus utilisateur
- Retourner des codes de statut

## Liste des appels système

### [Halt](./Halt.md)

Arrête immédiatement tout le système Nachos et affiche les statistiques de performance.

**Utilisation** :
```c
void Halt(void);
```

**Cas d'usage** :
- Terminaison globale du système
- Fin de programmes de test
- Affichage des statistiques système

### [Exit](./Exit.md)

Termine proprement le processus courant en attendant la fin de tous ses threads.

**Utilisation** :
```c
void Exit(int status);
```

**Cas d'usage** :
- Terminaison normale d'un processus
- Gestion d'erreurs avec codes de retour
- Synchronisation avec processus parent

## Différences principales

| Caractéristique | Halt | Exit |
|----------------|------|------|
| **Portée** | Système entier | Processus courant |
| **Autres processus** | Terminés | Non affectés |
| **Threads du processus** | Terminés immédiatement | Attendus avant terminaison |
| **Code de retour** | Aucun | Retourné au parent |
| **Statistiques** | Affichées | Non affichées |
| **Nettoyage** | Minimal | Complet |

## Conventions de codes de retour

Les codes de retour suivent généralement ces conventions :

- **0** : Succès, exécution normale
- **1-255** : Erreurs diverses (définies par l'application)

Exemples courants :
```c
#define EXIT_SUCCESS  0
#define EXIT_FAILURE  1
#define EXIT_INVALID_ARG  2
#define EXIT_IO_ERROR     3
```

## Hiérarchie de terminaison

Nachos offre plusieurs niveaux de terminaison :

1. **Thread** : `PthreadExit(retval)` - Termine le thread courant
2. **Processus** : `Exit(status)` - Termine le processus et tous ses threads
3. **Système** : `Halt()` - Termine tous les processus et le système

## Exemples

### Terminaison conditionnelle

```c
#include "syscall.h"

int main() {
    int value;
    int result = GetInt(&value);

    if (result != 0) {
        PutString("Input error!\n", 13);
        Exit(1);  // Terminaison avec erreur
    }

    if (value < 0) {
        PutString("Negative value not allowed\n", 27);
        Exit(2);  // Code d'erreur spécifique
    }

    PutString("Processing value: ", 18);
    PutInt(value);
    PutChar('\n');

    Exit(0);  // Succès
}
```

### Dernier processus déclenche Halt

```c
#include "syscall.h"

int main() {
    PutString("This is the only process.\n", 26);
    PutString("Exit will trigger Halt!\n", 24);

    Exit(0);  // Si dernier processus, appelle Halt()

    // Équivalent à Halt() dans ce cas
}
```

## Gestion d'erreur

Les appels système de contrôle ne peuvent pas échouer :
- Ils ne retournent jamais au code appelant
- Aucun code d'erreur n'est généré
- `errno` n'est pas modifié

## Thread-safety

Les deux appels système sont thread-safe :
- N'importe quel thread peut appeler `Halt` ou `Exit`
- Les appels concurrents sont gérés correctement
- Le premier appel déclenche la terminaison

## Voir aussi

- [PthreadExit](../threads/PthreadExit.md) - Terminer un thread
- [Join](../process/Join.md) - Récupérer le code de retour d'un processus
- [ForkExec](../process/ForkExec.md) - Créer un processus enfant

## Notes d'implémentation

### Architecture

Les appels système de contrôle sont implémentés dans :
- `code/userprog/exception.cc` - Handlers des syscalls
- `code/threads/interrupt.cc` - Logique d'arrêt système
- `code/userprog/process.cc` - Gestion des processus

### Convention d'appel

Registres utilisés :
- **$2** : Numéro de syscall, puis valeur de retour
- **$4** : Premier argument (status pour Exit)
- **PC** : Incrémenté après syscall (sauf Halt/Exit qui ne retournent pas)

### Différences POSIX

Nachos simplifie le modèle POSIX :
- Pas de signaux (SIGTERM, SIGKILL, etc.)
- Pas de handlers atexit()
- Pas de groupes de processus
- Code de retour limité à un entier

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
