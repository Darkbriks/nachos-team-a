# Gestion de connexion - Protocole TCP-like

Cette page documente la gestion de connexion dans NachOS ReliablePost, basée sur une machine à états de type TCP.

## Vue d'ensemble

Le protocole de connexion implémente une machine à états inspirée de TCP, avec établissement en 3-way handshake et fermeture en 4-way handshake. Il gère également les cas d'ouverture et fermeture simultanées.

## Machine à états complète

![Machine d'états TCP](./connection_state_machine.png)

### États de connexion

#### États initiaux

| État          | Description                              | Couleur    |
|---------------|------------------------------------------|------------|
| `CONN_CLOSED` | Aucune connexion n'existe                | Rouge      |
| `CONN_LISTEN` | Serveur en attente de connexion entrante | Vert clair |

#### Établissement de connexion

| État                | Description                                       | Couleur |
|---------------------|---------------------------------------------------|---------|
| `CONN_SYN_SENT`     | SYN envoyé, attente de SYN-ACK (client)           | Jaune   |
| `CONN_SYN_RECEIVED` | SYN reçu, SYN-ACK envoyé, attente d'ACK (serveur) | Jaune   |

#### Connexion établie

| État               | Description                                      | Couleur  |
|--------------------|--------------------------------------------------|----------|
| `CONN_ESTABLISHED` | Connexion établie, transfert de données possible | Vert     |

#### Fermeture de connexion

| État              | Description                               | Couleur |
|-------------------|-------------------------------------------|---------|
| `CONN_FIN_WAIT_1` | FIN envoyé, attente d'ACK ou FIN          | Rose    |
| `CONN_FIN_WAIT_2` | FIN acquitté, attente du FIN du peer      | Rose    |
| `CONN_CLOSE_WAIT` | FIN reçu, attente de Close() local        | Orange  |
| `CONN_CLOSING`    | FIN envoyé et reçu (fermeture simultanée) | Rose    |
| `CONN_LAST_ACK`   | Attente du dernier ACK après envoi de FIN | Orange  |
| `CONN_TIME_WAIT`  | Attente avant nettoyage final (2MSL)      | Violet  |

#### État terminal

| État              | Description                                          | Couleur |
|-------------------|------------------------------------------------------|---------|
| `CONN_TERMINATED` | Connexion complètement fermée, ressources libérables | Gris    |

## Cas d'erreur : Reset (RST)

Si un paquet est reçu pour une connexion inexistante, un `RST` est envoyé :

```
Machine A                     Machine B
    |                              |
    |--- DATA (pour conn invalide) >|
    |                              |
    |<-- RST -----------------------|
    | État: TERMINATED             |
```

Le `RST` force immédiatement la transition à `CONN_TERMINATED` sans handshake.

## Limitations

- **Pas de véritable TIME_WAIT** : La durée 2MSL n'est pas implémentée, passage immédiat à `TERMINATED`
- **Pas de buffer de réordonnancement** : Les paquets hors séquence sont ignorés
- **Une seule connexion par ReliablePost** : Pas de multiplexage

## Voir aussi

- [Transmission fiable](./ReliableTransmission.md) - Envoi/réception de données
- [Vue d'ensemble](./Network.md) - Introduction au réseau
- [Diagrammes](./packet_ordering.png) - Gestion des paquets

## Auteurs

Alioune Badara DIENE
Antoine

## Dernière révision

21 Jan 2026