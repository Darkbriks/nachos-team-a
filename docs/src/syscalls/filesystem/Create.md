# Create

`Create` - Crée un nouveau fichier

## Synopsis

```c
#include "syscall.h"

int Create(char *name, int size);
```

## Description

`Create` crée un nouveau fichier vide avec le nom spécifié dans le système de fichiers NachOS.

**Numéro d'appel système** : `SC_Create` (37)

### Comportement nominal

1. Validation de l'adresse et de la taille du nom
2. Copie du nom depuis l'espace utilisateur
3. Création du fichier via `fileSystem->Create()`
4. Retour de 0 en cas de succès

## Paramètres

### `name`

Chemin du fichier à créer.

**Type** : `char *`
**Direction** : IN
**Registre** : `$4`
**Contraintes** : Doit pointer vers une chaîne valide, taille max `MAX_PATH_SIZE`

### `size`

Taille de la chaîne `name`.

**Type** : `int`
**Direction** : IN
**Registre** : `$5`

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 14 | `E_FAULT` | Adresse `name` invalide |
| 28 | `E_FULL_DISK` | Disque plein ou création impossible |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userFile.cc:handle_SC_Create()`

### Flux d'exécution

```
Create(name, size)
        │
        ▼
    handle_SC_Create()
        │ ├─ valide l'adresse
        │ ├─ copie le nom depuis user space
        │ └─ fileSystem->Create(name, 0)
        ▼
    [retourne 0 ou -1]
```

## Exemple

```c
#include "syscall.h"

int main() {
    char *filename = "myfile.txt";

    if (Create(filename, 11) == 0) {
        PutString("Fichier cree\n", 13);
    } else {
        PutString("Erreur creation\n", 16);
    }

    return 0;
}
```

## Voir aussi

- [Open](./Open.md) - Ouvrir un fichier
- [Close](./Close.md) - Fermer un fichier

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
