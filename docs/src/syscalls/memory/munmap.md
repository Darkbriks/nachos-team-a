# munmap

## Synopsis

```c
#include "syscall.h"

int munmap(void* addr);
```

## Description

`munmap` (memory unmap) libère une zone de mémoire précédemment allouée par `mmap`.

<div class="callout callout-warning">
    <div class="callout-title">Implémentation simplifiée</div>
    <div class="callout-content">
        <strong>IMPORTANT</strong> : Cette implémentation de <code>munmap</code> est simplifiée
        et <strong>non conforme</strong> au véritable syscall Linux. Elle fait partie de
        l'implémentation partielle de <code>mmap</code> et ne gère que la libération de zones
        allouées dans la région de stack via <code>StackManager</code>.
        <br><br>
        Limitations principales :
        <ul>
            <li>Fonctionne uniquement pour les zones allouées par <code>mmap</code></li>
            <li>Pas de support de libération partielle</li>
            <li>Pas de gestion de taille (déduite de l'adresse)</li>
        </ul>
    </div>
</div>

### Comportement actuel

Dans NachOS, `munmap` effectue les opérations suivantes :

1. Reçoit l'adresse de la zone à libérer (doit correspondre à une allocation `mmap`)
2. Appelle `StackManager::FreeStack(addr)` pour libérer la zone
3. Retourne 0 en cas de succès, -1 en cas d'erreur

## Paramètres

### `addr`

Adresse de début de la zone à libérer.

**Type** : `void*` (registre `$4`)  
**Direction** : IN  
**Contraintes** :
- Doit correspondre exactement à une adresse retournée par `mmap`
- Ne peut pas libérer une partie d'une zone allouée

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification                   |
|--------|---------------------------------|
| `0`    | Succès, zone libérée            |
| `-1`   | Erreur (consulter `errno`)      |

## Erreurs

| errno     | Condition                                               |
|-----------|---------------------------------------------------------|
| `E_FAULT` | Processus ou `AddrSpace` invalide                       |
| `E_INVAL` | L'adresse ne correspond pas à une allocation `mmap`     |
| `E_INVAL` | La zone a déjà été libérée (double free)                |

## Détails d'implémentation

### Libération dans StackManager

Le `StackManager` :
1. Recherche la zone correspondant à l'adresse fournie
2. Vérifie que la zone existe et n'a pas déjà été libérée
3. Libère les frames physiques associées
4. Marque la zone comme libre dans sa liste interne

## Limitations

<div class="callout callout-warning">
    <div class="callout-title">Implémentation incomplète</div>
    <div class="callout-content">
        <ul>
            <li><strong>Pas de libération partielle</strong> : impossible de libérer une partie d'une zone</li>
            <li><strong>Pas de paramètre de taille</strong> : la taille est déduite de l'allocation initiale</li>
            <li><strong>Adresse exacte requise</strong> : doit correspondre exactement à l'adresse retournée par <code>mmap</code></li>
            <li><strong>Pas de vérification robuste</strong> : erreurs possibles si l'adresse est invalide</li>
        </ul>
    </div>
</div>

<div class="callout callout-danger">
    <div class="callout-title">Double free</div>
    <div class="callout-content">
        Appeler <code>munmap</code> deux fois sur la même adresse provoque une erreur.
        Assurez-vous de ne libérer chaque zone qu'une seule fois.
        <br><br>
        Bonne pratique : définir le pointeur à NULL après libération :
        <pre><code>munmap(zone);
zone = NULL;  // Évite les double free accidentels</code></pre>
    </div>
</div>

## Thread-safety

`munmap` est **thread-safe**. Les libérations sont protégées par le `StackManager` qui gère les accès concurrents.

Cependant, libérer une zone qu'un autre thread est en train d'utiliser provoque un **comportement indéfini**. C'est à l'utilisateur de synchroniser l'accès aux zones partagées.

## Fuites mémoire

Oublier d'appeler `munmap` provoque des **fuites mémoire** :

```c
void leak_example() {
    void* zone = (void*)mmap(NULL, 1024);
    // Utilisation...
    // OUBLI de munmap(zone) !
}  // La zone reste allouée et devient inaccessible
```

Après plusieurs appels, la mémoire disponible s'épuise et `mmap` échoue.

## Voir aussi

- [mmap](./mmap.md) - Allouer une zone mémoire
- [Memory Overview](./Memory.md) - Vue d'ensemble de la gestion mémoire
- [Sbrk](./Sbrk.md) - Extension du heap
- [Threads](../threads/Threads.md) - Libération des stacks de threads
- [errno](../../libs/errno.md) - Gestion des erreurs

## Auteurs

Antoine

## Dernière révision

21 Jan 2026