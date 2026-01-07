# Liste des headers importants

Documentation détaillée de tous les headers utilisés dans NachOS.

## Définitions

Les headers définis dans `code/userprog/process.h` :

```c
#define MAX_THREAD 30 
#define MAX_PROCESS 15 
```

---

### MAX_THREAD 

**Description** : Indique le nombre maximum de threads rattaché à un processus ( actif ou non ) 

---

### MAX_PROCESS


**Description** : Indique le nombre maximum de processus dans la machine ( actif ou non ). Nécessite suffisament de pages physiques

---

## Définitions

Les headers définis dans `code/userprog/addrspace.h` :

```c
#define UserStackSize 16384 
#define INITIAL_SEMAPHORE_TABLE_SIZE 16
#define MAX_SEMAPHORES_PER_PROCESS 512 
```


---

### UserStackSize

**Description** : Indique le nombre d'octets disponible pour la stack des codes utilisateurs 

---

### INITIAL_SEMAPHORE_TABLE_SIZE 


**Description** : Indique le nombre maximum de sémaphores pouvant être créés par défaut. Peut être réajusté via un syscall.

---

### MAX_SEMAPHORES_PER_PROCESS


**Description** : Indique le nombre maximum de sémaphores pouvant être créés par défaut pour un processus. 

---

## Définitions

Les headers définis dans `code/machine/machine.h` :

```c
#define NumPhysPages 8192
#define MemorySize (NumPhysPages * PageSize)
#define TLBSize 4
```

---

### NumPhysPages

**Description** : Indique le nombre de pages physiques allouées pour la VM nachos 

---

### MemorySize


**Description** : Le nombre total d'octets disponible pour Nachos

---

### TLBSize


**Description** :  TODO

---

---

## Auteurs

Tommy , 5 Jan 2026

## Dernière révision

7 Jan 2026 par Tommy 

