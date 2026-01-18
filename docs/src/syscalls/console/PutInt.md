# PutInt

`PutInt` - Écrit un entier dans la console de sortie standard

## SYNOPSIS
```c
#include "syscall.h"

int PutInt(int n);
```

## DESCRIPTION

`PutInt` convertit l'entier `n` en représentation décimale ASCII et l'affiche sur la console de sortie standard.

### Comportement nominal

- Convertit l'entier en chaîne décimale
- Affiche le signe `-` pour les nombres négatifs
- Pas de formatage spécial (pas de padding, pas de séparateurs)
- Utilise `snprintf` puis `SynchPutString` en interne
- Thread-safe via `IO_Lock`
- Retourne 0 en cas de succès, -1 en cas d'erreur

### Cas particuliers

- **Nombres négatifs** : Préfixés par `-`

## PARAMÈTRES

### `n`
L'entier à afficher.

**Type** : `int` (32-bit signé)  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** :
- Aucune contrainte sur la valeur
- Plage : -2147483648 à 2147483647

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : `0`

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

Auncun code d'erreur spécifique défini. Hérite des erreurs de `SynchPutString` qui est utilisé en interne.

## IMPLÉMENTATION

### Synchronisation

Hérite de la synchronisation de `SynchPutString` :
- `IO_Lock` pour exclusivité
- Bloquant jusqu'à affichage complet

### Thread-safety

**Garanti au niveau caractère** via `SynchPutString`

**Pas garanti au niveau nombre** : Deux threads peuvent entrelacer leurs chiffres

## DÉCISIONS DE CONCEPTION

### Choix 1 : Buffer de 12 octets

**Problème** : Quelle taille de buffer pour la conversion ?

**Solution retenue** : 12 octets

**Justification** :
- INT_MIN = -2147483648 : 11 caractères + `\0` = 12 octets
- Suffisant pour tous les int 32-bit signés
- Stack allocation (pas de malloc)

## EXEMPLES

### Exemple 1 : Affichage de divers entiers

```c
#include "syscall.h"

int main() {
    PutInt(42);
    PutChar('\n');

    PutInt(-12345);
    PutChar('\n');
    
    PutInt(0);
    PutChar('\n');
    
    PutInt(2147483647);  // INT_MAX
    PutChar('\n');
    
    return 0;
}
```

**Sortie** :
```
42
-12345
0
2147483647
```

### Exemple 2 : Calcul et affichage

```c
#include "syscall.h"

int main() {
    int a = 10, b = 20;
    int sum = a + b;
    
    PutInt(a);
    PutString(" + ", 3);
    PutInt(b);
    PutString(" = ", 3);
    PutInt(sum);
    PutChar('\n');
    
    return 0;
}
```

**Sortie** :
```
10 + 20 = 30
```

### États de la machine

**Avant l'appel** :
- `$4` : Valeur de l'entier à afficher

**Pendant l'appel** :
- Stack : buffer de 12 octets alloué
- Conversion int → string
- Affichage via console

**Après l'appel** :
- `$2` : 0 (si succès) ou -1 (si erreur)
- `errno` : défini si erreur (erreur de SynchPutString)

## PERFORMANCES

### Complexité

- **Temporelle** : O(log₁₀(n)) pour conversion + O(d) pour affichage (d = nombre de chiffres)
- **Spatiale** : O(1) - buffer fixe de 12 octets

## NOTES

- **Décimal uniquement** : Pas de support hexa/octal
- **Pas de padding** : Pas d'espaces ou zéros de remplissage
- **Thread-safe partiel** : Caractères atomiques mais pas les nombres
- **Dépend de snprintf** : Assume que la libc est linkée au noyau

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-info">

#### Dépendance à snprintf

**Description** : Utilise `snprintf` de la libc standard.

**Impact** :
- Fonctionne seulement si libc est linkée au noyau
- Comportement dépend de l'implémentation libc

**Alternative** : Implémenter une conversion manuelle int→string

</div>

## BUGS CONNUS

Aucun bug connu.

## Historique des versions

- **v1.0** : Implémentation initiale
- **v1.1** : Ajout du retour explicite manquant

## VOIR AUSSI

- [GetInt](./GetInt.md) - Lecture d'un entier
- [PutString](./PutString.md) - Affichage de chaîne
- [PutChar](./PutChar.md) - Affichage d'un caractère

## Auteurs

Antoine

## Dernière révision

18 Jan 2026