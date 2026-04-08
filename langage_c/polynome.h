#ifndef POLYNOME_H
#define POLYNOME_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

/* Question 2 & 7 : Définition de la structure de données */
typedef struct Monome {
    double coefficient;    /* Coefficient du monôme */
    int exposant;         /* Degré du monôme */
    struct Monome *suivant; /* Pointeur vers le monôme suivant (liste triée) */

    /* Champs spécifiques pour le Garbage Collector (Question 7) */
    struct Monome *general; /* Chaînage de tous les maillons alloués */
    int utile;              /* Marqueur pour le recyclage */
} Monome, *POINTEUR;

/* Variables globales pour le mécanisme de recyclage (Question 7) */
extern POINTEUR tousLesMaillons;
extern POINTEUR polyUtile[100];

/* Prototypes des fonctions (Questions 1 à 5) */
POINTEUR creerMonome(double coef, int exp);
void insererTrie(POINTEUR *tete, double coef, int exp);
void analyserPolynome(char *s, POINTEUR *poly);
void afficherPolynome(POINTEUR p);
double eval(POINTEUR p, double x);

#endif