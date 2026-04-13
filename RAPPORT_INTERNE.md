# Rapport Interne — PolyManager DIC2
### (Document de travail — non soumis)

**Module :** Programmation Orientée Objet Avancée  
**Filière :** DIC 2  
**Sujet :** Gestion de polynômes d'une variable — implémentation en C et Java  

---

## Membres du groupe

| Nom complet | Filière | Branche |
|-------------|---------|---------|
| Mouhamadou Madeniyou SALL | IABD | `dev-c-gc-sall` / `dev-java-gc-sall` |
| Adama SIDIBE | INFORMATIQUE | `dev-c-parser-sidibe` |
| Abdoul Aziz KANE | TR | `dev-c-core-kane` |
| Anna NDOYE | SSI | `dev-c-rec-anna` / `dev-java-parser-anna` |
| Mame Aïssatou Samb NGOM | SSI | `dev-c-advanced-ngom` / `dev-java-core-ngom` |
| Fatoumata BARRO | TR | `dev-c-math-barro` / `dev-java-rec-barro` |

---

## Table des matières

1. [Structure de données](#1-structure-de-données)
2. [Question 1 & 2 — Analyseur syntaxique et codage](#2-analyseur-syntaxique-et-codage)
3. [Question 3 — Affichage](#3-affichage)
4. [Question 4 — Codage par degré décroissant](#4-codage-par-degré-décroissant)
5. [Question 5 — Évaluation](#5-évaluation)
6. [Question 6 — Opérations arithmétiques](#6-opérations-arithmétiques)
7. [Question 7 — Garbage Collector](#7-garbage-collector)
8. [Question 8 — Versions récursives](#8-versions-récursives)
9. [Compilation et exécution](#9-compilation-et-exécution)
10. [Exemples d'exécution](#10-exemples-dexécution)

---

# PARTIE I — LANGAGE C

---

## 1. Structure de données

### `polynome.h` — Fichier d'en-tête complet

```c
#ifndef POLYNOME_H
#define POLYNOME_H
```
> `#ifndef` / `#define` : garde d'inclusion. Empêche que ce fichier soit inclus deux fois dans la même compilation, ce qui provoquerait des erreurs de redéfinition.

```c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
```
> Bibliothèques standard nécessaires :
> - `stdio.h` : `printf`, `fprintf`, `fgets`
> - `stdlib.h` : `malloc`, `free`, `exit`, `EXIT_FAILURE`
> - `ctype.h` : `isdigit` (tester si un caractère est un chiffre)
> - `math.h` : `pow` (puissance), `fabs` (valeur absolue flottante)
> - `string.h` : `strcspn` (trouver la position du `\n` dans une chaîne)

```c
typedef struct Monome {
    double coefficient;
    int    exposant;
    struct Monome *suivant;
    struct Monome *general;
    int utile;
} Monome, *POINTEUR;
```
> **Structure centrale du projet.**
> - `coefficient` : valeur réelle du monôme (ex : `-4.5` dans `-4.5X^5`)
> - `exposant` : degré du monôme (ex : `5` dans `-4.5X^5`)
> - `suivant` : pointeur vers le maillon suivant du **polynôme** (liste triée par degré décroissant)
> - `general` : pointeur vers le maillon suivant de la **liste globale** de tous les maillons jamais alloués (utilisé par le GC, Q7)
> - `utile` : entier utilisé comme booléen par le GC ; `1` = maillon à conserver, `0` = maillon à libérer
>
> Le `typedef` crée deux alias : `Monome` pour la structure, et `POINTEUR` pour `Monome*`. Ainsi `POINTEUR p` est équivalent à `Monome *p`.

```c
extern POINTEUR tousLesMaillons;
#define MAX_POLY 100
extern POINTEUR polyUtile[MAX_POLY];
extern int      nbPolyUtile;
```
> Variables globales déclarées avec `extern` : elles sont *définies* dans `polynome.c` mais *déclarables* ici pour que tout fichier incluant `polynome.h` y ait accès.
> - `tousLesMaillons` : tête de la liste chaînée de **tous** les maillons alloués (via le champ `general`)
> - `MAX_POLY 100` : constante de compilation, taille max du tableau de polynômes protégés
> - `polyUtile[]` : tableau des pointeurs de tête des polynômes à protéger lors du GC
> - `nbPolyUtile` : nombre de polynômes actuellement enregistrés dans `polyUtile`

```c
POINTEUR creerMonome(double coef, int exp);
void     insererTrie(POINTEUR *tete, double coef, int exp);
void     analyserPolynome(char *s, POINTEUR *poly);
void     afficherPolynome(POINTEUR p);
double   eval(POINTEUR p, double x);
void     enregistrerPoly(POINTEUR p);
void     recycler(void);
POINTEUR plus(POINTEUR a, POINTEUR b);
POINTEUR moins(POINTEUR a, POINTEUR b);
POINTEUR fois(POINTEUR a, POINTEUR b);
POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste);
POINTEUR plus_rec(POINTEUR a, POINTEUR b);
POINTEUR moins_rec(POINTEUR a, POINTEUR b);
```
> Déclarations (prototypes) de toutes les fonctions du projet. Elles permettent au compilateur de vérifier les types lors des appels avant que les définitions complètes soient vues.

---

## 2. Analyseur syntaxique et codage

### `polynome.c` — Variables globales

```c
POINTEUR tousLesMaillons  = NULL;
POINTEUR polyUtile[MAX_POLY];
int      nbPolyUtile       = 0;
```
> **Définitions** (pas seulement déclarations) des variables globales.
> - `tousLesMaillons = NULL` : la liste globale est vide au démarrage
> - `polyUtile[MAX_POLY]` : tableau de 100 pointeurs, non initialisé explicitement (les cases vaudront `NULL` car c'est une variable globale en C)
> - `nbPolyUtile = 0` : compteur mis à zéro

---

### Fonction `creerMonome`

```c
POINTEUR creerMonome(double coef, int exp) {
```
> Alloue un nouveau maillon et retourne son adresse. Prend le coefficient et l'exposant en paramètres.

```c
    POINTEUR p = (POINTEUR)malloc(sizeof(Monome));
```
> `malloc(sizeof(Monome))` : demande au système d'exploitation un bloc mémoire de la taille exacte d'une structure `Monome`. Retourne `void*`, casté en `POINTEUR` (`Monome*`).

```c
    if (!p) {
        fprintf(stderr, "Erreur : malloc a échoué\n");
        exit(EXIT_FAILURE);
    }
```
> Si `malloc` retourne `NULL` (mémoire épuisée), on affiche un message sur la sortie d'erreur (`stderr`) et on termine le programme avec un code d'erreur. C'est la seule vérification d'erreur d'allocation du projet.

```c
    p->coefficient = coef;
    p->exposant    = exp;
    p->suivant     = NULL;
```
> Initialisation des champs polynôme. `suivant = NULL` car ce maillon n'est pas encore inséré dans une liste.

```c
    p->utile   = 0;
    p->general = tousLesMaillons;
    tousLesMaillons = p;
```
> **Q7 — Enregistrement dans la liste globale.**
> - `utile = 0` : par défaut non marqué (à libérer si le GC tourne)
> - `p->general = tousLesMaillons` : le nouveau maillon pointe vers l'ancienne tête de la liste globale
> - `tousLesMaillons = p` : le nouveau maillon devient la nouvelle tête
>
> Résultat : tout maillon alloué via `creerMonome` est automatiquement ajouté en tête de `tousLesMaillons`. Cette liste est **indépendante** de la liste `suivant` du polynôme.

```c
    return p;
}
```
> Retourne l'adresse du maillon fraîchement alloué.

---

### Fonction `insererTrie`

```c
void insererTrie(POINTEUR *tete, double coef, int exp) {
```
> Prend l'**adresse** du pointeur de tête (`POINTEUR *tete` = `Monome **tete`) pour pouvoir modifier la tête elle-même si nécessaire. C'est le passage par pointeur de pointeur classique en C pour les listes chaînées.

```c
    if (coef == 0.0) return;
```
> Un monôme de coefficient zéro ne contribue pas au polynôme : on ignore l'insertion. Cela assure que la liste ne contient jamais de termes nuls.

```c
    POINTEUR curr = *tete;
    while (curr != NULL) {
        if (curr->exposant == exp) {
            curr->coefficient += coef;
            return;
        }
        curr = curr->suivant;
    }
```
> **Fusion des degrés identiques.** On parcourt la liste existante ; si un maillon de même exposant existe déjà, on additionne simplement les coefficients et on sort. Cela évite les doublons (ex : `X^2 + 3X^2` doit donner `4X^2`, pas deux maillons séparés).

```c
    POINTEUR nouveau = creerMonome(coef, exp);
```
> Crée un nouveau maillon (Q7 : il est automatiquement enregistré dans `tousLesMaillons`).

```c
    if (*tete == NULL || exp > (*tete)->exposant) {
        nouveau->suivant = *tete;
        *tete = nouveau;
```
> **Insertion en tête** : si la liste est vide, ou si le nouveau degré est plus grand que la tête actuelle (rappel : liste triée par degrés décroissants), on insère avant la tête actuelle.

```c
    } else {
        POINTEUR prec = *tete;
        while (prec->suivant != NULL && prec->suivant->exposant > exp)
            prec = prec->suivant;
        nouveau->suivant = prec->suivant;
        prec->suivant = nouveau;
    }
```
> **Insertion au milieu ou en fin.** On avance `prec` jusqu'à trouver la bonne position (le premier maillon dont le suivant a un exposant ≤ `exp`). On insère alors `nouveau` entre `prec` et `prec->suivant`.

---

### Fonctions auxiliaires statiques de l'analyseur

Le mot-clé `static` devant ces fonctions signifie qu'elles ne sont visibles qu'à l'intérieur de `polynome.c`. Ce sont des détails d'implémentation interne.

#### `passerEspaces`

```c
static void passerEspaces(char *s, int *i) {
    while (s[*i] == ' ' || s[*i] == '\t') (*i)++;
}
```
> Avance l'indice `*i` dans la chaîne `s` tant qu'on rencontre des espaces ou des tabulations. Utilisé avant et après chaque élément syntaxique pour rendre le parser insensible aux espaces.

#### `lireNaturel`

```c
static int lireNaturel(char *s, int *i) {
    int n = 0;
    if (!isdigit((unsigned char)s[*i])) {
        fprintf(stderr, "Erreur de syntaxe : chiffre attendu\n");
        exit(EXIT_FAILURE);
    }
```
> Vérifie qu'on est bien sur un chiffre. `(unsigned char)` évite un comportement indéfini sur des caractères de valeur négative (caractères accentués, etc.) avec `isdigit`.

```c
    while (isdigit((unsigned char)s[*i])) {
        n = n * 10 + (s[*i] - '0');
        (*i)++;
    }
    return n;
}
```
> Lit les chiffres un par un et accumule la valeur entière avec la formule classique `n = n*10 + chiffre`. `s[*i] - '0'` convertit le caractère ASCII du chiffre en sa valeur numérique (ex : `'3' - '0' = 3`).

#### `lireNombre`

```c
static double lireNombre(char *s, int *i) {
    double n = (double)lireNaturel(s, i);
    if (s[*i] == '.') {
        (*i)++;
        double frac = 0.0, div = 10.0;
        while (isdigit((unsigned char)s[*i])) {
            frac += (s[*i] - '0') / div;
            div  *= 10.0;
            (*i)++;
        }
        n += frac;
    }
    return n;
}
```
> Lit la partie entière via `lireNaturel`, puis si un `.` suit, lit les décimales. `div` commence à 10, puis 100, 1000… pour positionner chaque chiffre à la bonne décimale. Ex : `4.5` → `4 + 5/10 = 4.5`.

#### `analyserMonome`

```c
static void analyserMonome(char *s, int *i, POINTEUR *poly, int signe) {
    double c = 1.0;
    int    e = 0;
    passerEspaces(s, i);
```
> Valeurs par défaut : coefficient `1.0` (pour `X` seul) et exposant `0` (pour un nombre seul). On commence par sauter les espaces.

```c
    if (isdigit((unsigned char)s[*i])) {
        c = lireNombre(s, i);
        passerEspaces(s, i);
        if (s[*i] == '*') {
            (*i)++;
            passerEspaces(s, i);
        }
        if (s[*i] == 'X' || s[*i] == 'x') {
            (*i)++;
            if (s[*i] == '^') { (*i)++; e = lireNaturel(s, i); }
            else e = 1;
        }
```
> **Cas 1 : le monôme commence par un chiffre.** On lit le nombre, on passe un `*` optionnel, puis on regarde si un `X` suit. Si oui : `X` seul → exposant 1, `X^n` → exposant `n`. Si non : c'est un monôme constant (exposant reste 0).

```c
    } else if (s[*i] == 'X' || s[*i] == 'x') {
        (*i)++;
        if (s[*i] == '^') { (*i)++; e = lireNaturel(s, i); }
        else e = 1;
```
> **Cas 2 : le monôme commence par `X`.** Le coefficient est 1 (valeur par défaut). On lit l'exposant s'il y en a un.

```c
    } else {
        fprintf(stderr, "Erreur de syntaxe : monôme invalide\n");
        exit(EXIT_FAILURE);
    }
    insererTrie(poly, c * signe, e);
}
```
> **Cas 3 : caractère inattendu** → erreur de syntaxe. Si tout va bien, on insère le monôme avec `c * signe` (applique le signe + ou −).

#### `analyserPolynome`

```c
void analyserPolynome(char *s, POINTEUR *poly) {
    int i = 0, signe = 1;
    passerEspaces(s, &i);
    if      (s[i] == '-') { signe = -1; i++; }
    else if (s[i] == '+') {             i++; }
```
> Point d'entrée public. Initialise l'indice à 0, signe positif par défaut. Gère le signe optionnel devant le premier monôme.

```c
    analyserMonome(s, &i, poly, signe);
    passerEspaces(s, &i);

    while (s[i] != '\0' && s[i] != '\n') {
        if      (s[i] == '+') { signe =  1; i++; }
        else if (s[i] == '-') { signe = -1; i++; }
        else break;
        analyserMonome(s, &i, poly, signe);
        passerEspaces(s, &i);
    }
}
```
> Lit le premier monôme (obligatoire selon la grammaire), puis boucle sur les monômes suivants séparés par `+` ou `−`. `break` en cas de caractère inattendu (fin de chaîne utile, ou erreur silencieuse après un `\0` ou `\n`).

---

## 3. Affichage

### Fonction `afficherPolynome`

```c
void afficherPolynome(POINTEUR p) {
    if (p == NULL) { printf("0\n"); return; }
```
> Polynôme nul = liste vide = on affiche `0`. Retour immédiat.

```c
    int premier = 1;
    while (p != NULL) {
```
> `premier` sert à distinguer le premier monôme (pas de signe `+` devant) des suivants. La boucle parcourt tous les maillons.

```c
        if (!premier) {
            if (p->coefficient >= 0) printf(" + ");
            else                     printf(" - ");
        } else {
            if (p->coefficient < 0)  printf("-");
        }
```
> Pour le premier monôme : on n'affiche le signe que s'il est négatif (le `+` initial est implicite). Pour les suivants : on affiche ` + ` ou ` - ` avec des espaces pour la lisibilité.

```c
        double c = fabs(p->coefficient);
        if (c != 1.0 || p->exposant == 0) printf("%g", c);
```
> On travaille avec la valeur absolue du coefficient (le signe a déjà été affiché). La règle : si le coefficient est exactement `1` **et** que l'exposant est non nul, on ne l'affiche pas (convention mathématique : on écrit `X` et non `1X`). `%g` choisit automatiquement la notation la plus courte entre `%f` et `%e`.

```c
        if (p->exposant > 0) {
            printf("X");
            if (p->exposant > 1) printf("^%d", p->exposant);
        }
```
> On n'affiche `X` que si l'exposant est > 0. Si l'exposant est 1, on écrit juste `X`; si > 1, on écrit `X^n`.

```c
        premier = 0;
        p = p->suivant;
    }
    printf("\n");
}
```
> Fin de boucle : passage au maillon suivant, et saut de ligne final.

---

## 4. Codage par degré décroissant

Voir la fonction `insererTrie` ci-dessus (section 2). L'ordre décroissant est garanti par la logique d'insertion qui compare les exposants pour trouver la bonne position. Le premier maillon de la liste est toujours celui de plus haut degré.

---

## 5. Évaluation

### Fonction `eval`

```c
double eval(POINTEUR p, double x) {
    double res = 0.0;
    while (p != NULL) {
        res += p->coefficient * pow(x, (double)p->exposant);
        p = p->suivant;
    }
    return res;
}
```
> Parcourt la liste et accumule la valeur de chaque terme. `pow(x, exp)` calcule `x^exp`. Le cast `(double)p->exposant` assure que la fonction `pow` reçoit bien un `double` (évite des warnings sur certains compilateurs). Complexité : O(n) où n est le nombre de monômes.

---

## 6. Opérations arithmétiques

### Utilitaire interne `copierPoly`

```c
static POINTEUR copierPoly(POINTEUR a) {
    POINTEUR res = NULL;
    while (a != NULL) {
        insererTrie(&res, a->coefficient, a->exposant);
        a = a->suivant;
    }
    return res;
}
```
> Crée un **nouveau** polynôme identique à `a`. Chaque appel à `insererTrie` alloue un nouveau maillon via `creerMonome`, donc la copie est totalement indépendante de l'original. `static` : visible uniquement dans `polynome.c`.

---

### Fonction `plus`

```c
POINTEUR plus(POINTEUR a, POINTEUR b) {
    POINTEUR res = copierPoly(a);   // (1) copie de a
    while (b != NULL) {
        insererTrie(&res, b->coefficient, b->exposant);  // (2) fusion de b
        b = b->suivant;
    }
    return res;
}
```
> **(1)** On commence par copier intégralement `a` dans `res`.
> **(2)** Pour chaque monôme de `b`, on l'insère dans `res`. Si le degré existe déjà (vient de `a`), `insererTrie` additionne les coefficients. Si le résultat d'une fusion vaut 0, le maillon reste dans la liste avec coefficient 0 (choix de simplification : pas de suppression des termes nuls ici). `a` et `b` ne sont pas modifiés.

---

### Fonction `moins`

```c
POINTEUR moins(POINTEUR a, POINTEUR b) {
    POINTEUR res = copierPoly(a);
    while (b != NULL) {
        insererTrie(&res, -(b->coefficient), b->exposant);
        b = b->suivant;
    }
    return res;
}
```
> Identique à `plus` sauf qu'on insère les monômes de `b` avec leur coefficient **négativé** : `-(b->coefficient)`. `insererTrie` gère alors la fusion par soustraction.

---

### Fonction `fois`

```c
POINTEUR fois(POINTEUR a, POINTEUR b) {
    if (a == NULL || b == NULL) return NULL;
```
> Cas de base de la récursion : multiplier par le polynôme nul donne le polynôme nul.

```c
    POINTEUR res = NULL;
    insererTrie(&res,
                a->coefficient * b->coefficient,
                a->exposant    + b->exposant);
```
> **Terme tête × tête :** `aXⁿ × bXᵐ = ab Xⁿ⁺ᵐ`. Les coefficients se multiplient, les exposants s'additionnent.

```c
    POINTEUR t1 = NULL;
    POINTEUR bReste = b->suivant;
    while (bReste != NULL) {
        insererTrie(&t1,
                    a->coefficient * bReste->coefficient,
                    a->exposant    + bReste->exposant);
        bReste = bReste->suivant;
    }
```
> **Tête de a × reste de b :** `aXⁿ × Q'`. On multiplie le monôme de tête de `a` par chacun des monômes du reste de `b`.

```c
    POINTEUR t2 = fois(a->suivant, b);
```
> **Reste de a × tout b :** `P' × Q`. Appel récursif avec `a->suivant` (le reste de a). La récursion s'arrête quand `a->suivant == NULL`.

```c
    POINTEUR tmp   = plus(res, t1);
    POINTEUR final = plus(tmp, t2);
    return final;
}
```
> On assemble les trois parties : `res + t1 + t2`. Chaque `plus` alloue de nouveaux maillons (conformément à la règle de l'énoncé : ne pas modifier les maillons existants).

---

### Fonction `quotient`

```c
POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste) {
    if (b == NULL) {
        fprintf(stderr, "Erreur : division par le polynôme nul\n");
        exit(EXIT_FAILURE);
    }
```
> Vérification préalable : on ne peut pas diviser par zéro.

```c
    POINTEUR q   = NULL;
    POINTEUR rem = copierPoly(a);
```
> `q` accumule le quotient (initialement nul). `rem` est une copie du dividende `a` qui sera réduit à chaque itération.

```c
    while (rem != NULL && rem->exposant >= b->exposant) {
```
> La division s'arrête quand le degré du reste est strictement inférieur au degré du diviseur (condition d'arrêt de la division euclidienne polynomiale). `rem != NULL` couvre le cas où le reste devient le polynôme nul.

```c
        double  coefTerm = rem->coefficient / b->coefficient;
        int     expTerm  = rem->exposant    - b->exposant;
```
> Calcul du terme courant du quotient : on divise les coefficients de tête et on soustrait les exposants.

```c
        insererTrie(&q, coefTerm, expTerm);
```
> Ajout de ce terme au quotient.

```c
        POINTEUR termePoly = NULL;
        insererTrie(&termePoly, coefTerm, expTerm);
        POINTEUR produit = fois(termePoly, b);
        rem = moins(rem, produit);
    }
```
> On construit le monôme `coefTerm * X^expTerm` sous forme de polynôme à un terme, on le multiplie par `b`, puis on soustrait ce produit du reste courant. Cela correspond à l'étape de la division longue.

```c
    if (reste != NULL) *reste = rem;
    return q;
}
```
> Si l'appelant a fourni une adresse pour récupérer le reste (pointeur non NULL), on y stocke `rem`. On retourne le quotient.

---

## 7. Garbage Collector

### Fonction `enregistrerPoly`

```c
void enregistrerPoly(POINTEUR p) {
    if (nbPolyUtile >= MAX_POLY) {
        fprintf(stderr, "Avertissement : polyUtile plein\n");
        return;
    }
    polyUtile[nbPolyUtile++] = p;
}
```
> Ajoute la tête d'un polynôme dans le tableau des polynômes à protéger. `nbPolyUtile++` : post-incrément, on stocke à l'indice actuel puis on incrémente. La vérification de débordement évite un accès hors limites du tableau.

---

### Fonction `recycler`

```c
void recycler(void) {
    for (int i = 0; i < nbPolyUtile; i++) {
        POINTEUR p = polyUtile[i];
        while (p != NULL) {
            p->utile = 1;
            p = p->suivant;
        }
    }
```
> **Passe 1 — MARQUAGE.** Pour chaque polynôme enregistré dans `polyUtile`, on parcourt ses maillons via `->suivant` et on pose `utile = 1`. Un maillon peut être marqué plusieurs fois sans problème (si deux polynômes partagent des maillons, ce qui n'arrive pas ici mais est prévu par l'énoncé).

```c
    POINTEUR *courant = &tousLesMaillons;
    int liberes = 0;
```
> **Passe 2 — BALAYAGE.** `courant` est un pointeur sur `POINTEUR`, c'est-à-dire un `Monome**`. Il pointe initialement sur la variable `tousLesMaillons` elle-même (pas sur le maillon, mais sur la variable qui contient son adresse). Cette technique permet de modifier le lien précédent sans nœud sentinelle.

```c
    while (*courant != NULL) {
        POINTEUR m = *courant;
        if (m->utile == 1) {
            m->utile = 0;
            courant  = &(m->general);
```
> **Maillon utile :** on efface la marque (remise à zéro pour le prochain GC) et on **avance** `courant` pour qu'il pointe sur le champ `general` de ce maillon. Le maillon reste dans la liste.

```c
        } else {
            *courant = m->general;
            free(m);
            liberes++;
        }
    }
```
> **Maillon inutile :** `*courant = m->general` "saute" ce maillon dans la liste (le lien précédent pointe maintenant directement sur le suivant). Puis `free(m)` rend la mémoire au système. `liberes` est incrémenté pour le rapport final.

```c
    printf("[GC] %d maillon(s) libéré(s).\n", liberes);
}
```
> Affichage du bilan.

**Illustration du pointeur sur pointeur :**

Avant suppression d'un maillon inutile `M2` :
```
tousLesMaillons → M1 → M2 → M3
     ↑                 ↑
courant avant      *courant = M2
```
Après `*courant = m->general` :
```
tousLesMaillons → M1 → M3
                       ↑
                   *courant (maintenant M1->general = M3)
```
`courant` pointait sur `M1->general`. En écrivant `*courant = M2->general (= M3)`, on a mis `M1->general = M3` directement, sans jamais avoir besoin de connaître `M1`.

---

## 8. Versions récursives

### Fonction `plus_rec`

```c
POINTEUR plus_rec(POINTEUR a, POINTEUR b) {
    if (a == NULL) return copierPoly(b);
    if (b == NULL) return copierPoly(a);
```
> **Cas de base.** Si l'un des deux est vide, l'addition vaut l'autre. On retourne une copie (nouveaux maillons, pas de partage de structure).

```c
    if (a->exposant > b->exposant) {
        POINTEUR res    = creerMonome(a->coefficient, a->exposant);
        res->suivant    = plus_rec(a->suivant, b);
        return res;
```
> **a a le plus grand degré.** On place le monôme de `a` en tête du résultat, et on résout récursivement `a->suivant + b`. Le résultat de la récursion devient la suite de la liste.

```c
    } else if (b->exposant > a->exposant) {
        POINTEUR res    = creerMonome(b->coefficient, b->exposant);
        res->suivant    = plus_rec(a, b->suivant);
        return res;
```
> **b a le plus grand degré.** Symétrique : on place le monôme de `b` en tête et on résout `a + b->suivant`.

```c
    } else {
        double c = a->coefficient + b->coefficient;
        if (c == 0.0) {
            return plus_rec(a->suivant, b->suivant);
        }
        POINTEUR res = creerMonome(c, a->exposant);
        res->suivant = plus_rec(a->suivant, b->suivant);
        return res;
    }
}
```
> **Même degré.** On additionne les coefficients. Si le résultat est nul, on n'alloue pas de maillon (on saute les deux et on continue la récursion). Sinon, on crée le maillon avec la somme.

---

### Fonction `moins_rec`

```c
POINTEUR moins_rec(POINTEUR a, POINTEUR b) {
    if (a == NULL && b == NULL) return NULL;
    if (a == NULL) {
        POINTEUR neg = NULL;
        POINTEUR tmp = b;
        while (tmp != NULL) {
            insererTrie(&neg, -(tmp->coefficient), tmp->exposant);
            tmp = tmp->suivant;
        }
        return neg;
    }
    if (b == NULL) return copierPoly(a);
```
> **Cas de base.** `0 - 0 = 0`. `0 - b = -b` (on négative tous les coefficients de `b`). `a - 0 = a` (copie de `a`).

```c
    if (a->exposant > b->exposant) {
        POINTEUR res = creerMonome(a->coefficient, a->exposant);
        res->suivant = moins_rec(a->suivant, b);
        return res;
    } else if (b->exposant > a->exposant) {
        POINTEUR res = creerMonome(-(b->coefficient), b->exposant);
        res->suivant = moins_rec(a, b->suivant);
        return res;
    } else {
        double c = a->coefficient - b->coefficient;
        if (c == 0.0) return moins_rec(a->suivant, b->suivant);
        POINTEUR res = creerMonome(c, a->exposant);
        res->suivant = moins_rec(a->suivant, b->suivant);
        return res;
    }
}
```
> Même logique que `plus_rec`, sauf :
> - Quand `b->exposant > a->exposant` : on prend `-b->coefficient` (car on soustrait ce terme)
> - Quand les degrés sont égaux : on fait `a->coefficient - b->coefficient`

---

## 9. Fonction `main` (C)

```c
int main(void) {
    char buf[512];
    POINTEUR p = NULL, q = NULL;
```
> Tampon de 512 caractères pour la saisie. Deux polynômes `p` et `q` initialisés à `NULL` (polynôme nul = liste vide).

```c
    printf("Entrez le polynôme P : ");
    if (!fgets(buf, sizeof(buf), stdin)) return 1;
    buf[strcspn(buf, "\n")] = '\0';
    analyserPolynome(buf, &p);
```
> `fgets` lit jusqu'à 511 caractères depuis l'entrée standard (inclut le `\n` final). `strcspn(buf, "\n")` retourne l'indice du `\n`; on le remplace par `\0` pour terminer la chaîne proprement. Puis on analyse.

```c
    /* Q7 : polynôme temporaire non-enregistré */
    POINTEUR tmp = NULL;
    insererTrie(&tmp, 3.0, 2);
    insererTrie(&tmp, 1.0, 0);
    printf("[Q7] Polynôme temporaire = "); afficherPolynome(tmp);
    /* tmp n'est PAS enregistré → ses maillons seront libérés */
    enregistrerPoly(p);
    enregistrerPoly(q);
    recycler();
```
> **Démonstration du GC.** `tmp` simule un résultat intermédiaire qu'on "oublie" d'enregistrer. Après `recycler()`, ses maillons sont libérés. `p` et `q` étant enregistrés, ils survivent.

```c
    POINTEUR somme = plus(p, q);
    POINTEUR diff  = moins(p, q);
    POINTEUR prod  = fois(p, q);
    POINTEUR rem2  = NULL;
    POINTEUR quot  = quotient(p, q, &rem2);
    POINTEUR somme_rec = plus_rec(p, q);
    POINTEUR diff_rec  = moins_rec(p, q);
```
> Démonstration de Q6 et Q8. Tous ces appels allouent de nouveaux maillons (non enregistrés dans `polyUtile` dans cette démo, donc libérables lors d'un second GC).

---

# PARTIE II — LANGAGE JAVA

---

## 1. Classe `Monome.java`

```java
public class Monome {
    double coefficient;
    int    exposant;
    Monome suivant;
    Monome  general;
    boolean utile;
```
> Équivalent Java de la `struct Monome` en C. Pas de modificateur d'accès (`public`/`private`) sur les champs : accès "package-private", suffisant pour ce projet mono-package. `boolean utile` remplace `int utile` du C (Java dispose d'un vrai type booléen).

```java
    Monome(double coefficient, int exposant) {
        this.coefficient = coefficient;
        this.exposant    = exposant;
        this.suivant     = null;
        this.general     = null;
        this.utile       = false;
    }
}
```
> Constructeur : initialise tous les champs. `this.` lève l'ambiguïté entre les paramètres et les champs. En Java, les références sont `null` par défaut mais on les initialise explicitement pour la clarté.

---

## 2. Classe `Polynome.java`

```java
public class Polynome {
    private Monome tete;
    public static List<Monome> tousLesMaillons = new ArrayList<>();
```
> `tete` est `private` : seule la classe `Polynome` y accède directement (encapsulation). `tousLesMaillons` est `static` : partagé entre toutes les instances, analogue à la variable globale C.

```java
    public Polynome() {
        this.tete = null;
    }
```
> Constructeur par défaut : polynôme nul (liste vide).

---

### Méthode `creerMonome` (privée)

```java
private Monome creerMonome(double coef, int exp) {
    Monome m = new Monome(coef, exp);
    tousLesMaillons.add(m);
    return m;
}
```
> `new Monome(coef, exp)` alloue le maillon (la JVM gère la mémoire). `tousLesMaillons.add(m)` l'enregistre dans la liste globale (analogue à `p->general = tousLesMaillons; tousLesMaillons = p;` en C). Méthode `private` : seule `Polynome` peut créer des maillons de cette façon.

---

### Méthode `insererTrie`

```java
public void insererTrie(double coef, int exp) {
    if (coef == 0) return;
```
> On refuse les monômes nuls.

```java
    Monome courant = tete;
    while (courant != null) {
        if (courant.exposant == exp) {
            courant.coefficient += coef;
            return;
        }
        courant = courant.suivant;
    }
```
> Fusion des degrés identiques : même logique qu'en C.

```java
    Monome nouveau = creerMonome(coef, exp);
    if (tete == null || exp > tete.exposant) {
        nouveau.suivant = tete;
        tete = nouveau;
    } else {
        Monome prec = tete;
        while (prec.suivant != null && prec.suivant.exposant > exp)
            prec = prec.suivant;
        nouveau.suivant = prec.suivant;
        prec.suivant = nouveau;
    }
}
```
> Insertion triée identique à la version C. En Java, pas besoin de `**tete` car `tete` est un champ de l'objet, modifiable directement (`this.tete = nouveau`).

---

### Méthode `analyser`

```java
public void analyser(String chaine) {
    if (chaine == null || chaine.trim().isEmpty()) return;
    String s = chaine.replaceAll("\\s+", "");
```
> `replaceAll("\\s+", "")` supprime **tous** les espaces en une seule opération (regex `\s+` = un ou plusieurs caractères blancs). Approche différente du C où on passe les espaces caractère par caractère — ici on prétraite la chaîne entière.

```java
    int i = 0;
    while (i < s.length()) {
        int signe = 1;
        if      (s.charAt(i) == '+') { i++; }
        else if (s.charAt(i) == '-') { signe = -1; i++; }
```
> `charAt(i)` : accès au caractère à l'indice `i`. Pas d'arithmétique de pointeurs en Java.

```java
        double coef = 1.0;
        boolean coefLu = false;
        int debut = i;
        while (i < s.length() && (Character.isDigit(s.charAt(i)) || s.charAt(i) == '.')) {
            i++; coefLu = true;
        }
        if (coefLu) coef = Double.parseDouble(s.substring(debut, i));
```
> On mémorise la position de début (`debut`), on avance `i` tant qu'on lit des chiffres ou un point, puis on extrait la sous-chaîne `s.substring(debut, i)` et on la convertit en `double` avec `Double.parseDouble`. Plus haut niveau qu'en C où on construisait la valeur chiffre par chiffre.

```java
        if (i < s.length() && s.charAt(i) == '*') i++;
        int expo = 0;
        if (i < s.length() && (s.charAt(i) == 'X' || s.charAt(i) == 'x')) {
            i++;
            if (i < s.length() && s.charAt(i) == '^') {
                i++;
                int debutExp = i;
                while (i < s.length() && Character.isDigit(s.charAt(i))) i++;
                expo = Integer.parseInt(s.substring(debutExp, i));
            } else expo = 1;
        }
        this.insererTrie(coef * signe, expo);
    }
}
```
> Même logique que le C pour la lecture de l'exposant. `Integer.parseInt` pour convertir la sous-chaîne en entier.

---

### Méthode `afficher`

```java
public void afficher() {
    if (tete == null) { System.out.println("0"); return; }
    Monome p = tete;
    boolean premier = true;
    while (p != null) {
        double c = p.coefficient;
        if (c == 0) { p = p.suivant; continue; }
```
> En Java, `continue` passe directement à l'itération suivante. On saute les termes nuls (qui peuvent apparaître si une fusion a annulé un coefficient).

```java
        if (!premier && c > 0) System.out.print(" + ");
        else if (c < 0)        System.out.print(" - ");
        double absC = Math.abs(c);
        if (absC != 1 || p.exposant == 0) {
            if (absC == (long) absC) System.out.print((long) absC);
            else System.out.print(absC);
        }
```
> `(long) absC` : si la valeur absolue est entière (ex : `2.0`), on la caste en `long` pour afficher `2` et non `2.0`. Raffinement d'affichage absent de la version C.

```java
        if (p.exposant > 0) {
            System.out.print("X");
            if (p.exposant > 1) System.out.print("^" + p.exposant);
        }
        premier = false;
        p = p.suivant;
    }
    System.out.println();
}
```

---

### Méthode `evaluer`

```java
public double evaluer(double x) {
    double resultat = 0;
    Monome p = tete;
    while (p != null) {
        resultat += p.coefficient * Math.pow(x, p.exposant);
        p = p.suivant;
    }
    return resultat;
}
```
> `Math.pow` : équivalent de `pow` de `math.h`. Exactement la même logique qu'en C.

---

### Méthode `copier`

```java
public Polynome copier() {
    Polynome res = new Polynome();
    Monome p = tete;
    while (p != null) {
        res.insererTrie(p.coefficient, p.exposant);
        p = p.suivant;
    }
    return res;
}
```
> Crée et retourne un nouvel objet `Polynome` avec les mêmes maillons (nouveaux objets `Monome`). Analogue de `copierPoly` en C.

---

### Méthode `plus` (statique)

```java
public static Polynome plus(Polynome a, Polynome b) {
    Polynome res = a.copier();
    Monome p = b.getTete();
    while (p != null) {
        res.insererTrie(p.coefficient, p.exposant);
        p = p.suivant;
    }
    return res;
}
```
> `static` : pas besoin d'instance pour appeler `Polynome.plus(a, b)`. Même algorithme qu'en C. `getTete()` : accesseur public nécessaire car `tete` est `private`.

---

### Méthode `moins` (statique)

```java
public static Polynome moins(Polynome a, Polynome b) {
    Polynome res = a.copier();
    Monome p = b.getTete();
    while (p != null) {
        res.insererTrie(-p.coefficient, p.exposant);
        p = p.suivant;
    }
    return res;
}
```
> `-p.coefficient` : négation du coefficient de chaque monôme de `b`.

---

### Méthode `fois` (statique)

```java
public static Polynome fois(Polynome a, Polynome b) {
    if (a.getTete() == null || b.getTete() == null) return new Polynome();
    Monome headA = a.getTete();
    Monome headB = b.getTete();
```
> Cas de base : un polynôme nul × quoi que ce soit = polynôme nul.

```java
    Polynome res = new Polynome();
    res.insererTrie(headA.coefficient * headB.coefficient,
                    headA.exposant    + headB.exposant);
```
> Terme tête × tête.

```java
    Polynome bReste = new Polynome();
    bReste.setTete(headB.suivant);
    Polynome t1 = new Polynome();
    Monome p = bReste.getTete();
    while (p != null) {
        t1.insererTrie(headA.coefficient * p.coefficient,
                       headA.exposant    + p.exposant);
        p = p.suivant;
    }
```
> `bReste` pointe sur le reste de `b` (à partir du second maillon). On calcule `headA × chaque terme de bReste`. Note : `setTete` expose directement le maillon interne de `b`, ce qui est un compromis de conception pour éviter la copie complète.

```java
    Polynome aReste = new Polynome();
    aReste.setTete(headA.suivant);
    Polynome t2 = fois(aReste, b);
    return plus(plus(res, t1), t2);
}
```
> `aReste` = reste de `a`. `fois(aReste, b)` : récursion. Assemblage final : `res + t1 + t2`.

---

### Méthode `quotient` (statique)

```java
public static Polynome quotient(Polynome a, Polynome b, Polynome[] reste) {
    if (b.getTete() == null)
        throw new ArithmeticException("Division par le polynôme nul");
```
> En Java, on lève une exception plutôt que d'appeler `exit`. `ArithmeticException` est une exception non vérifiée (pas besoin de `throws` dans la signature).

```java
    Polynome q   = new Polynome();
    Polynome rem = a.copier();
    while (rem.getTete() != null
           && rem.getTete().exposant >= b.getTete().exposant) {
        double coefT = rem.getTete().coefficient / b.getTete().coefficient;
        int    expT  = rem.getTete().exposant    - b.getTete().exposant;
        q.insererTrie(coefT, expT);
        Polynome terme = new Polynome();
        terme.insererTrie(coefT, expT);
        Polynome prod = fois(terme, b);
        rem = moins(rem, prod);
    }
    if (reste != null && reste.length > 0) reste[0] = rem;
    return q;
}
```
> `Polynome[] reste` : tableau d'un élément utilisé comme "pointeur de sortie" (Java ne supporte pas le passage par référence de variables primitives ou d'objets). `reste[0] = rem` est l'équivalent de `*reste = rem` en C.

---

### Méthode `plusRec` (Q8, statique)

```java
public static Polynome plusRec(Monome a, Monome b) {
    if (a == null && b == null) return new Polynome();
    if (a == null) { Polynome r = new Polynome(); copierChaine(b, r); return r; }
    if (b == null) { Polynome r = new Polynome(); copierChaine(a, r); return r; }
```
> Cas de base. `copierChaine` est un utilitaire privé qui insère tous les maillons d'une chaîne dans un `Polynome`.

```java
    Polynome res = new Polynome();
    if (a.exposant > b.exposant) {
        Monome nv = new Monome(a.coefficient, a.exposant);
        Polynome suite = plusRec(a.suivant, b);
        nv.suivant = suite.getTete();
        res.setTete(nv);
```
> On crée un maillon `nv` pour la tête du résultat, on calcule récursivement la suite, puis on chaîne `nv.suivant` sur la tête du résultat récursif. `res.setTete(nv)` définit la tête de l'objet résultat. Note : les maillons créés ici avec `new Monome(...)` ne passent **pas** par `creerMonome`, donc ils ne sont pas enregistrés dans `tousLesMaillons`. C'est un point à améliorer si on voulait que le GC surveille aussi ces maillons.

---

### Méthode `copierChaine` (privée statique)

```java
private static void copierChaine(Monome src, Polynome dest) {
    while (src != null) {
        dest.insererTrie(src.coefficient, src.exposant);
        src = src.suivant;
    }
}
```
> Parcourt une chaîne de maillons et insère chaque monôme dans `dest`. Utilisé comme utilitaire par `plusRec` et `moinsRec`.

---

## 3. Classe `GarbageCollector.java`

### Champs statiques

```java
static Monome tousLesMaillons = null;
static final int MAX_POLY = 100;
static Monome[]  polyUtile  = new Monome[MAX_POLY];
static int       nbPolyUtile = 0;
```
> `static` : variables de classe, partagées. `final int MAX_POLY` : constante (équivalent du `#define MAX_POLY 100` en C). `new Monome[MAX_POLY]` : tableau initialisé à `null` partout.

---

### Méthode `allouer`

```java
public static Monome allouer(double coef, int exp) {
    Monome m = new Monome(coef, exp);
    m.general = tousLesMaillons;
    tousLesMaillons = m;
    return m;
}
```
> Alternative à `new Monome(coef, exp)` pour du code qui veut être surveillé par ce GC. Chaîne le nouveau maillon en tête de `tousLesMaillons` via le champ `general`. Analogue de `creerMonome` en C.

---

### Méthode `enregistrerPoly`

```java
public static void enregistrerPoly(Monome tete) {
    if (nbPolyUtile >= MAX_POLY) {
        System.err.println("Avertissement : polyUtile plein, polynôme non enregistré.");
        return;
    }
    polyUtile[nbPolyUtile++] = tete;
}
```
> `System.err.println` : sortie d'erreur, analogue de `fprintf(stderr, ...)`. `nbPolyUtile++` : post-incrément, même idiome qu'en C.

---

### Méthode `recycler`

```java
public static void recycler() {
    for (int i = 0; i < nbPolyUtile; i++) {
        Monome p = polyUtile[i];
        while (p != null) { p.utile = true; p = p.suivant; }
    }
```
> **Passe 1 — Marquage.** Identique à la version C.

```java
    Monome sentinelle = new Monome(0, 0);
    sentinelle.general = tousLesMaillons;
    Monome pred = sentinelle;
    Monome courant = tousLesMaillons;
    int liberes = 0;
```
> **Nœud sentinelle.** En Java on ne peut pas faire `Monome **courant = &tousLesMaillons` (pas de pointeurs sur variables). Solution : on crée un faux nœud `sentinelle` dont le champ `general` pointe sur `tousLesMaillons`. Ainsi on traite la tête de la liste comme n'importe quel autre lien.
> - `pred` : maillon précédent dans la liste (commence sur la sentinelle)
> - `courant` : maillon en cours d'examen

```java
    while (courant != null) {
        Monome suivantGeneral = courant.general;
        if (courant.utile) {
            courant.utile = false;
            pred = courant;
        } else {
            pred.general = suivantGeneral;
            courant.suivant = null;
            courant.general = null;
            liberes++;
        }
        courant = suivantGeneral;
    }
    tousLesMaillons = sentinelle.general;
```
> **Passe 2 — Balayage.**
> - On sauvegarde `courant.general` dans `suivantGeneral` **avant** de potentiellement annuler `courant.general`.
> - **Maillon utile :** effacer marque, `pred` avance sur `courant`.
> - **Maillon inutile :** `pred.general = suivantGeneral` court-circuite `courant`. On annule `courant.suivant` et `courant.general` pour aider le GC JVM (supprimer les références entrantes/sortantes). En Java, `free()` n'existe pas : c'est le GC JVM qui récupère l'objet une fois qu'il n'est plus référencé.
> - Après la boucle : `tousLesMaillons = sentinelle.general` met à jour la vraie tête (qui a peut-être été modifiée si le premier maillon était inutile).

---

## 4. Classe `MainGC.java` — Démonstration Q7

```java
static void afficher(Monome tete) { ... }
```
> Fonction d'affichage locale à cette classe (pas d'instance `Polynome` ici, on travaille directement avec les maillons).

```java
static Monome inserer(Monome tete, double coef, int exp) {
    if (coef == 0.0) return tete;
    Monome nouveau = GarbageCollector.allouer(coef, exp);
    ...
    return tete;
}
```
> Insertion triée utilisant `GarbageCollector.allouer` au lieu de `new Monome(...)`. Tout maillon créé ici est enregistré dans le GC.

```java
public static void main(String[] args) {
    Monome p = null; p = inserer(p, 1.0, 3); p = inserer(p, 2.0, 1); p = inserer(p, 1.0, 0);
    Monome q = null; q = inserer(q, 1.0, 1); q = inserer(q, 1.0, 0);
    Monome tmp = null; tmp = inserer(tmp, 3.0, 2); tmp = inserer(tmp, 1.0, 0);
    /* tmp non enregistré → ses maillons seront libérés */
    GarbageCollector.enregistrerPoly(p);
    GarbageCollector.enregistrerPoly(q);
    GarbageCollector.recycler();
    /* P et Q intacts après GC */
}
```
> Scénario : `p` et `q` sont construits et enregistrés comme utiles. `tmp` est construit mais **pas** enregistré, simulant un polynôme intermédiaire abandonné. Après `recycler()`, les 2 maillons de `tmp` sont libérés, ceux de `p` et `q` survivent.

---

## 5. Classe `Main.java` — Programme principal Java

```java
Polynome p = new Polynome();
p.analyser(sc.nextLine());
Polynome q = new Polynome();
q.analyser(sc.nextLine());
```
> Lecture de deux polynômes depuis la console.

```java
System.out.printf("P(%g) = %g%n", x, p.evaluer(x));
```
> `%g` : même format qu'en C (notation courte). `%n` : saut de ligne portable (différent de `\n` qui peut être `\r\n` sous Windows).

```java
Polynome[] reste = new Polynome[1];
Polynome quot = Polynome.quotient(p, q, reste);
```
> Le tableau d'un élément joue le rôle du `POINTEUR *reste` du C. `reste[0]` contiendra le polynôme reste après l'appel.

```java
Polynome.plusRec(p.getTete(), q.getTete()).afficher();
```
> `plusRec` prend des `Monome` (têtes de listes), pas des `Polynome`. `.afficher()` est appelé directement sur le `Polynome` retourné, sans le stocker.

---

## 10. Exemples d'exécution

### Exemple détaillé — Entrées

```
P : X^3 + 2*X + 1
Q : X + 1
```

### Trace d'exécution pas à pas de `analyserPolynome("X^3 + 2*X + 1", &p)`

| Étape | `i` | Caractère | Action |
|-------|-----|-----------|--------|
| 1 | 0 | `X` | Cas X : lire puissance → `^3`, insère (1.0, 3) |
| 2 | 3 | ` ` | Passer espaces |
| 3 | 4 | `+` | signe = +1, i++ |
| 4 | 5 | ` ` | analyserMonome → passerEspaces |
| 5 | 6 | `2` | lireNombre → 2.0, puis `*`, puis `X`, e=1, insère (2.0, 1) |
| 6 | 10 | ` ` | Passer espaces |
| 7 | 11 | `+` | signe = +1, i++ |
| 8 | 13 | `1` | lireNombre → 1.0, pas de X → expo=0, insère (1.0, 0) |

État final de la liste `p` : `[coef=1, exp=3] → [coef=2, exp=1] → [coef=1, exp=0] → NULL`

### Trace du GC avec `p = X^3+2X+1`, `q = X+1`, `tmp = 3X^2+1`

**Avant le GC** — `tousLesMaillons` (dans l'ordre d'allocation, têtes en premier) :
```
tmp[0] → tmp[1] → q[0] → q[1] → p[0] → p[1] → p[2] → NULL
(3X^2)   (cte 1)  (X)    (cte 1) (X^3)  (2X)   (cte 1)
```

**Passe 1 — Marquage** (`polyUtile = [p, q]`) :
- Parcours de `p` : `p[0].utile=1`, `p[1].utile=1`, `p[2].utile=1`
- Parcours de `q` : `q[0].utile=1`, `q[1].utile=1`
- `tmp[0].utile` et `tmp[1].utile` restent à 0

**Passe 2 — Balayage** (dans l'ordre de `tousLesMaillons`) :
```
tmp[0] : utile=0 → free, liberes=1
tmp[1] : utile=0 → free, liberes=2
q[0]   : utile=1 → effacer marque, garder
q[1]   : utile=1 → effacer marque, garder
p[0]   : utile=1 → effacer marque, garder
p[1]   : utile=1 → effacer marque, garder
p[2]   : utile=1 → effacer marque, garder
```

**Résultat :** `[GC] 2 maillon(s) libéré(s).`

### Résultat de `plus(X^3+2X+1, X+1)`

1. `copierPoly(p)` → `res = [X^3, 2X, 1]` (nouveaux maillons)
2. Insérer `X` de `q` dans `res` : degré 1 existe → `res[1].coef += 1 → 3X`
3. Insérer `1` de `q` dans `res` : degré 0 existe → `res[2].coef += 1 → 2`
4. Résultat : `X^3 + 3X + 2`

### Trace récursive de `plus_rec(X^3+2X+1, X+1)`

```
plus_rec([X^3, 2X, 1], [X, 1])
  a.exp(3) > b.exp(1) : créer X^3, suite = plus_rec([2X, 1], [X, 1])
    plus_rec([2X, 1], [X, 1])
      a.exp(1) == b.exp(1) : c = 2+1 = 3, créer 3X, suite = plus_rec([1], [1])
        plus_rec([1], [1])
          a.exp(0) == b.exp(0) : c = 1+1 = 2, créer 2, suite = plus_rec(NULL, NULL)
            plus_rec(NULL, NULL) → retourne Polynome vide (NULL)
          retourne [2]
        suite = [2]
      3X.suivant = [2] → retourne [3X, 2]
    suite = [3X, 2]
  X^3.suivant = [3X, 2] → retourne [X^3, 3X, 2]
```

**Résultat :** `X^3 + 3X + 2` ✓ (même que `plus` itératif)

---

## Compilation et exécution

### Version C

```bash
# Compilation
gcc -o polymanager langage_c/polynome.c -lm -Wall -Wextra

# Exécution
./polymanager
```

### Version Java

```bash
# Depuis langage_java/
javac Monome.java Polynome.java GarbageCollector.java Main.java MainGC.java

# Programme principal (Q1-8)
java Main

# Démo GC standalone (Q7)
java MainGC
```

---

*Document interne — PolyManager DIC2, DIC 2, Semestre 1, POO Avancée.*
