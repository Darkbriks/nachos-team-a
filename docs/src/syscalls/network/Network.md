# Appels système pour le réseau

Cette section documente les appels système liés au réseau dans Nachos, permettant d'établir une connexion, inspirée du protocole TCP/IP.

## Vue d'ensemble

Les appels système pour le réseau permettent de :
- Etablir une connexion
- Définir un port d'écoute
- Accepter une connexion sur un port d'écoute
- Envoyer et/ou recevoir des données
- Terminer une connexion

## Concepts fondamentaux

### Transmission fiable

Dans Nachos, les transmissions de messages suivent un certain protocole afin d'éviter le plus possible la perte de messages : un timeout est défini. Si un message n'est pas reçu après que le timeout soit écoulé, alors le message est réémis. Un nombre de tentatives est défini. Au-delà de ce nombre de tentatives, l'envoi aura échoué. Cependant, le timeout et le nombre de tentatives baissent drastiquement la probabilité de perte d'un message.

### Transmission de messages de longueur variable

Afin de ne pas avoir de limite pour la taille d'un message, nous définissons un principe de découpage de messages : si un message est trop long, alors on l'envoie en plusieurs parties à l'aide d'un buffer. Une fois les différentes parties du message reçues, ce message est reconstitué en un seul message et est considéré comme tel.

## Liste des appels système

### [connect](./Connect.md)

Etablit une connexion à un serveur distant

**Utilisation** :
```c
int connect(int remoteAddr, int remotePort, int localPort);
```

### [listen](./Listen.md)

Définit le port donné en argument comme port d'écoute

**Utilisation** :
```c
int listen(int port);
```

### [accept](../Accept.md)

Accepte une connexion sur un port d'écoute

**Utilisation** :
```c
int accept(int listenerId, int timeoutMs);
```

### [sendto](../Send.md)

Envoie des données à une machine connectée à la nôtre.

**Utilisation** :
```c
int sendto(int connId, char* data, int size);
```

### [recvfrom](../Recv.md)

Reçoit les données envoyées par une machine connectée à la nôtre.

**Utilisation** :
```c
int recvfrom(int connId, char* buffer, int size);
```

### [close](../Close.md)

Termine une connexion ou stoppe un listener.

**Utilisation** :
```c
int close(int id);
```

## Limitations et contraintes

## Codes d'erreur courants

| errno | Constante | Condition |
|-------|-----------|-----------|
| 105 | `E_TIMEOUT` | Temps écoulé |
| 100 | `E_REFUSED` | Connexion refusée |
| 1 | `E_INVAL` | Paramètre invalide |
| 107 | `E_NOPORT` | Port non défini |
| 101 | `E_NOTCONN` | Echec de connexion |
| 102 | `E_ADDRINUSE` | Port déjà utilisé |

## Voir aussi

- [connect](./Connect.md) - Etablit une connexion
- [listen](./Listen.md) - Définit un port d'écoute
- [accept](./Accept.md) - Accepte une connexion
- [sendto](./Send.md) - Envoyer des données
- [recvfrom](./Recv.md) - Recevoir des données
- [close](./Close.md) - Termine une connexion

## Auteurs

Victor, 22 Jan 2026

## Dernière révision

22 Jan 2026
