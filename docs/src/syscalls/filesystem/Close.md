# Close

`Close` - Ferme un fichier ouvert

## Synopsis

```c
#include "syscall.h"

int Close(int fd);
```

## Description

`Close` ferme un descripteur de fichier ouvert et libère les ressources associées.

**Numéro d'appel système** : `SC_Close` (41)

### Comportement nominal

1. Vérification que le descripteur est valide
2. Fermeture du fichier via `fileSystem->Close()`
3. Suppression de la table des fichiers ouverts

## Paramètres

### `fd`

Descripteur de fichier à fermer.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès |
| `< 0` | Erreur |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 9 | `E_BADF` | Descripteur invalide |
| 14 | `E_FAULT` | Erreur lors de la fermeture |

## Exemple

```c
#include "syscall.h"

int main() {
    OpenFileId fd = Open("data.txt", 8);
    if (fd >= 0) {
        // Operations sur le fichier...
        Close(fd);
    }
    return 0;
}
```

## Voir aussi

- [Open](./Open.md) - Ouvrir un fichier

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
