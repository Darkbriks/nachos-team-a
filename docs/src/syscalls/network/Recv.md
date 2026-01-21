# recvfrom

`recvfrom` - Recevoir des données.

## SYNOPSIS
```c
#include "syscall.h"

int recvfrom(int connId, char* buffer, int size);
```

## DESCRIPTION

`recvfrom` reçoit les données envoyées par une machine connectée à la nôtre.


### Comportement nominal

- Regarde si l'id de connexion et la taille sont valides
- Récupère le gestionnaire le de connexion
- Regarde si le message a été découpé
- Crée un buffer interne et stock les données dedans
- Copie le buffer interne vers le buffer de l'utilisateur


### Cas particuliers

- **connId < 0** : Retourne -1, `errno = E_INVAL`
- **size <= 0** : Retourne -1, `errno = E_INVAL`
- **type de message == MSG_CHUNK_BEGIN** : 
```
recvfrom(connId, buffer, size)
        │
        ▼
    start.S: recvfrom
        │ charge $4 = connId
        │ charge $5 = buffer
        │ charge $6 = size
        ▼
    syscall SC_sendto
        │
        ▼
    handle_SC_sendto()
        │ ├─ lit $4
        │ ├─ lit $5
        │ ├─ lit $6
        │ ├─ vérifie les valeurs de connId, buffer et send
        │ └─ reçoit le premier message de type MSG_CHUNK_BEGIN
        │ └─ crée un buffer avec pour taille, la taille de MAX_PUT_STRING si la taille des données à recevoir est supérieure, la taille des données sinon
        │ └─ tant que le message de type MSG_CHUNK_END n'est pas reçu, remplit le buffer interne et le copie dans celui de l'utilisateur pour reconstruire le message
        ▼

```


## PARAMÈTRES

### `connId`
Id de connexion

**Type** : `int`
**Direction** : IN
**Registre** : `$4`
**Contraintes** :
- Doit être ≥ 0


### `buffer`
Buffer pour récupérer les données

**Type** : `char*`
**Direction** : IN
**Registre** : `$5`
**Contraintes** :
- Doit être une adresse valide


### `size`
Taille à recevoir

**Type** : `int`
**Direction** : IN
**Registre** : `$6`
**Contraintes** :
- Doit être > 0

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : Taille des données reçues

**En cas d'erreur** : `-1` et `errno` est défini

## CODES D'ERREUR

| errno | Constante   | Condition                                             |
|-------|-------------|-------------------------------------------------------|
| 1     | `E_INVAL`   | `connId < 0` ou  `size < 1`                           |
| 2     | `E_FAULT`   | Adresse invalide                                      |
| 22    | `E_NOSYS`   | Gestionnaire de connexion non défini                  |


## FAILLES ET VULNÉRABILITÉS

Aucune faille de sécurité connue.

## BUGS CONNUS

Aucun bug connu à ce jour.

## VOIR AUSSI

- [Accept](./Accept.md) - Accépter une connexion
- [Close](./Close.md) - Fermer une connexion
- [Connect](./Connect.md) - Connexion à une autre machine
- [Listen](./Listen.md) - Ecouter les demandes de connexions
- [sendto](./Send.md) - Envoyer des données

## AUTEURS

Noa, 21 Jan 2026

## DERNIÈRE RÉVISION

21 Jan 2026
