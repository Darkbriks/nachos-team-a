# Sbrk

## Synopsis

```c
#include "syscall.h"

int Sbrk(int n);
```

## Description

`Sbrk` étend ou interroge le heap du processus. Le heap est la zone de mémoire dynamique utilisée pour les allocations dynamiques (malloc, calloc, etc.).

### Comportement

**Interrogation (n = 0)** :
- Retourne l'adresse actuelle du `brk` (limite du heap)
- Ne modifie pas le heap
- Permet de connaître la taille actuelle du heap

**Extension (n > 0)** :
- Alloue `n` pages supplémentaires au heap
- Retourne l'adresse du début de la nouvelle zone allouée (ancien `brk`)
- Met à jour le `brk` pour refléter la nouvelle taille
- Initialise les nouvelles pages dans la table des pages

**Réduction (n < 0)** :
- Non supporté dans l'implémentation actuelle
- Retourne `-1` avec `errno = E_INVAL`

### Granularité

L'allocation se fait par **pages** :
- Taille de page : 128 octets (constante `PageSize`)

## Paramètres

### `n`

Nombre de pages à allouer.

**Type** : `int` (registre `$4`)  
**Direction** : IN  
**Valeurs** :
- `0` : interroger le `brk` actuel sans modifier le heap
- `> 0` : allouer `n` pages supplémentaires
- `< 0` : **non supporté** (retourne erreur)

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur    | Signification                                       |
|-----------|-----------------------------------------------------|
| `>= 0`    | Adresse du début de la zone allouée (ou brk si n=0) |
| `-1`      | Erreur (consulter `errno`)                          |

## Erreurs

| errno     | Condition                                       |
|-----------|-------------------------------------------------|
| `E_INVAL` | `n` est négatif (réduction non supportée)       |
| `E_NOMEM` | Pas assez de frames physiques disponibles       |
| `E_NOMEM` | Le heap dépasserait `stackLimit` (collision)    |

## Exemples

### Exemple 1 : Interroger le brk

```c
#include "syscall.h"

int main() {
    int current_brk = Sbrk(0);
    
    PutString("Adresse brk actuelle: 0x", 24);
    PutInt(current_brk);
    PutChar('\n');
    
    return 0;
}
```

### Exemple 2 : Allouer de la mémoire

```c
#include "syscall.h"

int main() {
    // Allouer 5 pages (640 octets)
    void* new_mem = (void*)Sbrk(5);
    
    if (new_mem == (void*)-1) {
        PutString("Erreur: allocation echouee\n", 27);
        return 1;
    }
    
    PutString("Memoire allouee a: 0x", 21);
    PutInt((int)new_mem);
    PutChar('\n');
    
    // Utiliser la mémoire
    int* array = (int*)new_mem;
    for (int i = 0; i < 10; i++) {
        array[i] = i * i;
    }
    
    return 0;
}
```

## Détails d'implémentation

### Processus d'allocation

1. **Validation** : vérifier que `n >= 0`
2. **Vérification de collision** : s'assurer que `newBrk <= stackLimit`
3. **Vérification des frames** : s'assurer qu'il y a assez de frames physiques
4. **Allocation des pages** : pour chaque page de `brk` à `brk + n*PageSize` :
    - Obtenir un frame physique via `frameProvider->GetEmptyFrame()`
    - Configurer l'entrée dans la table des pages
    - Marquer la page comme valide
5. **Mise à jour du brk** : `brk += n * PageSize`
6. **Retour** : retourner l'ancien `brk`

### Limites du système

```c
#define INITIAL_HEAP_PAGES 2     // Pages allouées au démarrage
#define MAX_HEAP_PAGES 256       // Maximum de pages pour le heap
```

Le heap peut contenir au maximum 256 pages, soit 32 Ko (256 × 128 octets).

## Limitations

<div class="callout callout-warning">
    <div class="callout-title">Pas de réduction du heap</div>
    <div class="callout-content">
        <code>Sbrk</code> avec <code>n < 0</code> n'est pas supporté. Le heap ne peut que
        grandir, jamais rétrécir. Toute tentative de réduction retourne <code>-1</code>.
    </div>
</div>

<div class="callout callout-warning">
    <div class="callout-title">Granularité par page</div>
    <div class="callout-content">
        L'allocation se fait uniquement par pages entières (128 octets). Pour allouer
        200 octets, vous devez demander 2 pages (256 octets), ce qui gaspille 56 octets.
        Utilisez un allocateur comme celui fourni par <code>nos_stdlib.h</code> pour une granularité plus fine.
    </div>
</div>

## Relation avec l'allocateur

La bibliothèque `nos_mem` utilise `Sbrk` en interne pour obtenir de la mémoire du système :

Cela permet d'avoir un allocateur de granularité fine (octets) au-dessus de `Sbrk`.

## Thread-safety

`Sbrk` est **thread-safe**. L'accès au `brk` et à la table des pages est protégé au niveau de `AddrSpace`. Plusieurs threads peuvent appeler `Sbrk` simultanément sans risque de corruption.

## Voir aussi

- [Memory Overview](./Memory.md) - Vue d'ensemble de la gestion mémoire
- [mmap](./mmap.md) - Allouer une zone mémoire
- [Threads](../threads/Threads.md) - Impact des threads sur l'espace mémoire
- [errno](../../libs/errno.md) - Gestion des erreurs

## Auteurs

Antoine

## Dernière révision

21 Jan 2026