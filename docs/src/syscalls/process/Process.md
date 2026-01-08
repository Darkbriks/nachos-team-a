# Vue d'ensemble - Process

Cette page documente le système de process de NachOS et l'API Fork simplifiée.

## Vue d'ensemble

NachOS implémente un modèle de Process pouvant chacun contenir plusieurs threads. Un process est toujours enfant d'un autre sauf le kernel qui se lance au début.

## API rapide

| Fonction                            | Description                         |
|-------------------------------------|-------------------------------------|
| [ForkExec](./ForkExec.md) | Créer un nouveau processus |
| [ForkJoin](./PthreadExit.md)     | Attendre un autre processus          |


## Exemple minimal

```c
#include "syscall.h"


int main() {
    int pid = ForkExec("userpages0");
    int result = -1;
    if (pid > 0){
        ForkJoin(pid, &result);
    }
    return 0;
}
```

## Limitations connues


<div class="callout callout-limitation">
    <div class="callout-title">Nombre maximum de processus</div>
    <div class="callout-content">
        <p>Maximum <code>MAX_PROCESS</code> (15 par défaut) processus actifs sur la machine.</p>
        <p>Configurable dans <code>code/userprog/process.h</code>.</p>
    </div>
</div>
<div class="callout callout-fixme">
    <div class="callout-title">errno non thread-safe</div>
    <div class="callout-content">
        <p>La variable <code>errno</code> est globale et partagée entre tous les processus.</p>
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

### Terminaison  du processus

Un Process termine vraiment lorsque tout ses threads sont terminés il renvoie un exitCode qui sera retourné lors de l'appel à Join. La structure interne n'est détruite qu'après l'appel à ForkJoin sur ce processus pour éviter les zombies.

Par défaut l'exitcode est 0, Pour la changer il faut appeler explicitement la fonction Exit(int exitCode) et celui ci sera renvoyé lors de l'appel à ForkJoin.

<div class="callout callout-note">
    <div class="callout-title">Mécanisme interne</div>
    <div class="callout-content">
        <p>Le stub <code>PthreadCreate</code> dans <code>start.S</code> configure le registre <code>$ra</code> 
        pour pointer vers <code>PthreadExit_wrapper</code>, qui appelle <code>PthreadExit</code> avec la valeur 
        de retour en <code>$v0</code>.</p>
    </div>
</div>

```

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-title">Voir aussi</div>
    <div class="callout-content">

- [ForkJoin](./ForkJoin.md) - Attente d'un thread
- [ForkExec](./ForkExec.md) - Création d'un processus 
- [Gestion des erreurs](../errors.md) - errno et codes d'erreur

</div>
</div>

## Auteurs

Tommy, 8 Jan 2026

## Dernière révision

8 Jan 2026

