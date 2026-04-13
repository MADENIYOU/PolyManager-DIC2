# Rapport de Projet — PolyManager DIC2

**Module :** Programmation Orientée Objet Avancée  
**Filière :** DIC 2  
**Sujet :** Gestion de polynômes d'une variable — implémentation en C et Java  

---

## Membres du groupe

| Nom complet | Filière |
|-------------|---------|
| Mouhamadou Madeniyou SALL | IABD |
| Adama SIDIBE | INFORMATIQUE |
| Abdoul Aziz KANE | TR |
| Anna NDOYE | SSI |
| Mame Aïssatou Samb NGOM | SSI |
| Fatoumata BARRO | TR |

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

## 1. Structure de données

### 1.1 Version C (`polynome.h`)

Un polynôme est représenté par une **liste chaînée de maillons**, chaque maillon étant un `struct Monome` contenant le coefficient, l'exposant et les pointeurs de chaînage.

```c
typedef struct Monome {
    double coefficient;      /* Coefficient du monôme               */
    int    exposant;         /* Degré du monôme                      */
    struct Monome *suivant;  /* Maillon suivant (ordre décroissant)  */

    /* Champs Garbage Collector (Question 7) */
    struct Monome *general;  /* Chaîne TOUS les maillons alloués     */
    int utile;               /* Marqueur GC : 1 = utile, 0 = libre  */
} Monome, *POINTEUR;
```

Les variables globales pour le GC sont déclarées ainsi :

```c
extern POINTEUR tousLesMaillons;   /* Tête de la liste globale de tous les maillons */
#define MAX_POLY 100
extern POINTEUR polyUtile[MAX_POLY]; /* Tableau des polynômes à conserver */
extern int      nbPolyUtile;         /* Nombre de polynômes enregistrés   */
```

**Choix de conception :** Le polynôme nul est représenté par la liste vide (`NULL`). Les maillons sont maintenus en ordre décroissant des degrés pour faciliter les opérations.

### 1.2 Version Java

```java
public class Monome {
    double coefficient;
    int    exposant;
    Monome suivant;   // maillon suivant dans le polynôme (ordre décroissant)

    // Champs Garbage Collector (Question 7)
    Monome  general;  // maillon suivant dans la liste globale de tous les maillons
    boolean utile;    // true = à conserver, false = à libérer

    Monome(double coefficient, int exposant) {
        this.coefficient = coefficient;
        this.exposant    = exposant;
        this.suivant     = null;
        this.general     = null;
        this.utile       = false;
    }
}
```

La classe `Polynome` encapsule la tête de liste et toutes les opérations.

---

## 2. Analyseur syntaxique et codage

### 2.1 Grammaire reconnue

```
polynôme  → [ '–' ] monôme { ( '+' | '–' ) monôme }
monôme    → nombre '*' xpuissance | xpuissance | nombre
xpuissance→ 'X' | 'X' '^' naturel
naturel   → chiffre { chiffre }
nombre    → naturel [ '.' { chiffre } ]
chiffre   → '0' | '1' | ... | '9'
```

Exemple valide : `– 4.5 * X^5 + 2 * X^4 + X^3 - X + 123.0`

### 2.2 Implémentation C — fonctions de l'analyseur

L'analyseur procède **caractère par caractère** en utilisant un indice `i` sur la chaîne d'entrée. Des fonctions auxiliaires statiques modularisent chaque règle grammaticale.

```c
/* Passer les espaces */
static void passerEspaces(char *s, int *i) {
    while (s[*i] == ' ' || s[*i] == '\t') (*i)++;
}

/* Lire un entier naturel */
static int lireNaturel(char *s, int *i) {
    int n = 0;
    if (!isdigit((unsigned char)s[*i])) {
        fprintf(stderr, "Erreur de syntaxe : chiffre attendu\n");
        exit(EXIT_FAILURE);
    }
    while (isdigit((unsigned char)s[*i])) {
        n = n * 10 + (s[*i] - '0');
        (*i)++;
    }
    return n;
}

/* Lire un nombre réel */
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

/* Analyser un monôme (coefficient, exposant, signe) */
static void analyserMonome(char *s, int *i, POINTEUR *poly, int signe) {
    double c = 1.0;
    int    e = 0;
    passerEspaces(s, i);

    if (isdigit((unsigned char)s[*i])) {
        c = lireNombre(s, i);
        passerEspaces(s, i);
        if (s[*i] == '*') { (*i)++; passerEspaces(s, i); }
        if (s[*i] == 'X' || s[*i] == 'x') {
            (*i)++;
            if (s[*i] == '^') { (*i)++; e = lireNaturel(s, i); }
            else e = 1;
        }
    } else if (s[*i] == 'X' || s[*i] == 'x') {
        (*i)++;
        if (s[*i] == '^') { (*i)++; e = lireNaturel(s, i); }
        else e = 1;
    } else {
        fprintf(stderr, "Erreur de syntaxe : monôme invalide\n");
        exit(EXIT_FAILURE);
    }
    insererTrie(poly, c * signe, e);
}

/* Point d'entrée : analyser le polynôme complet */
void analyserPolynome(char *s, POINTEUR *poly) {
    int i = 0, signe = 1;
    passerEspaces(s, &i);
    if      (s[i] == '-') { signe = -1; i++; }
    else if (s[i] == '+') {             i++; }

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

**Principe :** à chaque erreur de syntaxe, un message est affiché et `exit(EXIT_FAILURE)` est appelé, conformément à l'énoncé.

### 2.3 Implémentation Java

```java
public void analyser(String chaine) {
    if (chaine == null || chaine.trim().isEmpty()) return;
    String s = chaine.replaceAll("\\s+", "");
    int i = 0;

    while (i < s.length()) {
        int signe = 1;
        if      (s.charAt(i) == '+') { i++; }
        else if (s.charAt(i) == '-') { signe = -1; i++; }

        double coef = 1.0;
        boolean coefLu = false;
        int debut = i;
        while (i < s.length() && (Character.isDigit(s.charAt(i)) || s.charAt(i) == '.')) {
            i++; coefLu = true;
        }
        if (coefLu) coef = Double.parseDouble(s.substring(debut, i));

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

---

## 3. Affichage

### 3.1 C

```c
void afficherPolynome(POINTEUR p) {
    if (p == NULL) { printf("0\n"); return; }

    int premier = 1;
    while (p != NULL) {
        if (!premier) {
            if (p->coefficient >= 0) printf(" + ");
            else                     printf(" - ");
        } else {
            if (p->coefficient < 0)  printf("-");
        }
        double c = fabs(p->coefficient);
        if (c != 1.0 || p->exposant == 0) printf("%g", c);
        if (p->exposant > 0) {
            printf("X");
            if (p->exposant > 1) printf("^%d", p->exposant);
        }
        premier = 0;
        p = p->suivant;
    }
    printf("\n");
}
```

**Règles d'affichage :**
- Polynôme nul → `0`
- Coefficient 1 ou −1 devant X ou Xⁿ : on n'affiche pas le `1`
- Signes intercalaires affichés entre les monômes (`+` ou `−`)

### 3.2 Java

```java
public void afficher() {
    if (tete == null) { System.out.println("0"); return; }
    Monome p = tete;
    boolean premier = true;
    while (p != null) {
        double c = p.coefficient;
        if (c == 0) { p = p.suivant; continue; }
        if (!premier && c > 0) System.out.print(" + ");
        else if (c < 0)        System.out.print(" - ");
        double absC = Math.abs(c);
        if (absC != 1 || p.exposant == 0) {
            if (absC == (long) absC) System.out.print((long) absC);
            else System.out.print(absC);
        }
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

## 4. Codage par degré décroissant

La liste chaînée est maintenue **triée en ordre décroissant** des exposants dès l'insertion. Si un monôme de même degré existe déjà, les coefficients sont **cumulés** (fusion) plutôt que de créer un doublon.

### 4.1 C

```c
POINTEUR creerMonome(double coef, int exp) {
    POINTEUR p = (POINTEUR)malloc(sizeof(Monome));
    if (!p) { fprintf(stderr, "Erreur : malloc a échoué\n"); exit(EXIT_FAILURE); }
    p->coefficient = coef;
    p->exposant    = exp;
    p->suivant     = NULL;
    /* Q7 : enregistrement dans la liste générale */
    p->utile   = 0;
    p->general = tousLesMaillons;
    tousLesMaillons = p;
    return p;
}

void insererTrie(POINTEUR *tete, double coef, int exp) {
    if (coef == 0.0) return;

    /* Fusion si degré déjà présent */
    POINTEUR curr = *tete;
    while (curr != NULL) {
        if (curr->exposant == exp) { curr->coefficient += coef; return; }
        curr = curr->suivant;
    }

    POINTEUR nouveau = creerMonome(coef, exp);
    if (*tete == NULL || exp > (*tete)->exposant) {
        nouveau->suivant = *tete;
        *tete = nouveau;
    } else {
        POINTEUR prec = *tete;
        while (prec->suivant != NULL && prec->suivant->exposant > exp)
            prec = prec->suivant;
        nouveau->suivant = prec->suivant;
        prec->suivant = nouveau;
    }
}
```

### 4.2 Java

```java
public void insererTrie(double coef, int exp) {
    if (coef == 0) return;
    // Fusion si le degré existe déjà
    Monome courant = tete;
    while (courant != null) {
        if (courant.exposant == exp) { courant.coefficient += coef; return; }
        courant = courant.suivant;
    }
    Monome nouveau = creerMonome(coef, exp);
    if (tete == null || exp > tete.exposant) {
        nouveau.suivant = tete; tete = nouveau;
    } else {
        Monome prec = tete;
        while (prec.suivant != null && prec.suivant.exposant > exp)
            prec = prec.suivant;
        nouveau.suivant = prec.suivant;
        prec.suivant = nouveau;
    }
}
```

---

## 5. Évaluation

### 5.1 C

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

### 5.2 Java

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

**Principe :** on parcourt la liste chaînée et on accumule `coef × x^exp` pour chaque maillon (règle de Horner non utilisée ici, mais l'implémentation est correcte).

---

## 6. Opérations arithmétiques

### 6.1 Principes généraux

Conformément à l'énoncé :
- On **alloue de nouveaux maillons** : les polynômes sources ne sont jamais modifiés.
- On **ne gère pas la libération** immédiate : c'est le rôle du GC (Question 7).

Utilitaire interne commun : `copierPoly(a)` crée une copie profonde du polynôme `a`.

### 6.2 Addition — `plus(a, b)`

**Algorithme :** copier `a`, puis insérer chaque monôme de `b` dans la copie (la fonction `insererTrie` gère les fusions de coefficients).

```c
/* C */
POINTEUR plus(POINTEUR a, POINTEUR b) {
    POINTEUR res = copierPoly(a);
    while (b != NULL) {
        insererTrie(&res, b->coefficient, b->exposant);
        b = b->suivant;
    }
    return res;
}
```

```java
/* Java */
public static Polynome plus(Polynome a, Polynome b) {
    Polynome res = a.copier();
    Monome p = b.getTete();
    while (p != null) { res.insererTrie(p.coefficient, p.exposant); p = p.suivant; }
    return res;
}
```

### 6.3 Soustraction — `moins(a, b)`

Même principe que l'addition, mais on insère les monômes de `b` avec leur coefficient **négativé**.

```c
/* C */
POINTEUR moins(POINTEUR a, POINTEUR b) {
    POINTEUR res = copierPoly(a);
    while (b != NULL) {
        insererTrie(&res, -(b->coefficient), b->exposant);
        b = b->suivant;
    }
    return res;
}
```

### 6.4 Multiplication — `fois(a, b)`

La formule récursive de l'énoncé est utilisée :

> P × Q = (aXⁿ + P') × (bXᵐ + Q') = abXⁿ⁺ᵐ + aXⁿ×Q' + P'×Q

```c
/* C */
POINTEUR fois(POINTEUR a, POINTEUR b) {
    if (a == NULL || b == NULL) return NULL;

    /* terme tête × tête */
    POINTEUR res = NULL;
    insererTrie(&res, a->coefficient * b->coefficient, a->exposant + b->exposant);

    /* aXⁿ × Q' (reste de b) */
    POINTEUR t1 = NULL;
    POINTEUR bReste = b->suivant;
    while (bReste != NULL) {
        insererTrie(&t1, a->coefficient * bReste->coefficient,
                         a->exposant    + bReste->exposant);
        bReste = bReste->suivant;
    }

    /* P' × Q (reste de a × tout b, récursion) */
    POINTEUR t2 = fois(a->suivant, b);

    POINTEUR tmp   = plus(res, t1);
    POINTEUR final = plus(tmp, t2);
    return final;
}
```

### 6.5 Division euclidienne — `quotient(a, b, &reste)`

**Algorithme de division longue :**

Tant que deg(reste) ≥ deg(b) :
1. Calculer le terme courant : `q_i = (coef_tête_reste / coef_tête_b) × X^(deg_reste - deg_b)`
2. Soustraire `q_i × b` du reste
3. Accumuler `q_i` dans le quotient

```c
/* C */
POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste) {
    if (b == NULL) { fprintf(stderr, "Division par zéro\n"); exit(EXIT_FAILURE); }
    POINTEUR q   = NULL;
    POINTEUR rem = copierPoly(a);

    while (rem != NULL && rem->exposant >= b->exposant) {
        double coefT = rem->coefficient / b->coefficient;
        int    expT  = rem->exposant    - b->exposant;
        insererTrie(&q, coefT, expT);

        POINTEUR termePoly = NULL;
        insererTrie(&termePoly, coefT, expT);
        POINTEUR produit = fois(termePoly, b);
        rem = moins(rem, produit);
    }
    if (reste != NULL) *reste = rem;
    return q;
}
```

---

## 7. Garbage Collector

### 7.1 Motivation

Les opérations arithmétiques allouent de nombreux maillons intermédiaires sans jamais les libérer. Sans mécanisme de recyclage, la mémoire s'épuise rapidement.

### 7.2 Mécanisme (Mark-and-Sweep)

Trois éléments clés :

| Élément | Rôle |
|---------|------|
| `Monome.general` | Chaîne **tous** les maillons jamais alloués dans la liste `tousLesMaillons` |
| `Monome.utile` | Marqueur booléen : `1` (C) / `true` (Java) = maillon à conserver |
| `polyUtile[]` | Tableau des adresses de tête des polynômes à protéger |

**Passe 1 — MARQUAGE :** parcourir les polynômes de `polyUtile[]` et marquer tous leurs maillons `utile = 1`.

**Passe 2 — BALAYAGE :** parcourir `tousLesMaillons` via le champ `general` ; libérer les maillons non marqués, effacer les marques des conservés.

### 7.3 Implémentation C

```c
void enregistrerPoly(POINTEUR p) {
    if (nbPolyUtile >= MAX_POLY) { fprintf(stderr, "polyUtile plein\n"); return; }
    polyUtile[nbPolyUtile++] = p;
}

void recycler(void) {
    /* Passe 1 : marquage */
    for (int i = 0; i < nbPolyUtile; i++) {
        POINTEUR p = polyUtile[i];
        while (p != NULL) { p->utile = 1; p = p->suivant; }
    }

    /* Passe 2 : balayage — pointeur sur pointeur pour retrait sans sentinelle */
    POINTEUR *courant = &tousLesMaillons;
    int liberes = 0;
    while (*courant != NULL) {
        POINTEUR m = *courant;
        if (m->utile == 1) {
            m->utile = 0;
            courant  = &(m->general);   /* avancer */
        } else {
            *courant = m->general;      /* sauter ce maillon */
            free(m);
            liberes++;
        }
    }
    printf("[GC] %d maillon(s) libéré(s).\n", liberes);
}
```

**Point technique important (C) :** l'utilisation d'un `POINTEUR *courant` (pointeur sur pointeur) permet de retirer un maillon de la liste sans sentinelle, en modifiant directement le champ `general` du maillon précédent.

### 7.4 Implémentation Java

```java
public static void recycler() {
    /* Passe 1 : marquage */
    for (int i = 0; i < nbPolyUtile; i++) {
        Monome p = polyUtile[i];
        while (p != null) { p.utile = true; p = p.suivant; }
    }

    /* Passe 2 : balayage avec sentinelle */
    Monome sentinelle = new Monome(0, 0);
    sentinelle.general = tousLesMaillons;
    Monome pred = sentinelle;
    Monome courant = tousLesMaillons;
    int liberes = 0;

    while (courant != null) {
        Monome suivantGeneral = courant.general;
        if (courant.utile) {
            courant.utile = false;
            pred = courant;
        } else {
            pred.general = suivantGeneral;
            courant.suivant = null; courant.general = null; /* aide le GC JVM */
            liberes++;
        }
        courant = suivantGeneral;
    }
    tousLesMaillons = sentinelle.general;
    System.out.println("[GC] " + liberes + " maillon(s) libéré(s).");
}
```

**Note Java :** En Java, la mémoire réelle est récupérée par le GC natif de la JVM dès qu'il n'y a plus de références vers un objet. Notre GC applicatif reproduit le mécanisme décrit dans l'énoncé à titre **pédagogique**, en gérant explicitement les listes de maillons.

### 7.5 Transparence du mécanisme

Conformément à l'énoncé, le GC fonctionne **"en sous-sol"** : les fonctions de traitement (`plus`, `moins`, `fois`, `eval`…) n'ont pas été modifiées et ne savent pas qu'un GC existe. Seule `creerMonome` a été étendue pour enregistrer chaque nouveau maillon dans `tousLesMaillons`.

---

## 8. Versions récursives

### 8.1 Addition récursive — `plus_rec(a, b)`

**Principe de récursion :**
- Cas de base : si `a` ou `b` est `NULL`, renvoyer une copie de l'autre.
- Cas récursif : comparer les degrés de tête.
  - Si `deg(a) > deg(b)` : prendre le monôme de `a`, puis récurser sur `a->suivant` et `b`.
  - Si `deg(b) > deg(a)` : prendre le monôme de `b`, puis récurser sur `a` et `b->suivant`.
  - Si `deg(a) == deg(b)` : additionner les coefficients (ou supprimer si résultat nul), puis récurser sur les restes.

```c
/* C */
POINTEUR plus_rec(POINTEUR a, POINTEUR b) {
    if (a == NULL) return copierPoly(b);
    if (b == NULL) return copierPoly(a);

    if (a->exposant > b->exposant) {
        POINTEUR res = creerMonome(a->coefficient, a->exposant);
        res->suivant = plus_rec(a->suivant, b);
        return res;
    } else if (b->exposant > a->exposant) {
        POINTEUR res = creerMonome(b->coefficient, b->exposant);
        res->suivant = plus_rec(a, b->suivant);
        return res;
    } else {
        double c = a->coefficient + b->coefficient;
        if (c == 0.0) return plus_rec(a->suivant, b->suivant);
        POINTEUR res = creerMonome(c, a->exposant);
        res->suivant = plus_rec(a->suivant, b->suivant);
        return res;
    }
}
```

```java
/* Java */
public static Polynome plusRec(Monome a, Monome b) {
    if (a == null && b == null) return new Polynome();
    if (a == null) { Polynome r = new Polynome(); copierChaine(b, r); return r; }
    if (b == null) { Polynome r = new Polynome(); copierChaine(a, r); return r; }

    Polynome res = new Polynome();
    if (a.exposant > b.exposant) {
        Monome nv = new Monome(a.coefficient, a.exposant);
        nv.suivant = plusRec(a.suivant, b).getTete();
        res.setTete(nv);
    } else if (b.exposant > a.exposant) {
        Monome nv = new Monome(b.coefficient, b.exposant);
        nv.suivant = plusRec(a, b.suivant).getTete();
        res.setTete(nv);
    } else {
        double c = a.coefficient + b.coefficient;
        if (c == 0.0) return plusRec(a.suivant, b.suivant);
        Monome nv = new Monome(c, a.exposant);
        nv.suivant = plusRec(a.suivant, b.suivant).getTete();
        res.setTete(nv);
    }
    return res;
}
```

### 8.2 Soustraction récursive — `moins_rec(a, b)`

Structure identique à `plus_rec`, mais les coefficients de `b` sont **négativés** lors de la comparaison des degrés égaux et lors du cas de base `a == NULL`.

```c
/* C */
POINTEUR moins_rec(POINTEUR a, POINTEUR b) {
    if (a == NULL && b == NULL) return NULL;
    if (a == NULL) {
        POINTEUR neg = NULL;
        while (b != NULL) { insererTrie(&neg, -(b->coefficient), b->exposant); b = b->suivant; }
        return neg;
    }
    if (b == NULL) return copierPoly(a);

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

---

## 9. Compilation et exécution

### 9.1 Version C

```bash
# Compilation
gcc -o polymanager langage_c/polynome.c -lm -Wall

# Exécution
./polymanager
```

Le fichier `polynome.h` doit se trouver dans le même répertoire que `polynome.c`.

### 9.2 Version Java

```bash
# Compilation (depuis le dossier langage_java/)
javac Monome.java Polynome.java GarbageCollector.java Main.java MainGC.java

# Programme principal (Q1-8)
java Main

# Démonstration GC seule (Q7)
java MainGC
```

---

## 10. Exemples d'exécution

### Exemple C — entrées

```
Entrez le polynôme P : -4.5 * X^5 + 2 * X^4 + X^3 - X + 123
Entrez le polynôme Q : X^2 + 1
```

### Sortie attendue

```
P = -4.5X^5 + 2X^4 + X^3 - X + 123
Q = X^2 + 1
P(1) = 121
Q(1) = 2

[Q7] Polynôme temporaire (non enregistré) = 3X^2 + 1
[Q7] Enregistrement de P et Q comme utiles...
[Q7] Lancement du GC...
[GC] 2 maillon(s) libéré(s).

Après GC — P et Q intacts :
P = -4.5X^5 + 2X^4 + X^3 - X + 123
Q = X^2 + 1

=== Q6 : Opérations arithmétiques ===
P + Q = -4.5X^5 + 2X^4 + X^3 + X^2 - X + 124
P - Q = -4.5X^5 + 2X^4 + X^3 - X^2 - X + 122
P * Q = -4.5X^7 + 2X^6 + X^5 - X^3 + 122.5X^2 - X + 123
P / Q = -4.5X^3 + 2X^2 + 5.5X - 3
reste = 126.5X - 126

=== Q8 : Versions récursives ===
P + Q (récursif) = -4.5X^5 + 2X^4 + X^3 + X^2 - X + 124
P - Q (récursif) = -4.5X^5 + 2X^4 + X^3 - X^2 - X + 122
```

### Exemple Java — GC (MainGC)

```
=== PolyManager DIC2 — Q7 Garbage Collector (Java) ===

P = X^3 + 2X + 1
Q = X + 1

[Q7] Polynôme temporaire (non enregistré) = 3X^2 + 1
[Q7] Enregistrement de P et Q comme utiles...
[Q7] Lancement du GC...
[GC] 2 maillon(s) libéré(s).

Après GC — P et Q intacts :
P = X^3 + 2X + 1
Q = X + 1
```

---

## Arborescence du projet

```
PolyManager-DIC2/
├── langage_c/
│   ├── polynome.h        # Structures, types, prototypes Q1-8
│   ├── polynome.c        # Implémentation complète Q1-8 + main()
│   └── polynome.txt      # Pseudo-code algorithmique (phase de conception)
└── langage_java/
    ├── Monome.java            # Structure d'un maillon (avec champs GC)
    ├── Polynome.java          # Q1-8 : analyseur, affichage, éval, opérations
    ├── GarbageCollector.java  # Q7 : mécanisme mark-and-sweep Java
    ├── Main.java              # Programme principal (démo Q1-8)
    └── MainGC.java            # Démo standalone du GC (Q7)
```

---

*Rapport — PolyManager DIC2, DIC 2, Semestre 1, POO Avancée.*
