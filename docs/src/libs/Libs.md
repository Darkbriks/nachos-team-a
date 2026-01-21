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
| pthread      | `nos_pthread.h`| Threads POSIX-like               |
| ftp          | `nos_client.h` | Transfert de fichiers réseau     |
| types        | `types.h`      | Types de base et macros          |

---

## FTP (`nos_client.h`, `nos_fileserver.h`)

Bibliothèque de transfert de fichiers sur le réseau NachOS. Implémente un protocole simple avec commandes `GET`/`PUT`. Le client permet de télécharger et envoyer des fichiers avec mesure du débit. Le serveur gère les requêtes entrantes.

---

## Pthread (`nos_pthread.h`)

Implémentation POSIX-like des threads : `pthread_create`, `pthread_join`, `pthread_exit`, `pthread_detach`, `pthread_self`. Chaque thread possède sa propre pile et son TLS (Thread Local Storage).

### TLS (`nos_tls.h`)

Structure thread-local accessible via `$gp`. Contient : `errno` thread-safe, pointeur vers la structure pthread, et 32 slots de données spécifiques au thread (`tsd`).

---

## Stdlib (`nos_stdlib.h`)

Fonctions utilitaires standard : gestion mémoire (`malloc`, `free`, `calloc`, `realloc`), conversions (`atoi`, `itoa`, `strtol`), contrôle (`exit`, `abort`, `atexit`), math (`abs`, `rand`).

---

## Types (`types.h`)

Types de base pour l'espace utilisateur 32-bit : `uint8_t` à `uint64_t`, `size_t`, `ssize_t`, `ptrdiff_t`, `uintptr_t`. Macros utilitaires : `PACKED`, `ALIGNED`, `NORETURN`, `LIKELY`/`UNLIKELY`.

---

## Voir aussi

- [errno](./errno.md) - Gestion des erreurs
- [Appels système](../syscalls/Syscalls.md) - API bas niveau

## Auteurs

Antoine PATRON
Alioune Badara DIENE

## Dernière révision

21 Jan 2026
