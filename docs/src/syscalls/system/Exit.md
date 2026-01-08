# Exit

`Exit` - Termine proprement le processus courant

## Synopsis

```c
#include "syscall.h"

void Exit(int status) __attribute__((noreturn));
```

## Description

`Exit` termine proprement le processus courant. Cette fonction attend la terminaison de tous les threads du processus, puis libère toutes les ressources associées. Si c'est le dernier processus actif du système, `Exit` arrête complètement Nachos.

**Numéro d'appel système** : `SC_Exit` (1)

### Comportement nominal

1. Lecture du code de retour `status` depuis le registre `$4`
2. Écriture du code de retour dans le registre `$2`
3. Attente de la terminaison de tous les threads du processus via `WaitForAllThreadsTerminate()`
4. Si c'est le dernier processus actif :
   - Suppression du processus
   - Arrêt du système avec `interrupt->Halt()`
5. Sinon :
   - Marquage du processus pour destruction différée
   - Terminaison du thread courant via `currentThread->Finish()`

<div class="callout callout-note">
    <div class="callout-title">Terminaison propre</div>
    <div class="callout-content">
        Contrairement à <code>Halt</code>, <code>Exit</code> garantit une terminaison propre
        du processus en attendant tous ses threads et en libérant ses ressources.
    </div>
</div>

## Paramètres

### `status`

Code de retour du processus, conventionnellement 0 pour succès, non-zéro pour erreur.

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Convention** :
- `0` = terminaison normale
- `non-zéro` = erreur ou terminaison anormale

## Valeur de retour

**Type** : `void`

Cette fonction ne retourne jamais au code utilisateur. Le code de retour `status` est écrit dans le registre `$2` avant la terminaison et peut être récupéré par un processus parent via `Join()`.

## Codes d'erreur

Aucun. Cette fonction ne peut pas échouer et ne retourne jamais.

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S:99`
- **Handler noyau** : `code/userprog/exception.cc:handle_SC_Exit()`

### Flux d'exécution

```
Exit(status)
    │
    ▼
start.S: Exit
    │ charge $4 = status
    │ charge $2 = SC_Exit (1)
    ▼
syscall SC_Exit
    │
    ▼
handle_SC_Exit()
    │ ├─ lit status depuis $4
    │ ├─ écrit status dans $2
    │ └─ récupère process
    ▼
process->WaitForAllThreadsTerminate()
    │ attend que tous les threads se terminent
    ▼
Si dernier processus actif:
    │ ├─ delete process
    │ └─ interrupt->Halt()
Sinon:
    │ ├─ processToBeDestroyed = process
    │ └─ currentThread->Finish()
    ▼
[Processus terminé]
```

## Exemples

### Exemple 1 : Terminaison normale

```c
#include "syscall.h"

int main() {
    PutString("Hello, Nachos!\n", 15);
    PutString("Program completed successfully.\n", 32);
    Exit(0);  // Code de retour 0 = succès
}
```

### Exemple 2 : Terminaison avec erreur

```c
#include "syscall.h"

int main() {
    int result;

    result = GetInt(&result);
    if (result != 0) {
        PutString("Error reading integer!\n", 23);
        Exit(1);  // Code d'erreur 1
    }

    PutString("Value read: ", 12);
    PutInt(result);
    PutChar('\n');

    Exit(0);  // Succès
}
```

### Exemple 3 : Attente des threads

```c
#include "syscall.h"

void *worker(void *arg) {
    int id = (int)(long)arg;
    int i;

    for (i = 0; i < 5; i++) {
        PutString("Thread ", 7);
        PutInt(id);
        PutString(" iteration ", 11);
        PutInt(i);
        PutChar('\n');
    }

    return 0;
}

int main() {
    posix_thread_t tid1, tid2;

    PutString("Starting threads...\n", 20);

    PthreadCreate(&tid1, 0, worker, (void *)1);
    PthreadCreate(&tid2, 0, worker, (void *)2);

    PutString("Main thread calling Exit()\n", 27);
    Exit(0);  // Attend automatiquement tid1 et tid2

    // Jamais atteint
}
```

**Comportement** : `Exit` attend que les threads 1 et 2 se terminent avant de détruire le processus.

### Exemple 4 : Exit vs return

```c
#include "syscall.h"

int main() {
    int choice = 1;

    if (choice == 0) {
        PutString("Using return\n", 13);
        return 0;  // Équivalent à Exit(0)
    } else {
        PutString("Using Exit\n", 11);
        Exit(0);   // Explicite
    }
}
```

<div class="callout callout-note">
    <div class="callout-title">return vs Exit</div>
    <div class="callout-content">
        Dans le thread principal, <code>return status;</code> est typiquement converti
        en <code>Exit(status);</code> par le runtime C. Les deux formes sont équivalentes.
    </div>
</div>

### Exemple 5 : Codes de retour conventionnels

```c
#include "syscall.h"

#define SUCCESS 0
#define ERROR_INVALID_INPUT 1
#define ERROR_OVERFLOW 2
#define ERROR_IO 3

int divide(int a, int b, int *result) {
    if (b == 0) return ERROR_INVALID_INPUT;
    *result = a / b;
    return SUCCESS;
}

int main() {
    int a = 10, b = 2, result;
    int status;

    status = divide(a, b, &result);

    if (status != SUCCESS) {
        PutString("Error: ", 7);
        PutInt(status);
        PutChar('\n');
        Exit(status);
    }

    PutString("Result: ", 8);
    PutInt(result);
    PutChar('\n');

    Exit(SUCCESS);
}
```

## Cas d'usage

### Terminaison normale

`Exit` est utilisé pour :
- Terminer proprement un programme après son exécution
- Retourner un code de statut au système ou au processus parent
- Garantir la libération de toutes les ressources du processus

### Gestion d'erreur

Les codes de retour permettent de signaler différents types d'erreurs :
- `0` : succès
- `1-255` : codes d'erreur spécifiques à l'application

## Thread-safety

`Exit` est thread-safe. N'importe quel thread d'un processus peut appeler `Exit`, et le système garantit que tous les autres threads du processus seront terminés proprement.

<div class="callout callout-warning">
    <div class="callout-title">Appel concurrent</div>
    <div class="callout-content">
        Si plusieurs threads appellent <code>Exit</code> simultanément, le premier appel
        déclenchera la terminaison du processus. Les autres threads seront terminés
        avant d'atteindre leur appel à <code>Exit</code>.
    </div>
</div>

## Différences avec POSIX

| Aspect | POSIX exit() | Nachos Exit() |
|--------|--------------|---------------|
| Prototype | `void exit(int status)` | `void Exit(int status)` |
| Casse | Minuscule | Majuscule |
| Threads | Termine tout le processus | Attend les threads du processus |
| atexit() | Appelle les handlers | Non supporté |
| Buffers stdio | Vidés | Non applicable |
| Processus parent | Notifié via SIGCHLD | Peut utiliser Join() |

## Relation avec Join()

Le code de retour passé à `Exit` peut être récupéré par un processus parent utilisant `Join()` :

```c
// Processus enfant
int main() {
    // ... traitement ...
    Exit(42);
}

// Processus parent
int main() {
    SpaceId child = Exec("child_program");
    int status = Join(child);  // Récupère 42
    PutInt(status);
    Exit(0);
}
```

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [Halt](./Halt.md) - Arrêter tout le système
- [PthreadExit](../threads/PthreadExit.md) - Terminer un thread
- [Join](../process/Join.md) - Attendre un processus enfant
- [ForkExec](../process/ForkExec.md) - Créer un processus

</div>
</div>

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
