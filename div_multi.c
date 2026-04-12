#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct Node {
    double coeff;
    int exp;
    struct Node *next;
} Node, *POINTEUR;

// Fonction utilitaire pour créer/ajouter un monôme à la fin
POINTEUR creerMonome(double c, int e) {
    if (c == 0) return NULL;
    POINTEUR nouveau = (POINTEUR)malloc(sizeof(Node));
    nouveau->coeff = c;
    nouveau->exp = e;
    nouveau->next = NULL;
    return nouveau;
}

// --- MULTIPLICATION (Question 6) ---
// Utilise la formule : P * Q = (a X^n * Q) + (P_reste * Q)
POINTEUR fois(POINTEUR a, POINTEUR b) {
    if (a == NULL || b == NULL) return NULL;

    POINTEUR resultat = NULL;
    for (POINTEUR pA = a; pA != NULL; pA = pA->next) {
        for (POINTEUR pB = b; pB != NULL; pB = pB->next) {
            double c = pA->coeff * pB->coeff;
            int e = pA->exp + pB->exp;
            // Ici, il faudrait idéalement une fonction 'ajouter_ou_sommer' 
            // pour maintenir l'ordre décroissant et combiner les termes.
            // Pour simplifier selon l'énoncé :
            POINTEUR temp = creerMonome(c, e);
            resultat = plus(resultat, temp); // Supposant 'plus' déjà écrit
        }
    }
    return resultat;
}

// --- DIVISION (Question 6) ---
// Calcule le quotient et remplit le reste
POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste) {
    if (b == NULL) return NULL; // Division par zéro

    POINTEUR q = NULL;
    POINTEUR r = copierPolynome(a); // Copie de l'original pour ne pas le modifier

    while (r != NULL && r->exp >= b->exp) {
        double c = r->coeff / b->coeff;
        int e = r->exp - b->exp;
        
        POINTEUR monomeQuotient = creerMonome(c, e);
        q = plus(q, monomeQuotient);

        // r = r - (monomeQuotient * b)
        POINTEUR temp = fois(monomeQuotient, b);
        r = moins(r, temp); // Supposant 'moins' déjà écrit
    }
    *reste = r;
    return q;
}
