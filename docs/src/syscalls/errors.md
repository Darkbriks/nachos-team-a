# Gestion des erreurs

Ce document décrit le mécanisme de gestion des erreurs dans NachOS et l'utilisation de la variable globale `errno`.

## Vue d'ensemble

NachOS utilise une convention de gestion d'erreurs inspirée de POSIX mais simplifiée :

- Les appels système qui peuvent échouer retournent `-1` en cas d'erreur
- Le code d'erreur spécifique est stocké dans la variable globale `errno`
- En cas de succès, `errno` est mis à 0

## Mécanisme d'erreur

### Convention de retour

```c
int result = SomeSystemCall(...);

if (result == -1) {
    // Erreur : consulter errno
    int error_code = GetLastError();
    // Gérer l'erreur
} else {
    // Succès : errno == 0
}
```

### Implémentation dans les stubs

Chaque stub d'appel système suit ce pattern (exemple `PutString`) :

```asm
PutString:
    addiu $2,$0,SC_PutString
    syscall
    bgez  $2,PutString_ok      # Si $v0 >= 0 : succès
    nop
    
    # Gestion d'erreur
    subu  $3,$0,$2             # $a0 = -$v0 (code positif)
    sw    $3,errno             # Stocker dans errno
    addiu $2,$0,-1             # Retourner -1
    j     $31
    nop
    
PutString_ok:
    sw    $0,errno             # errno = 0 (succès)
    j     $31
```

### Handlers noyau

Les handlers noyau retournent des codes d'erreur négatifs via la macro `RETURN()` :

```c
void handler_SC_putString() {
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    
    // Validations
    if (addr < 0) { 
        RETURN(-E_FAULT);  // Retourne -2
    }
    if (n < 0) { 
        RETURN(-E_INVAL);  // Retourne -1
    }
    
    // Traitement...
    RETURN(bytes_written);  // Retourne valeur positive
}
```

## Variable errno

### Déclaration

`errno` est une variable globale définie dans `start.S` :

```asm
.data
.align  2
.globl  errno
errno:
    .word   0    # Initialisée à 0
```

### Fonctions utilitaires

#### GetLastError

Retourne la valeur actuelle d'`errno` :

```c
int GetLastError(void);
```

**Exemple** :
```c
int result = GetString(buffer, -10);
if (result == -1) {
    int err = GetLastError();  // err = E_INVAL (1)
    PutString("Error code: ", 12);
    PutInt(err);
    PutChar('\n');
}
```

#### ClearError

Remet `errno` à 0 :

```c
void ClearError(void);
```

**Exemple** :
```c
ClearError();  // errno = 0
int err = GetLastError();  // err = 0
```

### Helper my_stdlib

La bibliothèque `my_stdlib` fournit une fonction pour afficher les erreurs :

```c
void print_error(const char *msg);
```

**Exemple** :
```c
#include "my_stdlib.h"

int main() {
    char buffer[64];
    int result = GetString(buffer, -10);
    
    if (result == -1) {
        print_error("GetString failed");
        // Affiche: "GetString failed (errno=1)\n"
    }
    
    return 0;
}
```

## Codes d'erreur

Voir [Liste des codes errno](./errno.md) pour la documentation complète de chaque code.

### Codes principaux

| Code | Constante | Description |
|------|-----------|-------------|
| 0 | E_SUCCESS | Aucune erreur |
| 1 | E_INVAL | Argument invalide |
| 2 | E_FAULT | Adresse mémoire invalide |
| 3 | E_OVERFLOW | Dépassement arithmétique |
| 4 | E_IO | Erreur d'entrée/sortie |
| 5 | E_FORMAT | Format invalide |
| 6 | E_EOF | Fin de fichier |
| 7 | E_NOMEM | Mémoire insuffisante |
| 8 | E_RANGE | Résultat hors limites |
| 9 | E_NOSPC | Aucun processus correspondant |

## Appels système avec gestion errno

### Avec gestion d'erreur

Ces appels retournent `-1` et positionnent `errno` en cas d'erreur :

### Sans gestion d'erreur

Ces appels ne modifient jamais `errno` :

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-critical">

#### errno global - Non thread-safe

**Description** : `errno` est une variable globale unique partagée par tous les threads.

**Impact** :
- **Race condition** : Plusieurs threads peuvent écraser mutuellement leur errno
- **Lecture incorrecte** : Thread A lit errno positionné par thread B
- **Debug impossible** : Impossible de tracer quelle erreur vient de quel thread

**Exploitation** :
```c
// Thread 1
int r1 = GetString(buf1, -10);  // errno = E_INVAL
// Context switch ici...
if (r1 == -1) {
    int err = GetLastError();  // Peut lire errno de Thread 2
}

// Thread 2 (simultané)
int r2 = GetInt(NULL);  // errno = E_FAULT (écrase E_INVAL)
```

**Statut** : ⚠️ **Correction prévue prochainement**

Cette limitation est connue et il est prévu de la corriger dans une future version. Pour l'instant, évitez d'utiliser errno dans du code multi-thread, ou protégez les accès avec des sémaphores.

</div>

<div class="vulnerability-section severity-warning">

#### Pas de mécanisme de protection

**Description** : Rien n'empêche un thread de modifier errno manuellement.

**Impact** : Corruption possible de l'état d'erreur

**Exploitation** :
```c
// Corrompre errno d'un autre thread
extern int errno;
errno = 42;  // Valeur invalide
```

</div>

<div class="vulnerability-section severity-warning">

#### Pas de fonction strerror

**Description** : Aucune fonction pour convertir errno en message lisible.

**Impact** : Messages d'erreur moins informatifs

</div>

## Exemples complets

### Exemple 1 : Gestion basique

```c
#include "syscall.h"
#include "my_stdlib.h"

int main() {
    char buffer[100];
    
    int result = GetString(buffer, 100);
    
    if (result == -1) {
        print_error("Failed to read string");
        Exit(1);
    }
    
    PutString("Read: ", 6);
    PutString(buffer, result);
    PutChar('\n');
    
    return 0;
}
```

### Exemple 2 : Distinction entre erreurs

```c
#include "syscall.h"
#include "my_stdlib.h"

int main() {
    int value;
    int result = GetInt(&value);
    
    if (result == -1) {
        int err = GetLastError();
        
        switch(err) {
            case E_INVAL:
                PutString("Invalid format\n", 15);
                break;
            case E_FAULT:
                PutString("Bad address\n", 12);
                break;
            default:
                PutString("Unknown error\n", 14);
        }
        
        Exit(1);
    }
    
    PutString("Got: ", 5);
    PutInt(value);
    PutChar('\n');
    
    return 0;
}
```

### Exemple 3 : Réessai sur erreur temporaire

```c
#include "syscall.h"

int main() {
    char buffer[100];
    int result;
    int attempts = 0;
    
    while (attempts < 3) {
        result = GetString(buffer, 100);
        
        if (result != -1) {
            break;  // Succès
        }
        
        int err = GetLastError();
        if (err != E_IO) {
            // Erreur non-récupérable
            Exit(1);
        }
        
        // Erreur I/O : réessayer
        attempts++;
        Yield();  // Laisser du temps
    }
    
    if (result == -1) {
        PutString("Failed after 3 attempts\n", 24);
        Exit(1);
    }
    
    return 0;
}
```

## Voir aussi

- [Liste des codes errno](./errno.md) - Documentation détaillée de chaque code
- [Console I/O Overview](console/Console.md) - Appels système avec gestion errno
- [my_stdlib](../userspace/my_stdlib.md) - Fonctions utilitaires pour errno

## Auteurs

Antoine, 20 Dec 2025

## Dernière révision

20 Dec 2025 par Antoine