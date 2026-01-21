# Read

`Read` - Lit des données depuis un fichier

## Synopsis

```c
#include "syscall.h"

int Read(char *buffer, int size, OpenFileId id);
```

## Description

`Read` lit jusqu'à `size` octets depuis le fichier ouvert dans le buffer.

**Numéro d'appel système** : `SC_Read` (39)

### Comportement nominal

1. Validation du descripteur et du buffer
2. Lecture depuis la position courante (seek)
3. Mise à jour de la position de lecture
4. Retour du nombre d'octets lus

## Paramètres

### `buffer`

Buffer pour stocker les données lues.

**Type** : `char *`
**Direction** : OUT
**Registre** : `$4`

### `size`

Nombre maximum d'octets à lire.

**Type** : `int`
**Direction** : IN
**Registre** : `$5`

### `id`

Descripteur de fichier.

**Type** : `OpenFileId`
**Direction** : IN
**Registre** : `$6`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `> 0` | Nombre d'octets lus |
| `0` | Fin de fichier (EOF) |
| `< 0` | Erreur |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 9 | `E_BADF` | Descripteur invalide |
| 14 | `E_FAULT` | Buffer invalide |
| 1 | `E_INVAL` | Taille négative |
| 3 | `E_OVERFLOW` | Débordement d'adresse |

## Exemple

```c
#include "syscall.h"

int main() {
    char buffer[100];
    OpenFileId fd = Open("data.txt", 8);

    int n = Read(buffer, 100, fd);
    if (n > 0) {
        PutString("Lu: ", 4);
        PutInt(n);
        PutString(" octets\n", 8);
    }

    Close(fd);
    return 0;
}
```

## Voir aussi

- [Write](./Write.md) - Écrire dans un fichier
- [Seek](./Seek.md) - Changer la position de lecture

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
