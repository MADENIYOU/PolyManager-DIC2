#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Monome {
    double coefficient;
    int exposant;
    struct Monome *suivant;
} Monome, *POINTEUR;

static const double EPS = 1e-12;

static int estNul(double x) {
    return fabs(x) < EPS;
}

static POINTEUR creerMonome(double coefficient, int exposant, POINTEUR suivant) {
    Monome *m = (Monome *)malloc(sizeof(Monome));
    if (!m) {
        fprintf(stderr, "Erreur: allocation memoire impossible.\n");
        exit(EXIT_FAILURE);
    }
    m->coefficient = coefficient;
    m->exposant = exposant;
    m->suivant = suivant;
    return m;
}

static POINTEUR copierPoly(POINTEUR p) {
    if (!p) return NULL;
    return creerMonome(p->coefficient, p->exposant, copierPoly(p->suivant));
}

static POINTEUR opposeRec(POINTEUR p) {
    if (!p) return NULL;
    return creerMonome(-p->coefficient, p->exposant, opposeRec(p->suivant));
}

/* Q8: version recursive de plus */
POINTEUR plus(POINTEUR a, POINTEUR b) {
    if (!a) return copierPoly(b);
    if (!b) return copierPoly(a);

    if (a->exposant == b->exposant) {
        double somme = a->coefficient + b->coefficient;
        POINTEUR suite = plus(a->suivant, b->suivant);
        if (estNul(somme)) return suite;
        return creerMonome(somme, a->exposant, suite);
    }
    if (a->exposant > b->exposant) {
        return creerMonome(a->coefficient, a->exposant, plus(a->suivant, b));
    }
    return creerMonome(b->coefficient, b->exposant, plus(a, b->suivant));
}

/* Q8: version recursive de moins */
POINTEUR moins(POINTEUR a, POINTEUR b) {
    if (!a) return opposeRec(b);
    if (!b) return copierPoly(a);

    if (a->exposant == b->exposant) {
        double diff = a->coefficient - b->coefficient;
        POINTEUR suite = moins(a->suivant, b->suivant);
        if (estNul(diff)) return suite;
        return creerMonome(diff, a->exposant, suite);
    }
    if (a->exposant > b->exposant) {
        return creerMonome(a->coefficient, a->exposant, moins(a->suivant, b));
    }
    return creerMonome(-b->coefficient, b->exposant, moins(a, b->suivant));
}

static void libererPoly(POINTEUR p) {
    while (p) {
        POINTEUR suivant = p->suivant;
        free(p);
        p = suivant;
    }
}

static POINTEUR depuisTableau(const double coeffs[], const int exposants[], int n) {
    POINTEUR tete = NULL;
    POINTEUR *queue = &tete;
    for (int i = 0; i < n; i++) {
        if (estNul(coeffs[i])) continue;
        *queue = creerMonome(coeffs[i], exposants[i], NULL);
        queue = &((*queue)->suivant);
    }
    return tete;
}

static void afficherPoly(POINTEUR p) {
    if (!p) {
        printf("0");
        return;
    }

    int premier = 1;
    while (p) {
        double c = p->coefficient;
        int e = p->exposant;

        if (!premier) {
            printf(c < 0 ? " - " : " + ");
        } else if (c < 0) {
            printf("-");
        }

        double absC = fabs(c);
        if (e == 0) {
            printf("%.2f", absC);
        } else if (e == 1) {
            if (!estNul(absC - 1.0)) printf("%.2f*", absC);
            printf("X");
        } else {
            if (!estNul(absC - 1.0)) printf("%.2f*", absC);
            printf("X^%d", e);
        }

        premier = 0;
        p = p->suivant;
    }
}

int main(void) {
    double cA[] = {4, -2, 1};
    int eA[] = {3, 1, 0};
    double cB[] = {1, 5, -3};
    int eB[] = {2, 1, 0};

    POINTEUR a = depuisTableau(cA, eA, 3);
    POINTEUR b = depuisTableau(cB, eB, 3);

    POINTEUR sRec = plus(a, b);
    POINTEUR dRec = moins(a, b);

    printf("A = "); afficherPoly(a); printf("\n");
    printf("B = "); afficherPoly(b); printf("\n");
    printf("plus(A,B) [recursif] = "); afficherPoly(sRec); printf("\n");
    printf("moins(A,B) [recursif] = "); afficherPoly(dRec); printf("\n");

    libererPoly(a);
    libererPoly(b);
    libererPoly(sRec);
    libererPoly(dRec);
    return 0;
}
