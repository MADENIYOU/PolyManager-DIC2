/*
 * polynome.c
 *
 * Projet PolyManager DIC2
 * Questions 1 à 5 : fusion travaux de Kane et Sidibé
 * Question 7      : Garbage Collector — Madeniyou Sall
 */

#include "polynome.h"

/* =====================================================
   Variables globales (Question 7)
   ===================================================== */

POINTEUR tousLesMaillons  = NULL;
POINTEUR polyUtile[MAX_POLY];
int      nbPolyUtile       = 0;


/* =====================================================
   Question 2 : Allocation d'un maillon
   (avec enregistrement dans la liste générale — Q7)
   ===================================================== */

POINTEUR creerMonome(double coef, int exp) {
    POINTEUR p = (POINTEUR)malloc(sizeof(Monome));
    if (!p) {
        fprintf(stderr, "Erreur : malloc a échoué\n");
        exit(EXIT_FAILURE);
    }
    p->coefficient = coef;
    p->exposant    = exp;
    p->suivant     = NULL;

    /* Q7 : chaîner dans la liste générale de tous les maillons */
    p->utile   = 0;
    p->general = tousLesMaillons;
    tousLesMaillons = p;

    return p;
}


/* =====================================================
   Question 4 : Insertion triée (degrés décroissants)
   ===================================================== */

void insererTrie(POINTEUR *tete, double coef, int exp) {
    if (coef == 0.0) return;

    /* Si le degré existe déjà, cumuler les coefficients */
    POINTEUR curr = *tete;
    while (curr != NULL) {
        if (curr->exposant == exp) {
            curr->coefficient += coef;
            return;
        }
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


/* =====================================================
   Questions 1 & 2 : Analyseur syntaxique
   ===================================================== */

static void passerEspaces(char *s, int *i) {
    while (s[*i] == ' ' || s[*i] == '\t') (*i)++;
}

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

static void analyserMonome(char *s, int *i, POINTEUR *poly, int signe) {
    double c = 1.0;
    int    e = 0;

    passerEspaces(s, i);

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


/* =====================================================
   Question 3 : Affichage
   ===================================================== */

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


/* =====================================================
   Question 5 : Évaluation
   ===================================================== */

double eval(POINTEUR p, double x) {
    double res = 0.0;
    while (p != NULL) {
        res += p->coefficient * pow(x, (double)p->exposant);
        p = p->suivant;
    }
    return res;
}


/* =====================================================
   Question 7 : Garbage Collector
   ===================================================== */

/*
 * Enregistrer un polynôme comme « utile » afin que le GC
 * ne libère pas ses maillons.
 */
void enregistrerPoly(POINTEUR p) {
    if (nbPolyUtile >= MAX_POLY) {
        fprintf(stderr, "Avertissement : polyUtile plein\n");
        return;
    }
    polyUtile[nbPolyUtile++] = p;
}

/*
 * recycler — deux passes :
 *
 *  Passe 1 (MARQUAGE)
 *    Pour chaque polynôme de polyUtile[], parcourir ses maillons
 *    et mettre utile = 1.
 *
 *  Passe 2 (BALAYAGE)
 *    Parcourir tousLesMaillons (via ->general) :
 *      - utile == 1 → effacer la marque, conserver le maillon
 *      - utile == 0 → retirer de la liste générale, free()
 */
void recycler(void) {
    /* --- Passe 1 : marquage --- */
    for (int i = 0; i < nbPolyUtile; i++) {
        POINTEUR p = polyUtile[i];
        while (p != NULL) {
            p->utile = 1;
            p = p->suivant;
        }
    }

    /* --- Passe 2 : balayage --- */
    POINTEUR *courant = &tousLesMaillons;
    int liberes = 0;

    while (*courant != NULL) {
        POINTEUR m = *courant;
        if (m->utile == 1) {
            m->utile = 0;              /* effacer la marque */
            courant  = &(m->general); /* passer au suivant */
        } else {
            *courant = m->general;    /* sauter ce maillon */
            free(m);
            liberes++;
        }
    }

    printf("[GC] %d maillon(s) libéré(s).\n", liberes);
}


/* =====================================================
   main — démonstration Q1-5 + Q7
   ===================================================== */

int main(void) {
    char buf[512];
    POINTEUR p = NULL, q = NULL;

    printf("=== PolyManager DIC2 — Q1-5 + Q7 GC ===\n\n");

    printf("Entrez le polynôme P : ");
    if (!fgets(buf, sizeof(buf), stdin)) return 1;
    buf[strcspn(buf, "\n")] = '\0';
    analyserPolynome(buf, &p);

    printf("Entrez le polynôme Q : ");
    if (!fgets(buf, sizeof(buf), stdin)) return 1;
    buf[strcspn(buf, "\n")] = '\0';
    analyserPolynome(buf, &q);

    printf("\nP = "); afficherPolynome(p);
    printf("Q = "); afficherPolynome(q);
    printf("P(1) = %g\n", eval(p, 1.0));
    printf("Q(1) = %g\n", eval(q, 1.0));

    /* --- Q7 : on crée un polynôme temporaire non-enregistré ---
       Il simule un résultat intermédiaire (ex: résultat de plus/fois)
       dont personne ne gardera l'adresse. */
    POINTEUR tmp = NULL;
    insererTrie(&tmp, 3.0, 2);
    insererTrie(&tmp, 1.0, 0);
    printf("\n[Q7] Polynôme temporaire (non enregistré) = ");
    afficherPolynome(tmp);
    /* tmp n'est PAS enregistré dans polyUtile → ses maillons seront libérés */

    printf("[Q7] Enregistrement de P et Q comme utiles...\n");
    enregistrerPoly(p);
    enregistrerPoly(q);

    printf("[Q7] Lancement du GC...\n");
    recycler();

    printf("\nAprès GC — P et Q intacts :\n");
    printf("P = "); afficherPolynome(p);
    printf("Q = "); afficherPolynome(q);

    return 0;
}
