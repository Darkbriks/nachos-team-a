# Liste des codes errno

Documentation détaillée de tous les codes d'erreur utilisés dans NachOS.

## Définitions

Les codes d'erreur sont définis dans `code/userprog/syscall.h` :

```c
#define E_SUCCESS       0   /* No error */
#define E_INVAL         1   /* Invalid argument */
#define E_FAULT         2   /* Bad address / memory access error */
#define E_OVERFLOW      3   /* Arithmetic overflow */
#define E_IO            4   /* I/O error */
#define E_FORMAT        5   /* Invalid format */
#define E_EOF           6   /* End of file */
#define E_NOMEM         7   /* Out of memory */
#define E_RANGE         8   /* Result out of range */
#define E_NOSPC         9   /* No such process */
```

## Codes d'erreur détaillés

---

### E_SUCCESS (0)

**Nom** : Succès (pas d'erreur)

**Description** : Opération réussie, aucune erreur.

**Quand positionné** :
- Automatiquement par les stubs en cas de succès
- `errno` est mis à 0 après chaque appel système réussi

**Utilisation** :
```c
int result = GetString(buffer, 100);
if (result != -1) {
    int err = GetLastError();  // err = 0 (E_SUCCESS)
}
```

---

### E_INVAL (1)

**Nom** : Argument invalide

**Description** : Un ou plusieurs arguments passés à l'appel système sont invalides.

**Causes courantes** :
- Taille négative (`n < 0`)
- Valeur hors limites
- Paramètre incohérent
- Format de données incorrect

---

### E_FAULT (2)

**Nom** : Adresse mémoire invalide

**Description** : L'adresse mémoire fournie est invalide ou inaccessible.

**Causes courantes** :
- Pointeur NULL
- Adresse négative
- Adresse hors de l'espace utilisateur
- Page non mappée

**Note** : Dans NachOS actuel, la validation d'adresse est incomplète. Beaucoup de cas invalides ne sont pas détectés et causent des crashes.

---

### E_OVERFLOW (3)

**Nom** : Dépassement arithmétique

**Description** : Une opération arithmétique a causé un dépassement de capacité. Peut également être utilisé de manière préventive pour éviter des dépassements d'adresse.

**Causes courantes** :
- Addition qui dépasse INT_MAX
- Vérification `n > INT32_MAX - addr` (éviter overflow d'adresse)

---

### E_IO (4)

**Nom** : Erreur d'entrée/sortie

**Description** : Erreur lors d'une opération d'entrée/sortie physique.

**Causes courantes** :
- Défaillance hardware (non simulée dans NachOS)
- Timeout de périphérique
- Erreur de transmission

**Note** : Actuellement non utilisé, notamment car le matériel simulé ne génère pas d'erreurs I/O.
---

### E_FORMAT (5)

**Nom** : Format invalide

**Description** : Format de données incorrect ou non reconnu.

**Causes courantes** :
- Fichier corrompu
- En-tête invalide
- Structure de données mal formée

---

### E_EOF (6)

**Nom** : Fin de fichier

**Description** : Tentative de lecture au-delà de la fin d'un fichier.

**Causes courantes** :
- `Read` sur fichier épuisé
- `GetChar` sur stdin redirigé depuis fichier terminé

**Note** : `GetChar` retourne EOF (-1) comme valeur, pas via errno.

---

### E_NOMEM (7)

**Nom** : Mémoire insuffisante

**Description** : Allocation mémoire échouée, plus assez de mémoire disponible.

**Causes courantes** :
- Allocation de pages échouée
- Heap saturé
- Trop de threads créés

---

### E_RANGE (8)

**Nom** : Résultat hors limites

**Description** : Le résultat d'une opération est hors des limites acceptables.

**Causes courantes** :
- Résultat mathématique hors intervalle
- Valeur retournée trop grande ou trop petite

---

### E_NOSPC (9)

**Nom** : Aucun processus correspondant (No such process)

**Description** : Aucun processus/thread trouvé avec l'ID spécifié.

**Causes courantes** :
- `JoinThread` avec TID invalide
- Thread déjà terminé
- TID jamais créé

## Voir aussi

- [Gestion des erreurs](./errors.md) - Vue d'ensemble du mécanisme errno
- [Console I/O](./console/README.md) - Appels avec gestion d'erreur
- [my_stdlib](../userspace/my_stdlib.md) - Fonctions print_error()

## Auteurs

Antoine, 20 Dec 2025

## Dernière révision

20 Dec 2025 par Antoine