# Open

`Open` - Ouvre un fichier existant

## Synopsis

```c
#include "syscall.h"

OpenFileId Open(char *name, int size);
```

## Description

`Open` ouvre un fichier existant et retourne un descripteur de fichier.

**Numéro d'appel système** : `SC_Open` (38)

### Comportement nominal

1. Validation de l'adresse et copie du nom
2. Vérification que la table des fichiers n'est pas pleine
3. Ouverture du fichier via `fileSystem->Open()`
4. Ajout à la table des fichiers ouverts du thread
5. Retour du descripteur de fichier

## Paramètres

### `name`

Chemin du fichier à ouvrir.

**Type** : `char *`
**Direction** : IN
**Registre** : `$4`

### `size`

Taille de la chaîne `name`.

**Type** : `int`
**Direction** : IN
**Registre** : `$5`

## Valeur de retour

**Type** : `OpenFileId` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `>= 0` | Descripteur de fichier valide |
| `< 0` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 14 | `E_FAULT` | Adresse invalide |
| 2 | `E_NOENT` | Fichier non trouvé |
| 24 | `E_FTABLE` | Table des fichiers pleine |

## Exemple

```c
#include "syscall.h"

int main() {
    OpenFileId fd = Open("myfile.txt", 11);
    if (fd >= 0) {
        // Utiliser le fichier...
        Close(fd);
    }
    return 0;
}
```

## Voir aussi

- [Create](./Create.md) - Créer un fichier
- [Close](./Close.md) - Fermer un fichier
- [Read](./Read.md) - Lire depuis un fichier

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
