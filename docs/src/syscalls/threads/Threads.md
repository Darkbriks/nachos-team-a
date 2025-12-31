# Threads utilisateur

Cette page documente le système de threads utilisateur de NachOS et l'API pthread simplifiée.

## Vue d'ensemble

NachOS implémente un modèle de threads 1:1 où chaque thread utilisateur est directement supporté par un thread noyau (kernel thread). L'API est inspirée de POSIX pthreads.

<div class="callout callout-future">
    <div class="callout-title">Évolution prévue</div>
    <div class="callout-content">
        <p>Dans une version future, il est envisagé de faire évoluer l'architecture vers un modèle plus proche de Linux :</p>
        <ul>
            <li>Bibliothèque pthread entièrement en espace utilisateur</li>
            <li>Syscalls bas niveau</li>
            <li>Allocation dynamique des stacks</li>
        </ul>
    </div>
</div>

## API rapide

| Fonction                            | Description                         |
|-------------------------------------|-------------------------------------|
| [PthreadCreate](./PthreadCreate.md) | Créer un nouveau thread             |
| [PthreadExit](./PthreadExit.md)     | Terminer le thread courant          |
| [PthreadJoin](./PthreadJoin.md)     | Attendre la terminaison d'un thread |
| [PthreadDetach](./PthreadDetach.md) | Détacher un thread                  |

### Attributs de création

| Fonction                                                              | Description                   |
|-----------------------------------------------------------------------|-------------------------------|
| [Pthread_attr_init](./attrs.md#pthread_attr_init)                     | Initialiser les attributs     |
| [Pthread_attr_destroy](./attrs.md#pthread_attr_destroy)               | Détruire les attributs        |
| [Pthread_attr_setdetachstate](./attrs.md#pthread_attr_setdetachstate) | Définir l'état de détachement |
| [Pthread_attr_getdetachstate](./attrs.md#pthread_attr_getdetachstate) | Obtenir l'état de détachement |

## Exemple minimal

```c
#include "syscall.h"

void *my_thread(void *arg) {
    int id = (int)(long)arg;
    PutString("Hello from thread ", 18);
    PutInt(id);
    PutChar('\n');
    return (void *)(long)(id * 2);
}

int main() {
    posix_thread_t tid;
    void *retval;
    
    // Créer un thread
    if (PthreadCreate(&tid, 0, my_thread, (void *)42) != 0) {
        PutString("Error creating thread\n", 22);
        return 1;
    }
    
    // Attendre sa terminaison
    PthreadJoin(tid, &retval);
    
    PutString("Thread returned: ", 17);
    PutInt((int)(long)retval);  // Affiche 84
    PutChar('\n');
    
    return 0;
}
```

## Limitations connues

<div class="callout callout-limitation">
    <div class="callout-title">Allocation de stack statique</div>
    <div class="callout-content">
        <p>Les stacks des threads sont allouées statiquement en fonction du TID :</p>
        <pre><code>stack_top = numPages * PageSize - (TID + 1) * stackSize</code></pre>
    </div>
</div>

<div class="callout callout-limitation">
    <div class="callout-title">Nombre maximum de threads</div>
    <div class="callout-content">
        <p>Maximum <code>MAX_THREAD</code> (10 par défaut) threads par processus.</p>
        <p>Configurable dans <code>code/userprog/process.h</code>.</p>
    </div>
</div>

<div class="callout callout-limitation">
    <div class="callout-title">Taille de stack fixe</div>
    <div class="callout-content">
        <p>Chaque thread dispose de 2 pages (256 octets) de stack.</p>
        <p>Pas de détection de stack overflow ni de guard pages.</p>
    </div>
</div>

<div class="callout callout-fixme">
    <div class="callout-title">errno non thread-safe</div>
    <div class="callout-content">
        <p>La variable <code>errno</code> est globale et partagée entre tous les threads.</p>
        <p>Voir <a href="../errors.md#errno-global---non-thread-safe">Gestion des erreurs</a> pour plus de détails.</p>
    </div>
</div>

## Modèle mémoire

Tous les threads d'un processus partagent le même espace d'adressage (`AddrSpace`), incluant :

- **Code** : segment `.text` (lecture seule)
- **Données** : segments `.data` et `.bss`
- **Tas** : variables dynamiques
- **Variables globales** : partagées entre tous les threads

Chaque thread possède :

- **Stack privée** : variables locales, frames d'appel
- **Registres** : sauvegardés/restaurés lors des context switches
- **TID** : identifiant unique au sein du processus

<div class="callout callout-warning">
    <div class="callout-title">Race conditions</div>
    <div class="callout-content">
        <p>Les variables globales sont partagées sans protection. Utilisez des <a href="../sync/Sync.md">sémaphores</a> pour synchroniser les accès.</p>
    </div>
</div>

## Layout mémoire

```
Espace d'adressage (numPages * PageSize)
┌────────────────────────────┐ ← numPages * PageSize
│      Stack main (TID 0)    │
├────────────────────────────┤ ← numPages * PageSize - stackSize
│      Stack thread 1        │
├────────────────────────────┤ ← numPages * PageSize - 2 * stackSize
│      Stack thread 2        │
├────────────────────────────┤
│           ...              │
├────────────────────────────┤
│     (espace libre)         │
├────────────────────────────┤
│           Tas              │
├────────────────────────────┤
│      .bss (données)        │
├────────────────────────────┤
│      .data (données)       │
├────────────────────────────┤
│      .text (code)          │
└────────────────────────────┘ ← 0
```

## Terminaison

### Terminaison explicite

Un thread peut terminer explicitement avec `PthreadExit()` :

```c
void *my_thread(void *arg) {
    // ... travail ...
    PthreadExit((void *)42);
    // Code jamais atteint
}
```

### Terminaison implicite

Si la fonction du thread retourne normalement, `PthreadExit()` est appelé automatiquement avec la valeur de retour :

```c
void *my_thread(void *arg) {
    return (void *)42;  // Équivalent à PthreadExit((void *)42)
}
```

<div class="callout callout-note">
    <div class="callout-title">Mécanisme interne</div>
    <div class="callout-content">
        <p>Le stub <code>PthreadCreate</code> dans <code>start.S</code> configure le registre <code>$ra</code> 
        pour pointer vers <code>PthreadExit_wrapper</code>, qui appelle <code>PthreadExit</code> avec la valeur 
        de retour en <code>$v0</code>.</p>
    </div>
</div>

### Terminaison du processus

<div class="callout callout-warning">
    <div class="callout-title">Attente des threads</div>
    <div class="callout-content">
        <p>Le thread principal attend automatiquement la terminaison de tous les threads enfants avant de terminer le processus.</p>
    </div>
</div>

```c
int main() {
    posix_thread_t t1, t2;
    
    PthreadCreate(&t1, 0, worker, 0);
    PthreadCreate(&t2, 0, worker, 0);

    // Le thread principal attend t1 et t2 avant de terminer
    return 0;
}
```

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [PthreadCreate](./PthreadCreate.md) - Création de thread
- [PthreadJoin](./PthreadJoin.md) - Attente de terminaison
- [Attributs de thread](./attrs.md) - Configuration des threads
- [Sémaphores](../sync/Sync.md) - Synchronisation entre threads
- [Gestion des erreurs](../errors.md) - errno et codes d'erreur

</div>
</div>

## Auteurs

Antoine, 31 Dec 2025

## Dernière révision

31 Dec 2025