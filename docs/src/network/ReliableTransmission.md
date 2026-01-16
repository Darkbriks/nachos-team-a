# SendReliable / ReceiveReliable

`SendReliable` - Envoie un message avec garantie de livraison
`ReceiveReliable` - Reçoit un message et envoie un acquittement

## SYNOPSIS

```cpp
#include "reliablepost.h"

// Envoi
unsigned int SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

// Réception
void ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);
```

## DESCRIPTION

### SendReliable

`SendReliable` envoie un message vers la machine distante avec garantie de livraison. Le message est placé dans une file d'attente et retransmis automatiquement jusqu'à réception d'un acquittement (ACK).

**Comportement :**
- Non-bloquant : retourne immédiatement après mise en file
- Retransmission automatique après `TEMPO` ticks sans ACK
- Abandon après `MAXREEMISSIONS` tentatives

### ReceiveReliable

`ReceiveReliable` reçoit un message de la boîte aux lettres spécifiée et envoie automatiquement un ACK à l'émetteur.

**Comportement :**
- Bloquant : attend qu'un message soit disponible
- ACK envoyé automatiquement

## PARAMÈTRES

### SendReliable

| Paramètre | Type | Description |
|-----------|------|-------------|
| `pktHdr` | `PacketHeader` | En-tête réseau (contient `to` : machine destinataire) |
| `mailHdr` | `MailHeader` | En-tête mail (contient `to`, `from` : boîtes aux lettres, `length`) |
| `data` | `const char*` | Données à envoyer |

### ReceiveReliable

| Paramètre | Type | Description |
|-----------|------|-------------|
| `box` | `int` | Numéro de la boîte aux lettres à écouter |
| `pktHdr` | `PacketHeader*` | [OUT] En-tête réseau du message reçu |
| `mailHdr` | `MailHeader*` | [OUT] En-tête mail du message reçu |
| `data` | `char*` | [OUT] Buffer pour les données reçues |

## VALEUR DE RETOUR

### SendReliable

- **Succès** : Numéro de séquence attribué au message (> 0)
- **Échec** : `0` si non connecté ou file pleine

### ReceiveReliable

Aucune valeur de retour. Les données sont écrites dans les paramètres de sortie.

## DIAGRAMME DE FONCTIONNEMENT

![Transmission fiable](./reliable_transmission.png)

## NOTES

- **Thread-safety** : Les appels sont protégés par un verrou interne
- **Ordre** : Les messages sont délivrés dans l'ordre d'envoi
- **Fiabilité** : Fonctionne même avec `-l 0.5` (50% de perte)

## VOIR AUSSI

- [Vue d'ensemble](./Network.md) - Introduction au réseau
- [Gestion de connexion](./ConnectionManagement.md) - Connect/Close
