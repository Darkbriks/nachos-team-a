# Vue d'ensemble du réseau

Cette page fournit une vue d'ensemble de la couche réseau fiable de NachOS.

## Introduction

NachOS fournit une couche réseau simulée permettant à plusieurs instances de communiquer entre elles. Par défaut, cette couche est **non fiable** : les paquets peuvent être perdus.

La classe `ReliablePost` implémente une couche de transmission fiable au-dessus de ce réseau, inspirée de TCP/IP.

## Architecture

```
┌─────────────────────────────────┐
│     Programme utilisateur       │
├─────────────────────────────────┤
│         ReliablePost            │  ← Transmission fiable
├─────────────────────────────────┤
│          PostOffice             │  ← Boîtes aux lettres
├─────────────────────────────────┤
│           Network               │  ← Réseau non fiable
└─────────────────────────────────┘
```

## Démarrage rapide

### Lancer deux machines

```bash
# Terminal 1 - Machine 0
./nachos-step6 -m 0 -l 1.0 -R 1

# Terminal 2 - Machine 1
./nachos-step6 -m 1 -l 1.0 -R 0
```

### Options de la ligne de commande

| Option | Description | Exemple |
|--------|-------------|---------|
| `-m N` | Identifiant de la machine | `-m 0` |
| `-l P` | Fiabilité du réseau (0.0 à 1.0) | `-l 0.5` (50% de perte) |
| `-R N` | Test de transmission fiable vers machine N | `-R 1` |

## Fonctionnalités

### Transmission fiable

- Acquittements automatiques (ACK)
- Retransmission en cas de timeout
- Détection des duplicatas
- Numéros de séquence

### Gestion de connexion

- Établissement de connexion (handshake)
- Fermeture gracieuse
- Détection de déconnexion

## API disponible

| Méthode | Description |
|---------|-------------|
| `Connect()` | Établit une connexion avec la machine distante |
| `IsConnected()` | Vérifie si la connexion est active |
| `Close()` | Ferme la connexion proprement |
| `SendReliable()` | Envoie un message avec garantie de livraison |
| `ReceiveReliable()` | Reçoit un message |

## Constantes importantes

```c
#define TEMPO 10000           // Timeout retransmission (ticks)
#define MAXREEMISSIONS 100    // Tentatives max avant abandon
#define MAX_PENDING_MSGS 10   // Messages en attente d'ACK max
```

## Voir aussi

- [Transmission fiable](./ReliableTransmission.md) - API SendReliable/ReceiveReliable
- [Gestion de connexion](./ConnectionManagement.md) - API Connect/Close
