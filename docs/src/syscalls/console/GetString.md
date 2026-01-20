# GetString

`GetString` - Lit une chaîne de caractères depuis la console d'entrée standard

## SYNOPSIS
```c
#include "syscall.h"

int GetString(char *s, int n);
```

## DESCRIPTION

`GetString` lit jusqu'à `n-1` caractères depuis la console d'entrée et les stocke dans le buffer pointé par `s`. La lecture s'arrête lorsqu'un caractère newline (`\n`) est rencontré ou après `n-1` caractères. Un caractère null (`\0`) est toujours ajouté à la fin, ce qui justifie la lecture de `n-1` caractères maximum.

### Comportement nominal

- Lit jusqu'à `n-1` caractères (réserve 1 octet pour `\0`)
- S'arrête au premier `\n` (inclus dans la chaîne)
- Ajoute automatiquement `\0` terminal
- Retourne le nombre de caractères lus (hors `\0`)
- Bloque jusqu'à ce que `\n` soit tapé ou `n-1` caractères lus
- Thread-safe via `IO_Lock`

### Cas particuliers

- **n = 0** : Retourne 0, aucune lecture effectuée
- **n < 0** : Retourne -1, `errno = E_INVAL`
- **n = 1** : Lit 0 caractère, écrit seulement `\0`
- **n > MAX_STRING_SIZE** : Tronqué à `MAX_STRING_SIZE` (256)
- **EOF** : Retourne nombre de caractères lus jusqu'à EOF
- **Pointeur NULL** : Comportement indéfini (pas de vérification)

## PARAMÈTRES

### `s`
Pointeur vers le buffer où stocker la chaîne lue.

**Type** : `char *`  
**Direction** : OUT  
**Registre** : `$4` (adresse virtuelle)  
**Contraintes** :
- Doit pointer vers mémoire accessible en écriture
- Doit être alloué par l'appelant
- Doit avoir au moins `n` octets d'espace
- **NON VÉRIFIÉ** : Peut être dans l'espace noyau
- **NON VÉRIFIÉ** : Peut être NULL

**Résponsabilité de l'appelant** : S'assurer que `s` est valide et suffisamment grand, sinon, des données adjacentes peuvent être corrompues.

### `n`
Taille du buffer (incluant l'espace pour `\0`).

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`  
**Contraintes** :
- Doit être ≥ 0
- Si > `MAX_STRING_SIZE`, tronqué automatiquement
- Au plus n-1 caractères lus

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : Nombre de caractères lus (hors `\0`), peut être 0

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante  | Condition  |
|-------|------------|------------|
| 1     | `E_INVAL`  | `n < 0`    |
| 2     | `E_FAULT`  | `addr < 0` |

## IMPLÉMENTATION

### Synchronisation

**Sémaphore `IO_Lock`**
- Acquis par `SynchGetChar` pour chaque caractère
- Libéré après chaque lecture
- Garantit atomicité de la lecture de chaque caractère, mais pas de la chaîne complète

**Sémaphore `readAvail`**
- P() pour chaque caractère attendu
- V() lorsqu'un caractère est disponible

### Thread-safety

**Caractères** : Pas de corruption de caractères individuels

**Chaînes** : Deux threads peuvent recevoir des caractères entrelacés :
```
User types: "ABCD\n"
Thread 1: GetString(buf1, 10)
Thread 2: GetString(buf2, 10)

Résultat possible:
buf1 = "AC\n"
buf2 = "BD\n"
```

## DÉCISIONS DE CONCEPTION

*TODO : A compléter (Inclure `\n`, taille max, arrêt au `\n`)*

## EXEMPLES

### Exemple 1 : Utilisation basique

```c
#include "syscall.h"

int main() {
    char buffer[100];
    
    PutString("Enter your name: ", 17);
    int n = GetString(buffer, 100);
    
    PutString("Hello, ", 7);
    PutString(buffer, n);
    
    return 0;
}
```

**Interaction** :
```
Enter your name: Bjarn Stroustrup
Hello, Bjarn Stroustrup
```

### Exemple 2 : Buffer petit

```c
#include "syscall.h"

int main() {
    char buffer[5];  // Seulement 4 chars + \0
    
    PutString("Type: ", 6);
    int n = GetString(buffer, 5);
    
    // Si l'utilisateur tape "Hello\n":
    // buffer = "Hell" (tronqué, pas de \n)
    // n = 4
    
    PutString("Got: ", 5);
    PutString(buffer, n);
    
    return 0;
}
```

### Exemple 3 : Gestion d'erreur

```c
#include "syscall.h"
#include "my_stdlib.h"

int main() {
    char buffer[64];
    
    int n = GetString(buffer, -10);  // Taille négative
    // n = -1
    
    if (n == -1) {
        // errno contient E_INVAL
    }
    
    return 0;
}
```

### Exemple 4 : Retirer le newline

```c
#include "syscall.h"

int main() {
    char buffer[100];
    
    int n = GetString(buffer, 100);
    
    // Retirer le \n si présent
    if (n > 0 && buffer[n-1] == '\n') {
        buffer[n-1] = '\0';
        n--;
    }
    
    PutString("Without newline: ", 17);
    PutString(buffer, n);
    
    return 0;
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Adresse buffer utilisateur
- `$5` : Taille du buffer
- Buffer peut contenir n'importe quoi

**Pendant l'appel** :
- Buffer noyau alloué sur stack (max 256 octets)
- Boucle de lecture caractère par caractère
- Thread bloqué sur chaque `SynchGetChar`

**Après l'appel** :
- `$2` : Nombre de caractères lus (ou -1)
- Buffer utilisateur : Contient la chaîne + `\0`
- `errno` : 0 ou code d'erreur

## PERFORMANCES

### Complexité

- **Temporelle** : O(n) où n = nombre de caractères lus
- **Spatiale** : O(min(n, MAX_STRING_SIZE)) - buffer sur stack

## NOTES

- **Buffer stack** : Limité à 256 octets
- **Newline inclus** : L'appel retourne le `\n` dans la chaîne mais le charactère n'est pas compté dans le retour
- **EOF** : En fin de fichier, retourne caractères lus

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-info">

#### Limitation MAX_STRING_SIZE

**Description** : Ajouter une constante MAX_GET_STRING sur le modèle de MAX_PUT_STRING, et remplir plusieurs fois le buffer si nécessaire.

**Workaround** :
```c
char big_buffer[1000];
int offset = 0;
while (offset < 999) {
    int n = GetString(big_buffer + offset, 256);
    if (big_buffer[offset + n - 1] == '\n') break;
    offset += n;
}
```

</div>

<div class="vulnerability-section severity-info">

#### Pas de timeout

**Description** : Peut bloquer indéfiniment en attendant l'utilisateur.

**Impact** : Pas de mécanisme pour timeout programmatique

</div>

## BUGS CONNUS

Aucun bug connu à ce jour.

## Historique des versions

- **v1.0** : Implémentation initiale
- **v2.0** : Ajout de la valeur de retour

## VOIR AUSSI

- [PutString](./PutString.md) - Écriture de chaîne
- [GetChar](./GetChar.md) - Lecture d'un caractère
- [GetInt](./GetInt.md) - Lecture d'un entier

## Auteurs

Antoine

## Dernière révision

18 Jan 2026