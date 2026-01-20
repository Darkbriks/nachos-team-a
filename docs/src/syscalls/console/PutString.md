# PutString

`PutString` - Écrit une chaîne de caractères dans la console de sortie standard

## SYNOPSIS
```c
#include "syscall.h"

int PutString(char *s, int n);
```

## DESCRIPTION

`PutString` écrit jusqu'à `n` octets de la chaîne pointée par `s` vers la console de sortie standard. L'écriture s'arrête au premier caractère null (`\0`) rencontré ou après `n` octets, selon ce qui arrive en premier.

### Comportement nominal

- Écrit jusqu'à `n` octets depuis `s`
- S'arrête au premier `\0` (non compté dans le retour)
- Retourne le nombre d'octets effectivement écrits
- Bloque jusqu'à ce que tous les caractères soient affichés
- Thread-safe via `IO_Lock`
- Limite maximale : `MAX_PUT_STRING` (8192 octets)

### Cas particuliers

- **n = 0** : Retourne immédiatement 0
- **n < 0** : Retourne -1, `errno = E_INVAL`
- **n > MAX_PUT_STRING** : Tronqué à `MAX_PUT_STRING`
- **Pointeur NULL** : Comportement indéfini
- **Overflow arithmétique** : Retourne -1, `errno = E_OVERFLOW`

## PARAMÈTRES

### `s`
Pointeur vers la chaîne de caractères à écrire.

**Type** : `char *`  
**Direction** : IN  
**Registre** : `$4` (adresse virtuelle)  
**Contraintes** :
- Doit pointer vers mémoire accessible en lecture
- **NON VÉRIFIÉ** : Peut être dans l'espace noyau
- **NON VÉRIFIÉ** : Peut être NULL

### `n`
Nombre maximum d'octets à écrire (`\0` est exclu).

**Type** : `int`  
**Direction** : IN  
**Registre** : `$5`  
**Contraintes** :
- Doit être ≥ 0
- Si > `MAX_PUT_STRING`, tronqué automatiquement
- Ne compte pas le `\0` terminal

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : Nombre d'octets écrits (0 à n), hors `\0`

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante    | Condition              |
|-------|--------------|------------------------|
| 1     | `E_INVAL`    | `n < 0`                |
| 2     | `E_FAULT`    | `addr < 0`             |
| 3     | `E_OVERFLOW` | `n > INT32_MAX - addr` |

## IMPLÉMENTATION

### Synchronisation

**Sémaphore `IO_Lock`**
- Acquis par chaque appel à `SynchPutChar` dans `SynchPutString`
- Libéré après chaque caractère affiché
- Pas de garantie d'atomicité pour la chaîne complète

### Thread-safety

**Garanti au niveau caractère** : Pas d'entrelacement de caractères individuels

**NON garanti au niveau chaîne** : Deux threads peuvent entrelacer leurs chaînes :
```
Thread 1: PutString("AAAA", 4)
Thread 2: PutString("BBBB", 4)
Résultat possible: "AABBAABB"
```

## DÉCISIONS DE CONCEPTION

*TODO : A compléter (MAX_PUT_STRING, Taille de buffer intermédiaire, Arrêt au \0)*

## EXEMPLES

### Exemple 1 : Utilisation basique

```c
#include "syscall.h"

int main() {
    int n = PutString("Hello, World!\n", 14);
    // n = 14 (tous les caractères écrits)
    return 0;
}
```

### Exemple 2 : Arrêt au `\0`

```c
#include "syscall.h"

int main() {
    char msg[] = "Hello\0 World";
    int n = PutString(msg, 100);
    // n = 5 (s'arrête au \0)
    // Affiche: "Hello"
    return 0;
}
```

### Exemple 3 : Gestion d'erreur

```c
#include "syscall.h"
#include "my_stdlib.h"

int main() {
    char msg[] = "Test";
    int n = PutString(msg, -10);  // Taille négative
    // n = -1
    
    if (n == -1) {
        // errno contient E_INVAL
    }
    
    return 0;
}
```

### Exemple 4 : Chaîne longue

```c
#include "syscall.h"

int main() {
    char long_msg[10000];
    // Remplir long_msg...
    
    // Sera tronqué à MAX_PUT_STRING (8192)
    int n = PutString(long_msg, 10000);
    // n = 8192
    
    return 0;
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Adresse de la chaîne en mémoire utilisateur
- `$5` : Nombre d'octets maximum
- Mémoire utilisateur : Contient la chaîne

**Pendant l'appel** :
- Buffer noyau alloué sur stack (256 octets)
- Boucle de copie + affichage
- Thread peut être bloqué sur `IO_Lock`

**Après l'appel** :
- `$v0` : Nombre d'octets écrits ou -1
- `errno` : Code d'erreur ou 0

## PERFORMANCES

### Complexité

- **Temporelle** : O(n) où n = min(longueur_chaîne, n_param, MAX_PUT_STRING)
- **Spatiale** : O(1) - buffer fixe de 256 octets

## NOTES

- **Buffer par blocs** : Réduit les appels à `copyStringFromMachine`
- **Limite de 8KB** : Applications doivent gérer les chaînes plus longues
- **Thread-safety partielle** : Caractères atomiques mais pas les chaînes complètes

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-info">

**Description** : Un programme peut monopoliser la console indéfiniment.

**Impact** : DoS par spam de PutString en boucle

**Exploitation** :
```c
while (1) {
    PutString("SPAM", 4);
}
```

</div>

## BUGS CONNUS

Aucun bug connu à ce jour.

## Historique des versions

- **v1.0** : Implémentation initiale
- **v2.0** : Ajout du paramètre `n`, déplacement vers un handler dédié
- **v3.0** : Ajout de la valeur de retour
- **v3.1** : Ajout de la gestion des quelques erreurs avec `errno`
- **v3.2** : Correction du retour en cas d'erreur partielle (retourne le nombre d'octets écrits avant que l'erreur ne survienne)

## VOIR AUSSI

- [PutChar](./PutChar.md) - Affichage d'un caractère
- [PutInt](./PutInt.md) - Affichage d'un entier
- [GetString](./GetString.md) - Lecture de chaîne

## Auteurs

Antoine

## Dernière révision

18 Jan 2026