# close

`close` - Termine une connexion


## Synopsis

```c
#include "syscall.h"

int close(int id);
```

## Description

`close` permet de terminer une connexion ou stopper un listener

## Comportement nominal

- récupère la connexion
- commence à fermer la connexion
- la libère

## Cas particuliers

- **id < 0** : Retourne -1, `errno = E_INVAL`
- **mgrr == nullptr** : Retourne -1, `errno = E_NOSYS`

## Paramètres

### `id`

Identifiant de la connexion ou du listener

**Type** : `int`  
**Direction** : IN  
**Registre** : `$4`  
**Contraintes** : L'identifiant doit exister et être accessible.

## Valeur de retour

**Type** : `int` (registre `$2`)

| Valeur | Signification |
|--------|---------------|
| `connId` | Identifiant de connexion |
| `-1` | Erreur (consulter `errno`) |

## Codes d'erreur

| errno | Constante | Condition |
|-------|-----------|-----------|
| 1 | `E_INVAL` | Paramètre invalide |

### Localisation du code

- **Stub utilisateur** : `code/test/start.S`
- **Handler noyau** : `code/userprog/usernetwork.cc:handle_SC_close()`
- **Implémentation** : `code/network/connectionmanager.cc:Close(...)`

### Flux d'exécution

```
close(id)
        │
        ▼
    start.S: close
        │ charge $4 = id
        ▼
    syscall SC_close
        │
        ▼
    handle_SC_close()
        │ ├─ lit $4
        │ ├─ GetConnectionManager()
        │ └─ CloseListener(id)
        ▼
    Close(id)
        │ ├─ GetConnection(id)
        │ ├─ InitiateClose()
        │ ├─ SetState(CONN_TERMINATED)
        │ └─ FreeConnection(id)
        ▼
```

## Exemples

### Exemple : Fermeture simple d'une connexion

```c
#include "syscall.h"

int main(){
    ...
    close(connId);
}
```
## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## VOIR AUSSI

- [Accept](./Accept.md) - Accepter une connexion
- [Connect](./Connect.md) - Connexion à une autre machine
- [Listen](./Listen.md) - Ecouter les demandes de connexions
- [sendto](./Send.md) - Envoyer des données
- [recvfrom](./Recv.md) - Recevoir des données

## Auteurs

Victor, 21 Jan 2026

## Dernière révision

21 Jan 2026
