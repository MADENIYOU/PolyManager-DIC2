#include "polynome.h"

/* Initialisation de la liste globale pour la Question 7 */
POINTEUR tousLesMaillons = NULL;

/* --- Question 2 : Allocation et Codage --- */
POINTEUR creerMonome(double coef, int exp) {
    POINTEUR p = (POINTEUR)malloc(sizeof(Monome));
    if (!p) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    p->coefficient = coef;
    p->exposant = exp;
    p->suivant = NULL;

    /* Question 7 : Ajout systématique à la liste de suivi général */
    p->utile = 0;
    p->general = tousLesMaillons;
    tousLesMaillons = p;

    return p;
}

/* --- Question 4 : Codage par degré décroissant --- */
void insererTrie(POINTEUR *tete, double coef, int exp) {
    if (coef == 0) return; /* Le polynôme nul est une liste vide [cite: 22] */

    /* Optimisation : si le degré existe déjà, on additionne les coefficients */
    POINTEUR curr = *tete;
    while (curr != NULL) {
        if (curr->exposant == exp) {
            curr->coefficient += coef;
            return;
        }
        curr = curr->suivant;
    }

    POINTEUR nouveau = creerMonome(coef, exp);
    /* Insertion en tête ou dans une liste vide */
    if (*tete == NULL || exp > (*tete)->exposant) {
        nouveau->suivant = *tete;
        *tete = nouveau;
    } else {
        /* Recherche de la position d'insertion pour garder l'ordre décroissant */
        POINTEUR prec = *tete;
        while (prec->suivant != NULL && prec->suivant->exposant > exp) {
            prec = prec->suivant;
        }
        nouveau->suivant = prec->suivant;
        prec->suivant = nouveau;
    }
}

/* --- Question 1 & 2 : Analyseur Syntaxique --- */
static void passerEspaces(char *s, int *i) {
    while (s[*i] == ' ') (*i)++;
}

static int lireNaturel(char *s, int *i) {
    int n = 0;
    while (isdigit(s[*i])) {
        n = n * 10 + (s[*i] - '0');
        (*i)++;
    }
    return n;
}

static double lireNombre(char *s, int *i) {
    double n = (double)lireNaturel(s, i);
    if (s[*i] == '.') {
        (*i)++;
        double frac = 0, div = 10;
        while (isdigit(s[*i])) {
            frac += (s[*i] - '0') / div;
            div *= 10;
            (*i)++;
        }
        n += frac;
    }
    return n;
}

static void analyserMonome(char *s, int *i, POINTEUR *poly, int signe) {
    double c = 1.0;
    int e = 0;
    passerEspaces(s, i);

    if (isdigit(s[*i])) {
        c = lireNombre(s, i);
        passerEspaces(s, i);
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
        fprintf(stderr, "Erreur de syntaxe : monôme invalide\n"); [cite: 19]
        exit(EXIT_FAILURE);
    }
    insererTrie(poly, c * signe, e);
}

void analyserPolynome(char *s, POINTEUR *poly) {
    int i = 0, signe = 1;
    passerEspaces(s, &i);
    
    if (s[i] == '-') { signe = -1; i++; }
    else if (s[i] == '+') i++;

    analyserMonome(s, &i, poly, signe);
    passerEspaces(s, &i);

    while (s[i] != '\0' && s[i] != '\n') {
        if (s[i] == '+') signe = 1;
        else if (s[i] == '-') signe = -1;
        else break;
        i++;
        analyserMonome(s, &i, poly, signe);
        passerEspaces(s, &i);
    }
}

/* --- Question 3 : Affichage --- */
void afficherPolynome(POINTEUR p) {
    if (p == NULL) { printf("0\n"); return; } [cite: 22]
    int premier = 1;
    while (p != NULL) {
        if (!premier && p->coefficient > 0) printf(" + ");
        else if (p->coefficient < 0) printf(" - ");
        
        double c = fabs(p->coefficient);
        if (c != 1 || p->exposant == 0) printf("%g", c);
        
        if (p->exposant > 0) {
            printf("X");
            if (p->exposant > 1) printf("^%d", p->exposant);
        }
        premier = 0;
        p = p->suivant;
    }
    printf("\n");
}

/* --- Question 5 : Évaluation --- */
double eval(POINTEUR p, double x) { [cite: 28]
    double res = 0;
    while (p != NULL) {
        res += p->coefficient * pow(x, (double)p->exposant); [cite: 29]
        p = p->suivant;
    }
    return res;
}