# errno - Gestion des erreurs

Cette page documente le mécanisme de gestion des erreurs dans NachOS via la variable `errno` et les codes d'erreur associés.

## Vue d'ensemble

NachOS utilise une convention de gestion d'erreurs inspirée de POSIX :

- Les appels système qui peuvent échouer retournent `-1` en cas d'erreur
- Le code d'erreur spécifique est stocké dans `errno`
- En cas de succès, `errno` est mis à `0`

<div class="callout callout-note">
    <div class="callout-title">Thread-safety</div>
    <div class="callout-content">
        <code>errno</code> est thread-safe. Chaque thread possède sa propre copie via 
        Thread-Local Storage (TLS). Un fallback sur une variable globale est utilisé 
        avant l'initialisation du TLS (thread principal avant création de threads).
    </div>
</div>

## Utilisation

### Header

```c
#include "nos_errno.h"
```

### Convention de retour

```c
int result = SomeSystemCall(...);

if (result == -1) {
    int err = errno;  // Lire le code d'erreur
    // Gérer l'erreur
} else {
    // Succès (errno == 0)
}
```

### Macro errno

La macro `errno` donne accès à la variable d'erreur du thread courant :

```c
#define errno (*__errno_location())
```

## API

### Fonctions

| Fonction             | Description                                       |
|----------------------|---------------------------------------------------|
| `__errno_location()` | Retourne un pointeur vers errno du thread courant |
| `__set_errno(value)` | Définit errno à la valeur spécifiée               |
| `__get_errno()`      | Retourne la valeur actuelle d'errno               |
| `__clear_errno()`    | Remet errno à 0                                   |

### Signatures

```c
static inline int* __errno_location();
static inline void __set_errno(int value);
static inline int  __get_errno();
static inline void __clear_errno();
```

## Codes d'erreur

### Tableau récapitulatif

| Code | Constante        | Description                           |
|------|------------------|---------------------------------------|
| 0    | `E_SUCCESS`      | Aucune erreur                         |
| 1    | `E_INVAL`        | Argument invalide                     |
| 2    | `E_FAULT`        | Adresse mémoire invalide              |
| 3    | `E_OVERFLOW`     | Dépassement arithmétique              |
| 4    | `E_IO`           | Erreur d'entrée/sortie                |
| 5    | `E_FORMAT`       | Format invalide                       |
| 6    | `E_EOF`          | Fin de fichier                        |
| 7    | `E_NOMEM`        | Mémoire insuffisante                  |
| 8    | `E_RANGE`        | Résultat hors limites                 |
| 9    | `E_NOSPC`        | Aucun processus correspondant         |
| 10   | `E_FTABLE`       | Table d'allocation pleine             |
| 11   | `E_NOENT`        | Entrée inexistante                    |
| 12   | `E_NOCPC`        | Pas un processus enfant               |
| 13   | `E_THREAD_LIMIT` | Limite de threads atteinte            |
| 14   | `E_STACK_ADDR`   | Adresse de pile invalide              |
| 15   | `E_BUSY`         | Ressource occupée                     |
| 16   | `E_AGAIN`        | Ressource temporairement indisponible |
| 17   | `E_DOM`          | Argument mathématique hors domaine    |
| 18   | `E_ILSEQ`        | Séquence d'octets illégale            |
| 19   | `E_PERM`         | Opération non permise                 |
| 20   | `E_ACCES`        | Permission refusée                    |
| 21   | `E_EXIST`        | Le fichier existe déjà                |
| 22   | `E_NOSYS`        | Fonction non implémentée              |
| 23   | `E_NOTDIR`       | N'est pas un répertoire               |
| 24   | `E_ISDIR`        | Est un répertoire                     |
| 25   | `E_BADF`         | Mauvais descripteur de fichier        |
| 26   | `E_DEADLK`       | Deadlock détecté                      |

## Bonnes pratiques

<div class="callout callout-warning">
    <div class="callout-title">Vérifier errno immédiatement</div>
    <div class="callout-content">
        <p>Lire <code>errno</code> immédiatement après l'appel système qui a échoué.
        Un appel ultérieur pourrait l'écraser.</p>
    </div>
</div>

## Voir aussi

- [Appels système](../syscalls/Syscalls.md) - Liste des syscalls
- [Threads](../syscalls/threads/Threads.md) - Gestion des threads
- [TLS](./tls.md) - Thread-Local Storage

## Auteurs

Antoine

## Dernière révision

18 Jan 2026