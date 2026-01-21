# sendto

`sendto` - Envoyer des données.

## SYNOPSIS
```c
#include "syscall.h"

int sendto(int connId, char* data, int size);
```

## DESCRIPTION

`sendto` envoie des données à une machine connectée à la nôtre.


### Comportement nominal

- Regarde si l'id de connexion et la taille sont valides
- Récupère le gestionnaire le de connexion
- Regarde si le message doit-être découper et donc le découpe en cas de message trop long
- Crée un buffer interne et stock les données dedans
- Envoie le buffer interne à l'autre machine


### Cas particuliers

- **connId < 0** : Retourne -1, `errno = E_INVAL`
- **size <= 0** : Retourne -1, `errno = E_INVAL`
- **size > MAX_PUT_STRING** : 

```
sendto(connId, data, size)
        │
        ▼
    start.S: sendto
        │ charge $4 = connId
        │ charge $5 = data
        │ charge $6 = size
        ▼
    syscall SC_sendto
        │
        ▼
    handle_SC_sendto()
        │ ├─ lit $4
        │ ├─ lit $5
        │ ├─ lit $6
        │ ├─ vérifie les valeurs de connId, data et send
        │ └─ envoie un message vide de type MSG_CHUNK_BEGIN
        │ └─ tant que le message n'est pas totalement envoyé, découpe le message avec une taille maximale de MAX_PUT_STRING
        │ └─ crée un buffer avec pour taille, la taille envoyé
        │ └─ CopyFromUserRaw(buffer, données à envoyer + position dans la données, taille à envoyer)
        │ └─ gestionnaire_de_connexion->Send(id de connexion, buffer, taille à envoyer)
        ▼
    Send(id de connexion, buffer, taille à envoyer)
        │ ├─ tant que la taille du message est supérieure à la taille maximale d'un paquet, découpage du paquet
        │ ├─ ajoute un flag FLAG_MORE_FRAGMENTS si il reste des données, FLAG_END_OF_MESSAGE sinon
        │ ├─ connexion->QueueSend(données + total envoyé, taille du chunk, flags) 
        │ └─ Détruit le process finit
        ▼
    QueueSend(données + total envoyé, taille du chunk, flags)
        │ ├─ ...
        ▼
    handle_SC_sendto()
        │ ├─ envoie un message vide de type MSG_CHUNK_END
        │ ├─ retourne la taille envoyé
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


### `data`
Données à envoyer

**Type** : `char*`
**Direction** : IN
**Registre** : `$5`
**Contraintes** :
- Doit être une adresse valide


### `size`
Taille à envoyer

**Type** : `int`
**Direction** : IN
**Registre** : `$6`
**Contraintes** :
- Doit être > 0

## VALEUR DE RETOUR

**Type** : `int` (registre `$2`)

**En cas de succès** : Taille des données envoyées

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
- [recvfrom](./Recv.md) - Recevoir des données

## AUTEURS

Noa, 21 Jan 2026

## DERNIÈRE RÉVISION

21 Jan 2026
