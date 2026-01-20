# thread_create

`thread_create` - Crée un nouveau thread dans le processus courant.

## Synopsis

```c
#include "syscall.h"

int thread_create(void* args);
```

## Description

`thread_create` crée un nouveau thread utilisateur qui partage l'espace d'adressage du processus appelant.

### Comportement

1. Copie la structure `user_thread_args` depuis l'espace utilisateur
2. Valide les paramètres (entry point, stack, TLS)
3. Crée un nouveau thread noyau associé au processus
4. Configure les registres utilisateur du nouveau thread
5. Ajoute le thread à la file d'ordonnancement
6. Retourne le TID du nouveau thread

## Paramètres

### `args`

Pointeur vers une structure `user_thread_args` :

```c
typedef struct {
    ptr_32 entry;      // Adresse de la fonction à exécuter
    ptr_32 arg;        // Argument passé à la fonction
    ptr_32 user_sp;    // Stack pointer (0 = allocation auto)
    ptr_32 tls_base;   // Base du TLS (0 = pas de TLS)
} user_thread_args;
```

**Type** : `void*` (pointeur vers `user_thread_args`)  
**Direction** : IN  
**Registre** : `$4`

#### Champs de la structure

| Champ      | Description                  | Contraintes                                                |
|------------|------------------------------|------------------------------------------------------------|
| `entry`    | Point d'entrée du thread     | Adresse valide, alignée sur 4 octets, dans le segment code |
| `arg`      | Argument passé à la fonction | Aucune (peut être 0)                                       |
| `user_sp`  | Stack pointer initial        | 0 ou adresse de stack valide                               |
| `tls_base` | Base du Thread-Local Storage | 0 ou adresse TLS valide                                    |

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification              |
|--------|----------------------------|
| `>= 0` | TID du thread créé         |
| `-1`   | Erreur (consulter `errno`) |

## Exemples

### Exemple 1 : Création simple

```c
#include "syscall.h"

typedef struct {
    unsigned int entry;
    unsigned int arg;
    unsigned int user_sp;
    unsigned int tls_base;
} thread_args;

void worker(int value) {
    PutString("Worker: ", 8);
    PutInt(value);
    PutChar('\n');
    thread_exit();
}

int main() {
    thread_args args = {
        .entry = (unsigned int)worker,
        .arg = 42,
        .user_sp = 0,
        .tls_base = 0
    };
    
    int tid = thread_create(&args);
    
    if (tid >= 0) {
        PutString("Thread cree: TID=", 17);
        PutInt(tid);
        PutChar('\n');
    }
    
    return 0;
}
```

### Exemple 2 : Avec TLS personnalisé

```c
#include "syscall.h"
#include "tls.h"

// Allouer une zone TLS
char tls_area[TLS_TOTAL_SIZE] __attribute__((aligned(4)));

void thread_with_tls(int arg) {
    // Le TLS est accessible via $gp
    tls_t* tls = __get_tls();
    tls->errno_val = 0;
    
    // ...
    
    thread_exit();
}

int main() {
    // Initialiser la structure TLS
    tls_t* tls = (tls_t*)tls_area;
    tls->self = tls;
    tls->errno_val = 0;
    
    thread_args args = {
        .entry = (unsigned int)thread_with_tls,
        .arg = 0,
        .user_sp = 0,
        .tls_base = (unsigned int)tls_area
    };
    
    thread_create(&args);
    
    return 0;
}
```

## Limitations

<div class="callout callout-warning">
    <div class="callout-title">Pas de join natif</div>
    <div class="callout-content">
        Le syscall <code>thread_create</code> ne fournit pas de mécanisme pour attendre
        la terminaison du thread. Utilisez les futex ou la bibliothèque pthread pour
        synchroniser avec les threads créés.
    </div>
</div>

<div class="callout callout-warning">
    <div class="callout-title">Stack utilisateur</div>
    <div class="callout-content">
        Si <code>user_sp</code> est 0, la stack est allouée automatiquement par le noyau.
        Si vous fournissez votre propre stack, assurez-vous qu'elle est correctement alignée
        et de taille suffisante.
    </div>
</div>

## Thread-safety

L'appel `thread_create` est thread-safe. Plusieurs threads peuvent créer des threads simultanément.

## Voir aussi

- [thread_exit](./thread_exit.md) - Terminer un thread
- [thread_self](./thread_self.md) - Obtenir son TID
- [Threads Overview](./Threads.md) - Vue d'ensemble
- [errno](../../libs/errno.md) - Gestion des erreurs

## Auteurs

Antoine

## Dernière révision

18 Jan 2026