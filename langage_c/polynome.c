#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "polynome.h"
#include <string.h>


/* =====================================================
   creerMonome
   ===================================================== */

Monome* creerMonome(double coef, int exp) {
    Monome *p = malloc(sizeof(Monome));
    if (!p) {
        fprintf(stderr, "ERREUR : malloc a echoue\n");
        exit(EXIT_FAILURE);
    }
    p->exposant = exp;
    p->coefficient = coef;
    p->suivant = NULL;
    return p;
}


/* =====================================================
   insererTrieMonome
   ===================================================== */

void insererTrieMonome(Monome **tete, double coef, int exp) {
    Monome *nouveau = creerMonome(coef, exp);
    Monome *p, *precedent;

    // Cas 1 : liste vide OU exposant plus grand que la tête
    if (*tete == NULL || exp > (*tete)->exposant) {
        nouveau->suivant = *tete;
        *tete = nouveau;

    // Cas 2 : exposant = 0 → toujours en fin de liste
    } else if (exp == 0) {
        precedent = *tete;
        while (precedent->suivant != NULL)
            precedent = precedent->suivant;
        precedent->suivant = nouveau;

    // Cas 3 : chercher la bonne position
    } else {
        precedent = *tete;
        p = (*tete)->suivant;
        while (p != NULL && p->exposant >= exp) {
            precedent = p;
            p = p->suivant;
        }
        nouveau->suivant = p;
        precedent->suivant = nouveau;
    }
}


/* =====================================================
   afficherPolynome
   ===================================================== */

void afficherPolynome(Monome *tete) {
    if (tete == NULL) {
        printf("0\n");
        return;
    }

    Monome *p = tete;
    int premier = 1;
    double coef;

    while (p != NULL) {
        coef = p->coefficient;

        if (premier) {
            if (coef < 0)
                printf("-");
            premier = 0;
        } else {
            if (coef >= 0)
                printf(" + ");
            else
                printf(" - ");
        }

        coef = fabs(coef);

        if (p->exposant == 0) {
            printf("%g", coef);
        } else if (p->exposant == 1) {
            if (coef != 1)
                printf("%g*", coef);
            printf("X");
        } else {
            if (coef != 1)
                printf("%g*", coef);
            printf("X^%d", p->exposant);
        }

        p = p->suivant;
    }
    printf("\n");
}


/* =====================================================
   skipEspaces
   ===================================================== */

void skipEspaces(char *ch, int *pos) {
    while (ch[*pos] == ' ')
        (*pos)++;
}


/* =====================================================
   analyseNaturel
   ===================================================== */

int analyseNaturel(char *ch, int *pos) {
    if (!isdigit((unsigned char)ch[*pos])) {
        fprintf(stderr, "ERREUR : chiffre attendu\n");
        exit(EXIT_FAILURE);
    }
    int val = 0;
    while (isdigit((unsigned char)ch[*pos])) {
        val = val * 10 + (ch[*pos] - '0');
        (*pos)++;
    }
    return val;
}


/* =====================================================
   analyseNombre
   ===================================================== */

double analyseNombre(char *ch, int *pos) {
    double partieEntiere = analyseNaturel(ch, pos);

    if (ch[*pos] == '.') {
        (*pos)++;
        double partieDec = 0;
        double nbDecimales = 0.1;
        while (isdigit((unsigned char)ch[*pos])) {
            partieDec = partieDec + (ch[*pos] - '0') * nbDecimales;
            nbDecimales = nbDecimales * 0.1;
            (*pos)++;
        }
        return partieEntiere + partieDec;
    }

    return partieEntiere;
}


/* =====================================================
   analyseXpuissance
   ===================================================== */

int analyseXpuissance(char *ch, int *pos) {
    if (ch[*pos] != 'X') {
        fprintf(stderr, "ERREUR : 'X' attendu\n");
        exit(EXIT_FAILURE);
    }
    (*pos)++;

    if (ch[*pos] == '^') {
        (*pos)++;
        return analyseNaturel(ch, pos);
    }

    return 1;
}


/* =====================================================
   analyseMonome
   ===================================================== */

void analyseMonome(char *ch, int *pos, Monome **tete, int signe) {
    double coef = 0;
    int expo = 0;

    skipEspaces(ch, pos);

    if (ch[*pos] == 'X') {
        coef = 1;
        expo = analyseXpuissance(ch, pos);

    } else if (isdigit((unsigned char)ch[*pos])) {
        coef = analyseNombre(ch, pos);
        skipEspaces(ch, pos);

        if (ch[*pos] == '*') {
            (*pos)++;
            skipEspaces(ch, pos);
            expo = analyseXpuissance(ch, pos);
        } else {
            expo = 0;
        }

    } else {
        fprintf(stderr, "Polynome vide\n");
        exit(EXIT_FAILURE);
    }

    insererTrieMonome(tete, coef * signe, expo);
}


/* =====================================================
   analyserPolynome
   ===================================================== */

void analyserPolynome(char *ch, Monome **tete) {
    int pos = 0;
    int signe = 1;

    skipEspaces(ch, &pos);

    if (ch[pos] == '-') {
        signe = -1;
        pos++;
    }

    analyseMonome(ch, &pos, tete, signe);

    while (ch[pos] != '\0') {
        skipEspaces(ch, &pos);

        if (ch[pos] == '+') {
            signe = 1;
            pos++;
        } else if (ch[pos] == '-') {
            signe = -1;
            pos++;
        } else if (ch[pos] != '\0') {
            fprintf(stderr, "ERREUR : '+' ou '-' attendu\n");
            exit(EXIT_FAILURE);
        }

        analyseMonome(ch, &pos, tete, signe);
    }
}


/* =====================================================
   main
   ===================================================== */

int main(void) {
    char chaine[256];
    Monome *tete = NULL;

    printf("Entrez votre polynome (ex: -4.5*X^5 + 2*X^4 + X^3 - X + 123) :\n");
    fgets(chaine, sizeof(chaine), stdin);

    // Enlever le \n laissé par fgets
    int len = strlen(chaine);
    if (len > 0 && chaine[len - 1] == '\n')
        chaine[len - 1] = '\0';

    analyserPolynome(chaine, &tete);

    printf("Polynome reconnu : ");
    afficherPolynome(tete);

    return 0;
}