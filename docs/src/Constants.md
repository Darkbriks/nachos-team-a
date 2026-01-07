# Constantes du projet

Description des principales constantes utilisées dans le projet Nachos.

---

## Processus et Threads

### MAX_THREAD

**Valeur** : `30`

**Description** : Nombre maximum de threads par processus (actifs ou non)

**Fichier** : `code/userprog/process.h`

---

### MAX_PROCESS

**Valeur** : `15`

**Description** : Nombre maximum de processus dans la machine (actifs ou non)

**Fichier** : `code/userprog/process.h`

---

### UserStackSize

**Valeur** : `16384`

**Description** : Taille en octets de la pile pour les programmes utilisateur

**Fichier** : `code/userprog/addrspace.h`

---

## Sémaphores

### INITIAL_SEMAPHORE_TABLE_SIZE

**Valeur** : `16`

**Description** : Taille initiale de la table de sémaphores par processus (peut s'étendre dynamiquement)

**Fichier** : `code/userprog/addrspace.h`

---

### MAX_SEMAPHORES_PER_PROCESS

**Valeur** : `512`

**Description** : Nombre maximum de sémaphores par processus

**Fichier** : `code/userprog/addrspace.h`

---

## Mémoire

### PageSize

**Valeur** : `SectorSize = 128` octets

**Description** : Taille d'une page mémoire

**Fichier** : `code/machine/machine.h`

---

### NumPhysPages

**Valeur** : `8192`

**Description** : Nombre de pages physiques disponibles dans la machine

**Fichier** : `code/machine/machine.h`

---

### MemorySize

**Valeur** : `NumPhysPages * PageSize = 8192 * 128 = 1 048 576` octets

**Description** : Taille totale de la mémoire physique

**Fichier** : `code/machine/machine.h`

---

### TLBSize

**Valeur** : `4`

**Fichier** : `code/machine/machine.h`

---

## Console I/O

### MAX_STRING_SIZE

**Valeur** : `256`

**Description** : Taille maximale d'une chaîne de caractères pour GetString (buffer interne)

**Fichier** : `code/threads/system.h`

---

### MAX_PUT_STRING

**Valeur** : `8192`

**Description** : Nombre maximum d'octets pouvant être écrits en une seule fois avec PutString

**Fichier** : `code/userprog/exception.h`

---

### MAX_PATH_SIZE

**Valeur** : `1024`

**Description** : Longueur maximale d'un chemin de fichier

**Fichier** : `code/threads/system.h`

---

## Auteurs

Tommy, 05 Jan 2026  
Antoine, 07 Jan 2026

## Dernière révision

07 Jan 2026