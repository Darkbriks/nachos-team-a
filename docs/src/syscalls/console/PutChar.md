# PutChar

`PutChar` - Écrit un caractère dans la console de sortie standard

## SYNOPSIS
```c
#include "syscall.h"

void PutChar(char c);
```

## DESCRIPTION

`PutChar` écrit le caractère spécifié `c` dans la console de sortie standard. Cette opération est bloquante et attend que le caractère soit effectivement affiché avant de retourner le contrôle au programme appelant.

### Comportement nominal

- Le caractère `c` est immédiatement envoyé au périphérique de sortie.
- L'appel bloque jusqu'à ce que le périphérique confirme l'écriture.
- Aucun retour de valeur n'est fourni.
- Aucun buffer n'est utilisé, l'écriture est directe.
- L'appel est thread-safe, plusieurs threads peuvent appeler `PutChar` simultanément sans corruption de données.
- Aucun effet de bord n'est attendu autre que l'affichage du caractère.
- Les caractères spéciaux (comme `\n`, `\t`) sont traités comme des caractères ordinaires, et leur affichage dépend du comportement du terminal.

### Cas particuliers

- **Caractères NULL (`\0`)** : N'est pas visuellement représenté mais peut être envoyé.
- **Caractères de contrôle** : Leur affichage dépend du terminal (ex. `\n` provoque un saut de ligne dans la plupart des terminaux).
- **Caractères non ASCII** : Dépend du support du terminal.

## PARAMÈTRES

### `c`
Le caractère à écrire dans la console.

**Type** : `char`  
**Direction** : IN
**Registre lu** : `$4`
**Contraintes** :
- Doit être sur 8 bits signé

## VALEUR DE RETOUR

Auncune valeur de retour (`void`). Cet appel ne produit pas de code d'erreur.

## CODES D'ERREUR

Aucun code d'erreur n'est défini pour cet appel système. En cas de problème matériel, le comportement est indéfini.

## IMPLÉMENTATION

### Thread-safety

L'implémentation utilise des sémaphores pour garantir que les appels concurrents à `PutChar` ne produisent pas d'erreurs, et que chaque caractère demandé soit affiché.

## DÉCISIONS DE CONCEPTION

*TODO : A compléter (Pourquoi bloquant synchrone, pourquoi pas d'erreur)*

## EXEMPLES

### Exemple 1 : Utilisation basique
```c
#include "syscall.h"

int main() {
    PutChar('H');
    PutChar('i');
    PutChar('\n');
    return 0;
}
```

**Sortie attendue** :
```
Hi
```

### Exemple 2 : Affichage d'une chaîne manuellement
```c
#include "syscall.h"

int main() {
    const char *msg = "Hello World\n";
    int i = 0;
    
    while (msg[i] != '\0') {
        PutChar(msg[i]);
        i++;
    }
    
    return 0;
}
```

**Sortie attendue** :
```
Hello World
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- `$4` : Contient le caractère à afficher
- `IO_Lock` : Peut être libre ou occupé

**Pendant l'appel** :
- Thread bloqué sur `writeDone` si `IO_Lock` était libre
- Thread bloqué sur `IO_Lock` si un autre thread utilise la console

**Après l'appel** :
- Caractère affiché sur console
- `IO_Lock` libéré
- Registres préservés

## PERFORMANCES

### Complexité

- **Temporelle** : O(1) - temps constant + latence hardware
- **Spatiale** : O(1) - pas d'allocation mémoire

### Cas limites

- **Pire cas** : Contention élevée avec beaucoup de threads écrivant simultanément
- **Meilleur cas** : Aucune contention, `IO_Lock` immédiatement disponible

## NOTES

- **Pas de buffering** : Chaque appel à `PutChar` génère une interaction hardware.
- **Performance** : Pour afficher des chaînes, préférez `PutString` pour réduire le nombre d'appels système.
- **Thread-safety** : L'appel est conçu pour être sûr en environnement multi-thread.
- **Atomicité** : Chaque appel est atomique vis-à-vis des autres appels à `PutChar`.

## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue à ce jour.

## BUGS CONNUS

Aucun bug connu à ce jour.

## Historique des versions

- **v1.0** : Implémentation initiale sans synchronisation
- **v1.1** : Ajout de `IO_Lock` pour thread-safety

## VOIR AUSSI

- [PutString](./PutString.md) - Écrit une chaîne de caractères
- [PutInt](./PutInt.md) - Écrit un entier
- [GetChar](./GetChar.md) - Lecture d'un caractère

## Auteurs

Antoine

## Dernière révision

18 Jan 2026