# Attributs de thread

Cette page documente les fonctions de gestion des attributs de thread (`pthread_attr_*`).

## Vue d'ensemble

Les attributs permettent de configurer le comportement d'un thread avant sa création. Une fois le thread créé, les attributs ne sont plus liés au thread.

<div class="callout callout-security">
    <div class="callout-title">Sécurité des syscalls</div>
    <div class="callout-content">
        Actuellement, des pointeurs sont necessaires pour passer les attributs entre l'espace utilisateur et le noyau.
        Cela peut causer des craches ou des comportements indéfinis si les pointeurs sont invalides.
        Cela sera corrigé lorsque l'API pthread sera remontée en espace utilisateur.
        En attendant, assurez-vous que tous les pointeurs passés aux syscalls sont valides.
    </div>
</div>

### Structure `pthread_attr_t`

```c
typedef struct {
    posix_thread_detachstate_t detachstate;  // JOINABLE ou DETACHED
} pthread_attr_t;
```

<div class="callout callout-limitation">
    <div class="callout-title">Attributs limités</div>
    <div class="callout-content">
        Seul l'attribut <code>detachstate</code> est actuellement supporté.
        Les attributs POSIX suivants ne sont pas implémentés :
        <ul>
            <li><code>stacksize</code> - Taille de stack (fixée à 2 pages)</li>
            <li><code>stackaddr</code> - Adresse de stack (calculée automatiquement)</li>
            <li><code>guardsize</code> - Taille de la guard page</li>
            <li><code>schedpolicy</code> - Politique d'ordonnancement</li>
            <li><code>schedparam</code> - Paramètres d'ordonnancement</li>
        </ul>
    </div>
</div>

## Cycle de vie des attributs

```
Pthread_attr_init(&attr)
        │
        ▼
    [attr valide]
        │
        ├── Pthread_attr_setdetachstate(&attr, DETACHED)
        │
        ├── PthreadCreate(&tid, &attr, func, arg)
        │   (attr peut être réutilisé ou modifié)
        │
        ├── PthreadCreate(&tid2, &attr, func, arg)
        │   (même attr pour plusieurs threads)
        │
        ▼
Pthread_attr_destroy(&attr)
```

---

## Pthread_attr_init

Initialise un objet d'attributs avec les valeurs par défaut.

**Numéro d'appel système** : `SC_Pthread_attr_init` (22)

### Synopsis

```c
int Pthread_attr_init(pthread_attr_t *attr);
```

### Description

Initialise `attr` avec les valeurs par défaut :
- `detachstate` = `JOINABLE`

### Paramètres

| Paramètre | Type               | Direction | Description                     |
|-----------|--------------------|-----------|---------------------------------|
| `attr`    | `pthread_attr_t *` | OUT       | Objet d'attributs à initialiser |

### Valeur de retour

| Valeur | Signification              |
|--------|----------------------------|
| `0`    | Succès                     |
| `-1`   | Erreur (`attr` est `NULL`) |

### Exemple

```c
pthread_attr_t attr;

if (Pthread_attr_init(&attr) != 0) {
    PutString("Init failed\n", 12);
    return 1;
}

// Utiliser attr...

Pthread_attr_destroy(&attr);
```

---

## Pthread_attr_destroy

Détruit un objet d'attributs.

**Numéro d'appel système** : `SC_Pthread_attr_destroy` (23)

### Synopsis

```c
int Pthread_attr_destroy(pthread_attr_t *attr);
```

### Description

Libère les ressources associées à `attr`. Dans l'implémentation actuelle, cette fonction ne fait rien car il n'y a pas de ressources dynamiques.

<div class="callout callout-tip">
    <div class="callout-title">Bonne pratique</div>
    <div class="callout-content">
        Toujours appeler <code>Pthread_attr_destroy</code> après utilisation, 
        même si l'implémentation actuelle n'en a pas besoin. 
        Cela garantit la compatibilité future.
    </div>
</div>

### Paramètres

| Paramètre | Type               | Direction | Description                  |
|-----------|--------------------|-----------|------------------------------|
| `attr`    | `pthread_attr_t *` | IN        | Objet d'attributs à détruire |

### Valeur de retour

| Valeur | Signification              |
|--------|----------------------------|
| `0`    | Succès                     |
| `-1`   | Erreur (`attr` est `NULL`) |

---

## Pthread_attr_setdetachstate

Définit l'état de détachement.

### Synopsis

```c
int Pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
```

### Description

Configure si les threads créés avec cet attribut seront joinables ou détachés.

**Numéro d'appel système** : `SC_Pthread_attr_setdetachestate` (24)

### Paramètres

| Paramètre     | Type               | Direction | Description                      |
|---------------|--------------------|-----------|----------------------------------|
| `attr`        | `pthread_attr_t *` | IN/OUT    | Objet d'attributs                |
| `detachstate` | `int`              | IN        | `JOINABLE` (0) ou `DETACHED` (1) |

### Valeur de retour

| Valeur | Signification                                  |
|--------|------------------------------------------------|
| `0`    | Succès                                         |
| `-1`   | Erreur (`attr` NULL ou `detachstate` invalide) |

### Constantes

```c
#define JOINABLE 0
#define DETACHED 1
```

### Exemple

```c
pthread_attr_t attr;

Pthread_attr_init(&attr);
Pthread_attr_setdetachstate(&attr, DETACHED);

posix_thread_t tid;
PthreadCreate(&tid, &attr, my_func, 0);
// Thread créé en mode détaché

Pthread_attr_destroy(&attr);
```

---

## Pthread_attr_getdetachstate

Récupère l'état de détachement.

### Synopsis

```c
int Pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
```

### Description

Récupère la valeur de l'attribut `detachstate`.

**Numéro d'appel système** : `SC_Pthread_attr_getdetachestate` (25)

### Paramètres

| Paramètre     | Type                     | Direction | Description                     |
|---------------|--------------------------|-----------|---------------------------------|
| `attr`        | `const pthread_attr_t *` | IN        | Objet d'attributs               |
| `detachstate` | `int *`                  | OUT       | Pointeur pour stocker la valeur |

### Valeur de retour

| Valeur | Signification                         |
|--------|---------------------------------------|
| `0`    | Succès                                |
| `-1`   | Erreur (`attr` ou `detachstate` NULL) |

### Exemple

```c
pthread_attr_t attr;
int state;

Pthread_attr_init(&attr);

Pthread_attr_getdetachstate(&attr, &state);
// state == JOINABLE (valeur par défaut)

Pthread_attr_setdetachstate(&attr, DETACHED);
Pthread_attr_getdetachstate(&attr, &state);
// state == DETACHED

Pthread_attr_destroy(&attr);
```

---

## Exemple complet

```c
#include "syscall.h"

void *joinable_worker(void *arg) {
    PutString("Joinable worker\n", 16);
    return (void *)42;
}

void *detached_worker(void *arg) {
    PutString("Detached worker\n", 16);
    return 0;  // Valeur perdue
}

int main() {
    pthread_attr_t attr;
    posix_thread_t tid1, tid2;
    void *result;
    int state;
    
    // Initialiser les attributs
    Pthread_attr_init(&attr);
    
    // Vérifier la valeur par défaut
    Pthread_attr_getdetachstate(&attr, &state);
    if (state == JOINABLE) {
        PutString("Default: JOINABLE\n", 18);
    }
    
    // Créer un thread joinable (par défaut)
    PthreadCreate(&tid1, &attr, joinable_worker, 0);
    
    // Modifier l'attribut pour le prochain thread
    Pthread_attr_setdetachstate(&attr, DETACHED);
    
    // Créer un thread détaché
    PthreadCreate(&tid2, &attr, detached_worker, 0);
    
    // Joindre uniquement le thread joinable
    PthreadJoin(tid1, &result);
    PutString("Joinable returned: ", 19);
    PutInt((int)(long)result);
    PutChar('\n');
    
    // PthreadJoin(tid2, 0);  // ERREUR : tid2 est détaché
    
    // Nettoyer
    Pthread_attr_destroy(&attr);
    
    return 0;
}
```

## Implémentation

### Localisation du code

- **Définition structure** : `code/threads/thread.h` (`posix_thread_attr_t`)
- **Fonctions noyau** : `code/threads/thread.cc` (`posix_thread_attr_*`)
- **Stubs utilisateur** : `code/test/start.S`

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Utilisation des attributs
- [PthreadDetach](./PthreadDetach.md) - Alternative au détachement via attributs
- [Vue d'ensemble](Threads.md) - Guide des threads

</div>
</div>

## Auteurs

Antoine, 31 Dec 2025

## Dernière révision

31 Dec 2025
