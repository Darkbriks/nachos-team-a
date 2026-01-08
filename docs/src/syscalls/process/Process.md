# Appels Système de Gestion des Processus

Cette section documente les appels système liés à la gestion des processus dans Nachos, permettant de créer et synchroniser des processus utilisateur.

## Vue d'ensemble

Les appels système de gestion des processus permettent de :
- Créer de nouveaux processus avec leurs propres espaces d'adressage
- Attendre la terminaison de processus enfants
- Gérer la hiérarchie parent-enfant entre processus
- Coordonner l'exécution de programmes multiples

## Concepts fondamentaux

### Processus vs Threads

Dans Nachos, il existe une distinction claire entre processus et threads :

| Aspect | Processus | Thread |
|--------|-----------|---------|
| **Espace d'adressage** | Propre espace mémoire isolé | Partagé avec autres threads du processus |
| **Communication** | Difficile (espaces séparés) | Facile (mémoire partagée) |
| **Création** | `ForkExec()` | `PthreadCreate()` |
| **Terminaison** | `Exit()` | `PthreadExit()` |
| **Synchronisation** | `Join()` ou `ForkJoin()` | `PthreadJoin()` |
| **Identifiant** | PID (Process ID) | TID (Thread ID) |
| **Isolation** | Forte | Aucune |
| **Overhead** | Élevé | Faible |

### PID (Process ID)

Chaque processus possède un identifiant unique (PID) attribué lors de sa création :
- Le premier processus a généralement le PID 0
- Les PIDs sont alloués via un bitmap `process_bitmap`
- Limite : `MAX_PROCESS` processus simultanés

### Hiérarchie parent-enfant

Nachos maintient une hiérarchie entre processus :
- Un processus qui crée un autre processus devient son **parent**
- Le processus créé devient un **enfant** du créateur
- Un parent peut attendre la terminaison de ses enfants avec `Join()` ou `ForkJoin()`
- Un enfant ne peut pas attendre son parent (erreur `E_INVAL`)

### Espace d'adressage (AddrSpace)

Chaque processus possède :
- Son propre espace d'adressage virtuel
- Ses propres pages mémoire
- Son propre code, données et pile
- Isolation complète des autres processus

### Thread principal

Chaque processus contient au minimum :
- Un thread principal qui exécute la fonction `main()`
- Possibilité de créer des threads additionnels avec `PthreadCreate()`
- Tous les threads d'un processus partagent le même `AddrSpace`

## Liste des appels système

### [ForkExec](./ForkExec.md)

Crée un nouveau processus qui exécute un fichier exécutable.

**Utilisation** :
```c
int ForkExec(char *file_name);
```

**Caractéristiques** :
- Retourne immédiatement (exécution asynchrone)
- Crée un nouvel espace d'adressage
- Le nouveau processus devient un enfant du processus appelant
- Retourne le PID du processus créé

**Exemple** :
```c
int child_pid = ForkExec("./program");
if (child_pid < 0) {
    PutString("Failed to create process\n", 26);
}
```

### [Join / ForkJoin](./ForkJoin.md)

Attend la terminaison d'un processus enfant.

**Utilisation** :
```c
int Join(int pid);          // Ancienne version
int ForkJoin(int pid);      // Nouvelle version
```

**Caractéristiques** :
- Bloque jusqu'à la terminaison de l'enfant
- Ne peut attendre que ses propres enfants
- Récupère le code de retour du processus enfant
- Nettoie les ressources du processus terminé

**Exemple** :
```c
int child_pid = ForkExec("./program");
ForkJoin(child_pid);  // Attend la fin
PutString("Child finished\n", 16);
```

### [Exit](../system/Exit.md)

Termine le processus courant proprement.

**Utilisation** :
```c
void Exit(int status);
```

**Comportement** :
- Attend la terminaison de tous les threads du processus
- Libère toutes les ressources (mémoire, fichiers, sémaphores)
- Notifie le processus parent
- Si dernier processus actif, arrête le système

## Cycle de vie d'un processus

```
┌─────────────────────────────────────────────────────────┐
│                    CRÉATION                             │
│  ForkExec("program") depuis processus parent            │
│  ├─ Alloue PID via process_bitmap                       │
│  ├─ Crée objet Process                                  │
│  ├─ Crée AddrSpace et charge l'exécutable               │
│  ├─ Crée thread principal                               │
│  └─ Ajoute à la hiérarchie parent-enfant                │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                   EXÉCUTION                             │
│  Le processus s'exécute de manière indépendante         │
│  ├─ Peut créer des threads (PthreadCreate)              │
│  ├─ Peut créer des processus enfants (ForkExec)         │
│  ├─ Peut allouer des ressources (sémaphores, etc.)      │
│  └─ S'exécute jusqu'à Exit() ou return depuis main()    │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                  TERMINAISON                            │
│  Exit(status) appelé explicitement ou implicitement     │
│  ├─ Attend terminaison de tous les threads              │
│  ├─ Marque processus comme TERMINATED                   │
│  ├─ Libère ressources (AddrSpace, sémaphores, etc.)     │
│  ├─ Réveille le parent si en attente (Join/ForkJoin)    │
│  └─ Processus marqué pour destruction                   │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│                   DESTRUCTION                           │
│  Après Join/ForkJoin du parent ou si orphelin           │
│  ├─ Libère PID dans process_bitmap                      │
│  ├─ Détruit objet Process                               │
│  └─ Nettoie toutes les structures de données            │
└─────────────────────────────────────────────────────────┘
```

## Patterns courants

### Pattern 1 : Création et attente simple

```c
int main() {
    int child_pid = ForkExec("./worker");

    if (child_pid < 0) {
        PutString("Error creating process\n", 23);
        return 1;
    }

    PutString("Parent waiting for child...\n", 28);
    ForkJoin(child_pid);
    PutString("Child terminated\n", 17);

    return 0;
}
```

### Pattern 2 : Création de plusieurs processus

```c
int main() {
    int pids[5];
    int i;

    // Créer 5 processus
    for (i = 0; i < 5; i++) {
        pids[i] = ForkExec("./worker");
        if (pids[i] < 0) {
            PutString("Error at iteration ", 19);
            PutInt(i);
            PutChar('\n');
            break;
        }
    }

    // Attendre tous les processus
    for (i = 0; i < 5; i++) {
        if (pids[i] >= 0) {
            ForkJoin(pids[i]);
        }
    }

    PutString("All children finished\n", 22);
    return 0;
}
```

### Pattern 3 : Pipeline de processus

```c
int main() {
    int stage1_pid, stage2_pid;

    // Premier étage
    stage1_pid = ForkExec("./stage1");
    ForkJoin(stage1_pid);
    PutString("Stage 1 complete\n", 17);

    // Deuxième étage (attend que stage1 soit terminé)
    stage2_pid = ForkExec("./stage2");
    ForkJoin(stage2_pid);
    PutString("Stage 2 complete\n", 17);

    PutString("Pipeline finished\n", 18);
    return 0;
}
```

### Pattern 4 : Processus avec code de retour

```c
// Processus enfant (child.c)
int main() {
    int result = compute_something();

    if (result < 0) {
        Exit(1);  // Échec
    }

    Exit(0);  // Succès
}

// Processus parent (parent.c)
int main() {
    int child_pid = ForkExec("./child");
    int status = ForkJoin(child_pid);

    if (status == 0) {
        PutString("Child succeeded\n", 16);
    } else {
        PutString("Child failed with status ", 25);
        PutInt(status);
        PutChar('\n');
    }

    return 0;
}
```

### Pattern 5 : Processus maître-esclaves

```c
#define NUM_WORKERS 4

int main() {
    int worker_pids[NUM_WORKERS];
    int i;

    PutString("Master: Starting workers\n", 25);
    
    // Lancer les workers
    for (i = 0; i < NUM_WORKERS; i++) {
        worker_pids[i] = ForkExec("./worker");
        PutString("Master: Started worker ", 23);
        PutInt(worker_pids[i]);
        PutChar('\n');
    }

    PutString("Master: All workers started\n", 28);

    // Attendre tous les workers
    for (i = 0; i < NUM_WORKERS; i++) {
        ForkJoin(worker_pids[i]);
        PutString("Master: Worker ", 15);
        PutInt(worker_pids[i]);
        PutString(" finished\n", 10);
    }

    PutString("Master: All work complete\n", 26);
    return 0;
}
```

## Synchronisation et coordination

### Attente de terminaison

`Join` / `ForkJoin` permet au parent de :
- Bloquer jusqu'à la terminaison d'un enfant spécifique
- Récupérer le code de retour de l'enfant
- Garantir que les ressources sont nettoyées

### Ordre d'exécution

Sans synchronisation explicite :
- Les processus s'exécutent de manière concurrente
- L'ordre d'exécution est non-déterministe
- Le scheduler décide quand chaque processus s'exécute

Avec `ForkJoin` :
- Le parent peut forcer un ordre séquentiel
- Permet de créer des pipelines de traitement

## Limitations et contraintes

### Limite de processus

<div class="callout callout-limitation">
    <div class="callout-title">MAX_PROCESS</div>
    <div class="callout-content">
        Le système supporte un nombre maximum de processus simultanés défini par
        <code>MAX_PROCESS</code>. Au-delà, <code>ForkExec</code> échoue avec <code>E_NOMEM</code>.
    </div>
</div>

### Hiérarchie stricte

- Un processus ne peut attendre que ses propres enfants
- Impossible d'attendre un processus arbitraire
- Pas de notion de "groupe de processus" comme dans POSIX

### Pas de communication inter-processus (IPC)

Nachos ne fournit pas nativement :
- Pipes
- Mémoire partagée
- Files de messages
- Sockets

Les processus doivent utiliser :
- Fichiers pour échanger des données
- Codes de retour pour signaler des statuts
- Ordre d'exécution via `ForkJoin`

## Codes d'erreur courants

| errno | Constante | Signification | Contexte |
|-------|-----------|---------------|----------|
| 1 | `E_INVAL` | PID invalide ou tentative d'attendre soi-même | Join/ForkJoin |
| 7 | `E_NOMEM` | Plus de PID disponibles | ForkExec |
| 9 | `E_NOSPC` | Processus introuvable | Join/ForkJoin |
| 11 | `E_NOENT` | Fichier exécutable introuvable | ForkExec |
| 12 | `E_NOCPC` | Le processus n'est pas un enfant | Join/ForkJoin |

## Thread-safety

Les appels système de processus sont thread-safe :
- Plusieurs threads d'un même processus peuvent appeler `ForkExec`
- La structure parent-enfant est protégée par des verrous
- Les PIDs sont alloués atomiquement via `process_bitmap`

## Performance

### Coût de création

| Opération | Coût |
|-----------|------|
| Allocation PID | O(log(n)) |
| Création AddrSpace | Elevé |
| Création thread principal | Moyen |
| **Total ForkExec** | Elevé |

### Comparaison thread vs processus

Pour exécuter du code concurrent :
- **Threads** : Préférer si partage de données nécessaire (léger, rapide)
- **Processus** : Préférer si isolation nécessaire (lourd, sûr)

## Différences avec POSIX

| Aspect | POSIX (fork + exec) | Nachos (ForkExec) |
|--------|-------------------|-------------------|
| Appel | `fork()` puis `exec()` | `ForkExec()` combiné |
| Copie mémoire | fork copie l'espace parent | Charge nouvel exécutable |
| Héritage | Hérite état parent | Nouvel état propre |
| PID du parent | `getppid()` | Non accessible |
| Attente | `wait()` / `waitpid()` | `Join()` / `ForkJoin()` |
| Signaux | SIGCHLD au parent | Réveil direct |
| Groupes | Groupes de processus | Non supporté |
| Sessions | Sessions et contrôle terminal | Non supporté |

## Voir aussi

- [ForkExec](./ForkExec.md) - Créer un processus
- [ForkJoin](./ForkJoin.md) - Attendre un processus
- [Exit](../system/Exit.md) - Terminer un processus
- [PthreadCreate](../threads/PthreadCreate.md) - Créer un thread
- [Threads Overview](../threads/Threads.md) - Gestion des threads

## Auteurs

Alioune Badara DIENE, 8 Jan 2026

## Dernière révision

8 Jan 2026
