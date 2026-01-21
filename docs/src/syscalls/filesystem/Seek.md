# Seek

`Seek` - Change la position de lecture/écriture

## Synopsis

```c
#include "syscall.h"

int Seek(int fd, int new_seek);
```

## Description

`Seek` déplace la position de lecture/écriture dans un fichier ouvert.

**Numéro d'appel système** : `SC_Seek` (44)

### Comportement nominal

1. Validation du descripteur
2. Appel de `fileSystem->Seek()`
3. Mise à jour de la position dans la table des fichiers

## Paramètres

### `fd`

Descripteur de fichier.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`

### `new_seek`

Nouvelle position en octets depuis le début du fichier.

**Type** : `int`
**Direction** : IN
**Registre** : `$5`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `>= 0` | Nouvelle position |
| `< 0` | Erreur |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 9 | `E_BADF` | Descripteur invalide |

## Exemple

```c
#include "syscall.h"

int main() {
    char buffer[10];
    OpenFileId fd = Open("data.txt", 8);

    // Lire les 10 premiers octets
    Read(buffer, 10, fd);

    // Revenir au debut
    Seek(fd, 0);

    // Relire
    Read(buffer, 10, fd);

    Close(fd);
    return 0;
}
```

## Voir aussi

- [Read](./Read.md) - Lire depuis un fichier
- [Write](./Write.md) - Écrire dans un fichier
- [FileLen](./FileLen.md) - Obtenir la taille du fichier

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
