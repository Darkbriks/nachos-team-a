# FileLen

`FileLen` - Retourne la taille d'un fichier

## Synopsis

```c
#include "syscall.h"

unsigned int FileLen(int fd);
```

## Description

`FileLen` retourne la taille en octets d'un fichier ouvert.

**Numéro d'appel système** : `SC_FileLen` (43)

## Paramètres

### `fd`

Descripteur de fichier.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`

## Valeur de retour

**Type** : `unsigned int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `>= 0` | Taille du fichier en octets |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 9 | `E_BADF` | Descripteur invalide |

## Exemple

```c
#include "syscall.h"

int main() {
    OpenFileId fd = Open("data.bin", 8);

    unsigned int size = FileLen(fd);
    PutString("Taille: ", 8);
    PutInt(size);
    PutString(" octets\n", 8);

    Close(fd);
    return 0;
}
```

## Voir aussi

- [Open](./Open.md) - Ouvrir un fichier
- [Seek](./Seek.md) - Se positionner dans le fichier

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
