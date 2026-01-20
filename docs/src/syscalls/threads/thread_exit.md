# thread_exit

`thread_exit` - Termine le thread courant.

## Synopsis

```c
#include "syscall.h"

void thread_exit();
```

## Description

`thread_exit` termine immédiatement l'exécution du thread appelant. Les ressources du thread sont libérées et le thread est retiré de la liste des threads du processus.

### Comportement

1. Marque le thread comme `TERMINATED`
2. Signale au processus que le thread a terminé
3. Retire le thread de la liste des threads du processus
4. Libère le CPU

<div class="callout callout-warning">
    <div class="callout-title">Pas de retour</div>
    <div class="callout-content">
        <code>thread_exit</code> ne retourne jamais. Tout code placé après cet appel
        ne sera jamais exécuté.
    </div>
</div>

## Paramètres

Aucun paramètre.

## Valeur de retour

Cette fonction ne retourne pas.

## Exemples

### Exemple : Terminaison simple

```c
#include "syscall.h"

void worker(int arg) {
    PutString("Travail en cours...\n", 20);
    
    // Travail terminé
    thread_exit();
    
    // Ce code n'est JAMAIS exécuté
    PutString("Jamais affiche\n", 15);
}
```

<div class="callout callout-note">
    <div class="callout-title">Thread principal</div>
    <div class="callout-content">
        Si le thread principal appelle <code>thread_exit()</code>, les autres threads
        du processus continuent leur exécution. Le processus ne se termine que lorsque
        tous ses threads ont terminé.
    </div>
</div>

## Comportement spécial

### Dernier thread du processus

Quand le dernier thread d'un processus appelle `thread_exit()` :

1. Le processus est marqué comme terminé
2. Le code de sortie est défini (0 par défaut)
3. Les ressources du processus sont nettoyées

### Ressources libérées

À la terminaison d'un thread :

- Stack noyau : libérée
- Contexte CPU : abandonné
- TID : recyclé pour réutilisation

<div class="callout callout-warning">
    <div class="callout-title">Ressources utilisateur</div>
    <div class="callout-content">
        Les ressources allouées en espace utilisateur (stack, TLS custom, etc.) ne sont
        <strong>pas</strong> automatiquement libérées par <code>thread_exit</code>.
        Si vous avez alloué manuellement ces ressources, utilisez la bibliothèque pthread
        qui gère le nettoyage, ou libérez-les avant d'appeler <code>thread_exit</code>.
    </div>
</div>

## Différence avec return

Dans une fonction thread, `return` et `thread_exit()` ont le même effet :

```c
void worker(int arg) {
    // Ces deux sont équivalents :
    thread_exit();
    // ou
    return;  // Appelle implicitement thread_exit()
}
```

Cependant, `thread_exit()` peut être appelé depuis n'importe où dans la pile d'appels, pas seulement depuis la fonction de niveau supérieur.

## Thread-safety

L'appel est thread-safe et peut être utilisé simultanément par plusieurs threads sans risque de corruption de données.

## Voir aussi

- [thread_create](./thread_create.md) - Créer un thread
- [thread_self](./thread_self.md) - Obtenir son TID
- [Threads Overview](./Threads.md) - Vue d'ensemble

## Auteurs

Antoine

## Dernière révision

18 Jan 2026