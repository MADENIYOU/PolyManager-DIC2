/* ============================================================
   Toutes les opérations ITÉRATIVES sur les polynômes :
     - plus(a, b)               : addition
     - moins(a, b)              : soustraction
     - fois(a, b)               : multiplication
     - quotient(a, b, *reste)   : division euclidienne
   Représentation : liste chaînée de monômes triée par
   ordre DÉCROISSANT des exposants.
   ============================================================ */

#include <stdio.h>
#include <stdlib.h>


typedef struct Maillon {
    double coefficient;
    int    exposant;
    struct Maillon *suivant;
} Maillon;

typedef Maillon *POINTEUR;


POINTEUR nouveau_maillon(double coef, int expo) {
    POINTEUR m = malloc(sizeof(Maillon));
    if (!m) { perror("malloc"); exit(EXIT_FAILURE); }
    m->coefficient = coef;
    m->exposant    = expo;
    m->suivant     = NULL;
    return m;
}


void ajouter_en_queue(POINTEUR *tete, POINTEUR *queue, POINTEUR m) {
    if (*tete == NULL) {
        *tete  = m;
        *queue = m;
    } else {
        (*queue)->suivant = m;
        *queue = m;
    }
}

void afficher(POINTEUR p) {
    if (!p) { printf("0\n"); return; }
    int premier = 1;
    while (p) {
        if (!premier && p->coefficient >= 0) printf(" + ");
        else if (!premier)                    printf(" ");
        if (p->exposant == 0)
            printf("%.2f", p->coefficient);
        else if (p->exposant == 1)
            printf("%.2f*X", p->coefficient);
        else
            printf("%.2f*X^%d", p->coefficient, p->exposant);
        premier = 0;
        p = p->suivant;
    }
    printf("\n");
}


POINTEUR plus(POINTEUR a, POINTEUR b) {

    POINTEUR tete  = NULL;
    POINTEUR queue = NULL;

    while (a != NULL && b != NULL) {

        if (a->exposant > b->exposant) {
            POINTEUR m = nouveau_maillon(a->coefficient, a->exposant);
            ajouter_en_queue(&tete, &queue, m);
            a = a->suivant;

        } else if (b->exposant > a->exposant) {
            POINTEUR m = nouveau_maillon(b->coefficient, b->exposant);
            ajouter_en_queue(&tete, &queue, m);
            b = b->suivant;

        } else {
            double somme = a->coefficient + b->coefficient;
            if (somme != 0.0) {
                POINTEUR m = nouveau_maillon(somme, a->exposant);
                ajouter_en_queue(&tete, &queue, m);
            }
            a = a->suivant;
            b = b->suivant;
        }
    }

    if (a != NULL) ajouter_en_queue(&tete, &queue, a);
    if (b != NULL) ajouter_en_queue(&tete, &queue, b);

    return tete;
}


POINTEUR moins(POINTEUR a, POINTEUR b) {

    POINTEUR tete  = NULL;
    POINTEUR queue = NULL;

    while (a != NULL && b != NULL) {

        if (a->exposant > b->exposant) {
            POINTEUR m = nouveau_maillon(a->coefficient, a->exposant);
            ajouter_en_queue(&tete, &queue, m);
            a = a->suivant;

        } else if (b->exposant > a->exposant) {
            POINTEUR m = nouveau_maillon(-(b->coefficient), b->exposant);
            ajouter_en_queue(&tete, &queue, m);
            b = b->suivant;

        } else {
            double diff = a->coefficient - b->coefficient;
            if (diff != 0.0) {
                POINTEUR m = nouveau_maillon(diff, a->exposant);
                ajouter_en_queue(&tete, &queue, m);
            }
            a = a->suivant;
            b = b->suivant;
        }
    }

    if (a != NULL) ajouter_en_queue(&tete, &queue, a);

    while (b != NULL) {
        POINTEUR m = nouveau_maillon(-(b->coefficient), b->exposant);
        ajouter_en_queue(&tete, &queue, m);
        b = b->suivant;
    }

    return tete;
}

POINTEUR fois(POINTEUR a, POINTEUR b) {

    POINTEUR resultat = NULL; 

    POINTEUR curseur_a = a;
    while (curseur_a != NULL) {

        POINTEUR curseur_b = b;
        while (curseur_b != NULL) {

            double coef_produit = curseur_a->coefficient * curseur_b->coefficient;
            int    expo_produit = curseur_a->exposant    + curseur_b->exposant;

            POINTEUR terme = nouveau_maillon(coef_produit, expo_produit);

            resultat = plus(resultat, terme);

            curseur_b = curseur_b->suivant;
        }

        curseur_a = curseur_a->suivant;
    }

    return resultat;
}

POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste) {

    if (b == NULL) {
        fprintf(stderr, "Erreur : division par le polynome nul.\n");
        *reste = a;
        return NULL;
    }

    POINTEUR quotient_tete  = NULL;
    POINTEUR quotient_queue = NULL;

    POINTEUR dividende = a;

    while (dividende != NULL && dividende->exposant >= b->exposant) {

        double coef_q = dividende->coefficient / b->coefficient;
        int    expo_q = dividende->exposant    - b->exposant;

        POINTEUR terme_q = nouveau_maillon(coef_q, expo_q);
        ajouter_en_queue(&quotient_tete, &quotient_queue, terme_q);

        POINTEUR mono_q          = nouveau_maillon(coef_q, expo_q);
        POINTEUR produit_partiel = fois(mono_q, b);

        dividende = moins(dividende, produit_partiel);
    }

    *reste = dividende;

    return quotient_tete;
}


int main(void) {

    POINTEUR P = nouveau_maillon(3.0, 3);
    P->suivant = nouveau_maillon(2.0, 2);
    P->suivant->suivant = nouveau_maillon(5.0, 0);

    POINTEUR Q = nouveau_maillon(4.0, 3);
    Q->suivant = nouveau_maillon(-2.0, 2);
    Q->suivant->suivant = nouveau_maillon(7.0, 1);

    printf("========================================\n");
    printf("P = "); afficher(P);
    printf("Q = "); afficher(Q);
    printf("========================================\n\n");

    printf("--- ADDITION ---\n");
    POINTEUR S = plus(P, Q);
    printf("P + Q = "); afficher(S);

    printf("\n--- SOUSTRACTION ---\n");
    POINTEUR D = moins(P, Q);
    printf("P - Q = "); afficher(D);

    printf("\n--- MULTIPLICATION ---\n");
    POINTEUR PQ = fois(P, Q);
    printf("P * Q = "); afficher(PQ);

    printf("\n--- DIVISION ---\n");

    POINTEUR A = nouveau_maillon(1.0, 4);
    A->suivant = nouveau_maillon(3.0, 3);
    A->suivant->suivant = nouveau_maillon(-2.0, 2);
    A->suivant->suivant->suivant = nouveau_maillon(4.0, 1);
    A->suivant->suivant->suivant->suivant = nouveau_maillon(-1.0, 0);

    POINTEUR B = nouveau_maillon(1.0, 2);
    B->suivant = nouveau_maillon(1.0, 0);

    printf("A = "); afficher(A);
    printf("B = "); afficher(B);

    POINTEUR reste_r = NULL;
    POINTEUR Quot = quotient(A, B, &reste_r);

    printf("A / B  (quotient) = "); afficher(Quot);
    printf("A / B  (reste)    = "); afficher(reste_r);

    printf("\nVerification B*Q + R = A :\n");
    POINTEUR verif = plus(fois(B, Quot), reste_r);
    printf("B*Q + R = "); afficher(verif);

    return 0;
}