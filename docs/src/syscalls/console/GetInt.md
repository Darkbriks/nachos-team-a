# GetInt

`GetInt` - Lit un entier depuis la console d'entrée standard

## SYNOPSIS
```c
#include "syscall.h"

int GetInt(int *n);
```

## DESCRIPTION

`GetInt` lit une ligne de texte depuis la console, la convertit en entier, et stocke le résultat à l'adresse pointée par `n`. La fonction attend que l'utilisateur tape Enter après avoir saisi un nombre.

Numéro d'appel système : `16`

### Comportement nominal

- Lit jusqu'à 11 caractères (taille d'un int 32-bit)
- Utilise `SynchGetString` puis `sscanf` pour la conversion
- Accepte les nombres négatifs (préfixe `-`)
- Retourne 0 en cas de succès, -1 en cas d'erreur
- Bloque jusqu'à ce que Enter soit pressé

### Cas particuliers

- **Entrée vide** : Erreur, retourne -1
- **Entrée non-numérique** : Erreur (`errno = E_INVAL`)
- **Overflow** : Comportement dépend de `sscanf` (typiquement INT_MAX/MIN)
- **Plusieurs nombres** : Seul le premier est pris (`"42 56"` → 42)

## PARAMÈTRES

### `n`
Pointeur vers l'entier où stocker le résultat.

**Type** : `int *`  
**Direction** : OUT  
**Registre** : `$4`
**Contraintes** :
- Doit pointer vers mémoire accessible en écriture
- Doit être aloué par le programme utilisateur
- **NON VÉRIFIÉ** : Peut être NULL
- **NON VÉRIFIÉ** : Peut être dans l'espace noyau

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : `0` et `*n` contient la valeur lue

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno  | Constante | Condition                       |
|--------|-----------|---------------------------------|
| 1      | `E_INVAL` | Format invalide (non numérique) |
| 2      | `E_FAULT` | `addr < 0`                      |

## IMPLÉMENTATION

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/exception.cc:handler_SC_GetInt()`
- **Implémentation** : Utilise `SynchConsole::SynchGetString()` + `sscanf`

### Synchronisation

Hérite de `SynchGetString` :
- `IO_Lock` pour exclusivité
- `readAvail` pour attendre input utilisateur
- Bloquant jusqu'à newline

### Thread-safety

Hérite de `SynchGetString` :
- **Garanti au niveau caractère**
- **Pas garanti au niveau nombre** : Entrées entrelacées possibles

## DÉCISIONS DE CONCEPTION

### Choix 1 : Buffer de 12 octets

**Problème** : Quelle taille pour lire la chaîne ?

**Solution retenue** : 12 octets

**Justification** :
- INT_MIN = -2147483648 : 11 caractères + `\0` = 12 octets
- Cohérent avec `PutInt`

## EXEMPLES

### Exemple 1 : Utilisation basique

```c
#include "syscall.h"

int main() {
    int number;
    
    PutString("Enter a number: ", 16);
    GetInt(&number);
    
    PutString("You entered: ", 13);
    PutInt(number);
    PutChar('\n');
    
    return 0;
}
```

**Interaction** :
```
Enter a number: 42
You entered: 42
```

### Exemple 2 : Gestion d'erreur

```c
#include "syscall.h"
#include "my_stdlib.h"

int main() {
    int number;
    int result;
    
    PutString("Enter a number: ", 16);
    result = GetInt(&number);
    
    if (result == -1) {
        print_error("Invalid input");
        // errno contient E_INVAL
    } else {
        PutString("Valid: ", 7);
        PutInt(number);
        PutChar('\n');
    }
    
    return 0;
}
```

**Interaction** :
```
Enter a number: abc
Invalid input
```

### Exemple 3 : Boucle de validation

```c
#include "syscall.h"

int main() {
    int number;
    int valid = 0;
    
    while (!valid) {
        PutString("Enter a positive number: ", 25);
        
        if (GetInt(&number) == 0 && number > 0) {
            valid = 1;
        } else {
            PutString("Invalid, try again.\n", 20);
        }
    }
    
    PutString("Thank you! You entered: ", 24);
    PutInt(number);
    PutChar('\n');
    
    return 0;
}
```

### Exemple 4 : Calcul

```c
#include "syscall.h"

int main() {
    int a, b, sum;
    
    PutString("Enter first number: ", 20);
    GetInt(&a);
    
    PutString("Enter second number: ", 21);
    GetInt(&b);
    
    sum = a + b;
    
    PutInt(a);
    PutString(" + ", 3);
    PutInt(b);
    PutString(" = ", 3);
    PutInt(sum);
    PutChar('\n');
    
    return 0;
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Adresse où écrire l'entier
- Mémoire à `addr` : Contenu indéfini

**Pendant l'appel** :
- Buffer de 12 octets alloué sur stack
- Lecture de la ligne (bloquant)
- Conversion string → int

**Après l'appel** :
- `$v0` : 0 (succès) ou -1 (erreur)
- `*n` : Contient la valeur lue (si succès)
- `errno` : 0 ou code d'erreur

## PERFORMANCES

### Complexité

- **Temporelle** : O(k) où k = longueur de l'entrée utilisateur (max 12)
- **Spatiale** : O(1) - buffer fixe de 12 octets

## NOTES

- **Format décimal uniquement** : Pas de support hexa/octal
- **Overflow** : Comportement de `sscanf` (wraparound ou INT_MAX/MIN)

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-critical">

#### Aucune validation de l'espace d'adressage

**Description** : Seul `addr < 0` est vérifié.

**Impact** :
- **Écriture arbitraire en mémoire noyau** : Corruption critique
- **Ascension de privilèges** : Écraser des pointeurs/structures noyau

**Exploitation** :
```c
// Écrire dans la mémoire noyau
int *kernel_ptr = (int *)0x00000000;
GetInt(kernel_ptr);
// L'utilisateur contrôle maintenant la valeur en mémoire noyau
```

</div>

<div class="vulnerability-section severity-critical">

#### WriteMem peut échouer silencieusement

**Description** : `machine->WriteMem(addr, 4, *new_val)` peut échouer si la page n'est pas mappée ou accessible en écriture.

**Impact** :
- **Écriture ignorée**
- **Pas de notification d'erreur**

**Code problématique** :
```c
machine->WriteMem(addr, 4, *new_val);  // Potentielle échec ignoré
delete new_val;
RETURN(0);  // Succès même si WriteMem a échoué !
```

</div>

<div class="vulnerability-section severity-critical">

#### Buffer overflow potentiel sur SynchGetString

**Description** : Si `SynchGetString` écrit plus de 12 octets (impossible normalement mais...), stack overflow.

**Impact** : Corruption de stack du noyau

**Correction** : `SynchGetString` limite à 12 mais vérifier quand même

</div>

<div class="vulnerability-section severity-info">

#### Pas de timeout

**Description** : Bloque indéfiniment en attendant l'utilisateur.

**Impact** : Pas de mécanisme de timeout programmatique

</div>

## BUGS CONNUS

Aucun bug connu à ce jour.

## Historique des versions

- **v1.0** : Implémentation initiale
- **v2.0** : Ajout de la valeur de retour et gestion d'erreurs basique
- **v2.1** : Fix allocation inutile

## VOIR AUSSI

- [PutInt](./PutInt.md) - Affichage d'un entier
- [GetString](./GetString.md) - Lecture de chaîne
- [GetChar](./GetChar.md) - Lecture d'un caractère

## AUTEURS

Antoine, 20 Dec 2025

## DERNIÈRE RÉVISION

20 Dec 2025 par Antoine