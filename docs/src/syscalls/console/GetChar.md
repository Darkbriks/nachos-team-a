# GetChar

`GetChar` - Lit un caractère depuis la console d'entrée standard

## SYNOPSIS
```c
#include "syscall.h"

char GetChar(void);
```

## DESCRIPTION

`GetChar` lit et retourne un seul caractère depuis la console d'entrée standard (clavier). L'appel bloque jusqu'à ce qu'un caractère soit disponible.

### Comportement nominal

- L'appel bloque jusqu'à ce qu'un caractère soit tapé par l'utilisateur
- Le caractère est lu depuis le buffer interne de la console
- Les caractères de contrôle (Ctrl+C, etc.) sont passés tels quels
- L'appel est thread-safe

### Cas particuliers

- **EOF** : Retourné si le flux d'entrée est fermé (ex: redirection fichier terminée)
- **Interruption clavier** : Les interruptions ne sont pas gérées au niveau syscall
- **Buffer vide** : Thread bloqué sur sémaphore `readAvail` jusqu'à input

## PARAMÈTRES

Aucun paramètre.

## VALEUR DE RETOUR

**Type** : `char` (registre `$2`)

**Valeurs possibles** :
- Caractère lu (0x00 à 0x7F, signé)
- `EOF` (-1) si fin de flux

## CODES D'ERREUR

Aucun code d'erreur. La variable `errno` n'est jamais modifiée.

### Thread-safety

L'implementation utilise des sémaphores pour garantir un accès exclusif au buffer de la console, évitant les conditions de course lors de lectures concurrentes.

## DÉCISIONS DE CONCEPTION

*TODO : A compléter (Pourquoi bloquant synchrone, pourquoi pas d’erreur)*

## EXEMPLES

### Exemple 1 : Lecture simple

```c
#include "syscall.h"

int main() {
    char c;
    
    PutString("Press any key: ", 15);
    c = GetChar();
    
    PutString("\nYou pressed: ", 14);
    PutChar(c);
    PutChar('\n');
    
    return 0;
}
```

**Interaction** :
```
Press any key: A
You pressed: A
```

### Exemple 2 : Lecture jusqu'à newline

```c
#include "syscall.h"

int main() {
    char c;
    int count = 0;
    
    PutString("Type and press Enter:\n", 22);
    
    do {
        c = GetChar();
        PutChar(c);
        count++;
    } while (c != '\n');
    
    PutString("Characters typed: ", 18);
    PutInt(count - 1);  // -1 pour exclure '\n'
    PutChar('\n');
    
    return 0;
}
```

### Exemple 3 : Gestion EOF

```c
#include "syscall.h"

int main() {
    char c;
    
    while ((c = GetChar()) != EOF) {
        PutChar(c);
    }
    
    PutString("\nEOF reached\n", 13);
    return 0;
}
```

## COMPORTEMENT DÉTAILLÉ

### États de la machine

**Avant l'appel** :
- Thread peut être en READY ou RUNNING
- `readAvail` : 0 ou plus (selon buffer)
- `IO_Lock` : libre ou occupé

**Pendant l'appel** :
- Thread en état BLOCKED sur `readAvail`
- Attend interruption clavier
- CPU libéré pour d'autres threads

**Après l'appel** :
- `$2` : Contient le caractère lu
- Autres registres préservés
- Thread retourne en RUNNING
- `IO_Lock` libéré

## PERFORMANCES

### Complexité

- **Temporelle** : O(1) + temps d'attente utilisateur
- **Spatiale** : O(1)

## NOTES

- **Pas de buffering** : Chaque caractère nécessite un appel système
- **Efficacité** : Pour lire des chaînes, utilisez `GetString`
- **Thread-safety** : Sûr mais non recommendé, car il n'y a pas de garantie d'ordre entre threads
- **Atomicité** : Chaque appel est atomique vis-à-vis des autres appels à `GetChar`

## FAILLES ET VULNÉRABILITÉS

<div class="vulnerability-section severity-info">

#### Pas de timeout

**Description** : `GetChar` peut bloquer indéfiniment.

**Impact** : Programmes ne peuvent pas implémenter de timeout sur la lecture

</div>

## BUGS CONNUS

Aucun bug connu à ce jour.

## Historique des versions

- **v1.0** : Implémentation initiale sans synchronisation
- **v1.1** : Ajout de `IO_Lock` pour thread-safety

## VOIR AUSSI

- [GetString](./GetString.md) - Lecture de chaîne
- [GetInt](./GetInt.md) - Lecture d'un entier
- [PutChar](./PutChar.md) - Affiche un caractère

## Auteurs

Antoine

## Dernière révision

18 Jan 2026