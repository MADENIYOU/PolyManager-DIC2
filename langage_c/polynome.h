#ifndef POLYNOME_H
#define POLYNOME_H

/* =====================================================
   Structure
   ===================================================== */

typedef struct Monome {
    int exposant;
    double coefficient;
    struct Monome *suivant;
} Monome;


/* =====================================================
   Prototypes
   ===================================================== */

Monome* creerMonome(double coef, int exp);

void insererTrieMonome(Monome **tete, double coef, int exp);

void afficherPolynome(Monome *tete);

void skipEspaces(char *ch, int *pos);

int analyseNaturel(char *ch, int *pos);

double analyseNombre(char *ch, int *pos);

int analyseXpuissance(char *ch, int *pos);

void analyseMonome(char *ch, int *pos, Monome **tete, int signe);

void analyserPolynome(char *ch, Monome **tete);

#endif