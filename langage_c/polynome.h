#ifndef POLYNOME_H
#define POLYNOME_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

/* =====================================================
   Question 2 & 7 : Définition de la structure
   ===================================================== */
typedef struct Monome {
    double coefficient;
    int    exposant;
    struct Monome *suivant;     /* liste triée par degré décroissant */

    /* --- Champs Garbage Collector (Question 7) --- */
    struct Monome *general;     /* chaîne TOUS les maillons alloués  */
    int utile;                  /* marqueur : 1 = utile, 0 = libérer */
} Monome, *POINTEUR;

/* =====================================================
   Variables globales GC (Question 7)
   ===================================================== */
extern POINTEUR tousLesMaillons;
#define MAX_POLY 100
extern POINTEUR polyUtile[MAX_POLY];
extern int      nbPolyUtile;

/* =====================================================
   Prototypes Q1-5 (Kane + Sidibé)
   ===================================================== */
POINTEUR creerMonome(double coef, int exp);
void     insererTrie(POINTEUR *tete, double coef, int exp);
void     analyserPolynome(char *s, POINTEUR *poly);
void     afficherPolynome(POINTEUR p);
double   eval(POINTEUR p, double x);

/* =====================================================
   Prototypes Q7 : Garbage Collector
   ===================================================== */
void enregistrerPoly(POINTEUR p);
void recycler(void);

/* =====================================================
   Prototypes Q6 : Opérations arithmétiques
   ===================================================== */
POINTEUR plus(POINTEUR a, POINTEUR b);
POINTEUR moins(POINTEUR a, POINTEUR b);
POINTEUR fois(POINTEUR a, POINTEUR b);
POINTEUR quotient(POINTEUR a, POINTEUR b, POINTEUR *reste);

/* =====================================================
   Prototypes Q8 : Versions récursives addition/soustraction
   ===================================================== */
POINTEUR plus_rec(POINTEUR a, POINTEUR b);
POINTEUR moins_rec(POINTEUR a, POINTEUR b);

#endif
