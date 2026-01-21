# Write

`Write` - Écrit des données dans un fichier

## Synopsis

```c
#include "syscall.h"

int Write(char *buffer, int size, OpenFileId id);
```

## Description

`Write` écrit `size` octets depuis le buffer dans le fichier ouvert.

**Numéro d'appel système** : `SC_Write` (40)

### Comportement nominal

1. Validation du descripteur et du buffer
2. Écriture depuis la position courante (seek)
3. Mise à jour de la position d'écriture
4. Retour du nombre d'octets écrits

## Paramètres

### `buffer`

Buffer contenant les données à écrire.

**Type** : `char *`
**Direction** : IN
**Registre** : `$4`

### `size`

Nombre d'octets à écrire.

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
| `> 0` | Nombre d'octets écrits |
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
    char *data = "Hello, NachOS!";
    OpenFileId fd = Open("output.txt", 10);

    int n = Write(data, 14, fd);
    PutString("Ecrit: ", 7);
    PutInt(n);
    PutString(" octets\n", 8);

    Close(fd);
    return 0;
}
```

## Voir aussi

- [Read](./Read.md) - Lire depuis un fichier
- [Seek](./Seek.md) - Changer la position d'écriture

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
