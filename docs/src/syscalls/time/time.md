# time

`time` - Obtient le temps calendaire actuel (wall clock)

## Synopsis

```c
#include "syscall.h"

int time(time_t *loc);
```

## Description

`time` retourne le temps calendaire actuel (wall clock time) en secondes depuis l'époque Unix (1er janvier 1970, 00:00:00 UTC). Contrairement à `GetCurrentTick` qui retourne des ticks système internes, `time` fournit le temps réel du monde extérieur.

**Numéro d'appel système** : `SC_time` (42)

### Comportement nominal

1. Validation de l'adresse `loc`
2. Appel de la fonction système `time(NULL)` côté noyau
3. Écriture de la valeur 64 bits dans la mémoire utilisateur
4. Retour de 0 en cas de succès

<div class="callout callout-note">
    <div class="callout-title">Temps réel vs Ticks</div>
    <div class="callout-content">
        <code>time</code> retourne le temps réel en secondes, tandis que <code>GetCurrentTick</code>
        retourne des ticks système internes. Utilisez <code>time</code> pour mesurer des durées
        en secondes/minutes/heures réelles.
    </div>
</div>

## Paramètres

### `loc`

Pointeur vers une variable où stocker le temps actuel.

**Type** : `time_t *` (équivalent à `long long *`)
**Direction** : OUT
**Registre** : `$4`
**Contraintes** : Doit pointer vers une zone mémoire valide de 8 octets minimum

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `0` | Succès, le temps a été écrit dans `loc` |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 14 | `E_FAULT` | `loc` pointe vers une adresse invalide |

## Implémentation

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/userIO.cc:handle_SC_time()`

### Flux d'exécution

```
time(loc)
        │
        ▼
    start.S: time
        │ charge $4 = loc
        │ charge $2 = SC_time (42)
        ▼
    syscall SC_time
        │
        ▼
    handle_SC_time()
        │ ├─ lit adresse depuis $4
        │ ├─ valide l'adresse utilisateur
        │ ├─ appelle time(NULL) côté noyau
        │ ├─ écrit les 32 bits bas à loc
        │ └─ écrit les 32 bits hauts à loc+4
        ▼
    [retourne 0 dans $2]
```

### Détails techniques

La valeur `time_t` est stockée sur 64 bits en mémoire utilisateur :
- Les 32 bits de poids faible sont écrits à l'adresse `loc`
- Les 32 bits de poids fort sont écrits à l'adresse `loc + 4`

Cela permet de supporter des timestamps au-delà de l'année 2038 (qui sait...).

## Exemples

### Exemple 1 : Afficher le temps actuel

```c
#include "syscall.h"

int main() {
    time_t now;

    if (time(&now) == 0) {
        PutString("Temps actuel (secondes depuis epoch): ", 38);
        PutInt((int)now);
        PutChar('\n');
    } else {
        PutString("Erreur lors de la lecture du temps\n", 35);
    }

    return 0;
}
```

### Exemple 2 : Mesurer le temps d'exécution

```c
#include "syscall.h"

int main() {
    time_t start, end;
    int elapsed;

    time(&start);

    // Opération à mesurer
    int i;
    for (i = 0; i < 1000; i++) {
        PutChar('.');
    }
    PutChar('\n');

    time(&end);

    elapsed = (int)(end - start);
    PutString("Temps ecoule: ", 14);
    PutInt(elapsed);
    PutString(" secondes\n", 10);

    return 0;
}
```


### Exemple 3 : Afficher le temps en format lisible

```c
#include "syscall.h"

int main() {
    time_t now;
    int seconds, minutes, hours;

    time(&now);

    // Calcul des heures/minutes/secondes du jour (UTC)
    seconds = (int)(now % 60);
    minutes = (int)((now / 60) % 60);
    hours = (int)((now / 3600) % 24);

    PutString("Heure UTC: ", 11);
    PutInt(hours);
    PutChar(':');
    PutInt(minutes);
    PutChar(':');
    PutInt(seconds);
    PutChar('\n');

    return 0;
}
```

## Cas d'usage

### Mesure de performances

`time` est utilisé pour :
- Mesurer le temps d'exécution de transferts de fichiers
- Calculer le débit (throughput) en octets/seconde
- Comparer les performances de différentes implémentations

### Horodatage

`time` permet de :
- Horodater des événements dans les logs
- Enregistrer des timestamps dans des fichiers
- Calculer des délais d'expiration

## Thread-safety

`time` est thread-safe. Chaque thread peut appeler `time` indépendamment.

## Différences avec POSIX

| Aspect | POSIX time() | Nachos time() |
|--------|-------------|---------------|
| Signature | `time_t time(time_t *tloc)` | `int time(time_t *loc)` |
| Retour sur succès | Retourne aussi le temps | Retourne 0 |
| Retour sur erreur | `(time_t)-1` | `-1` (int) |
| Paramètre NULL | Accepté | Non supporté |

## Voir aussi

<div class="callout callout-see-also">
    <div class="callout-content">

- [GetCurrentTick](./GetCurrentTick.md) - Obtenir le tick système courant
- [Sleep](./Sleep.md) - Dormir pendant un nombre de ticks
- [SleepUntil](./SleepUntil.md) - Dormir jusqu'à un instant précis

    </div>
</div>

## Auteurs

Alioune Badara DIENE, 21 Jan 2026

## Dernière révision

21 Jan 2026
