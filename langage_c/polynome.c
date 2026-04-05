#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef struct Maillon {
    double coefficient;
    int exposant;
    struct Maillon *suivant;
} Maillon;

typedef Maillon *POINTEUR;

static const char *input_str = NULL; /* chaîne en cours d'analyse */
static int pos = 0;    /* position courante         */

/* ---- utilitaires de lecture ---- */
static char courant(){ return input_str[pos]; }
static void avancer(){ pos++; }
static void skipEspaces(){ while (courant() == ' ' || courant() == '\t') avancer(); }

static void erreurSyntaxe(const char *msg) {
    fprintf(stderr, "\nErreur de syntaxe à la position %d : %s\n"
                    "  -> près de : '...%s'\n", pos, msg, input_str + pos);
    exit(1);
}

/* ---- naturel ---- */
static int lireNaturel() {
    if (!isdigit((unsigned char)courant()))
        erreurSyntaxe("chiffre attendu");
    int val = 0;
    while (isdigit((unsigned char)courant())) {
        val = val * 10 + (courant() - '0');
        avancer();
    }
    return val;
}

/* ---- nombre (double) ---- */
static double lireNombre() {
    double entiere = (double)lireNaturel();
    double frac = 0.0;
    if (courant() == '.') {
        avancer();
        double diviseur = 10.0;
        while (isdigit((unsigned char)courant())) {
            frac += (courant() - '0') / diviseur;
            diviseur *= 10.0;
            avancer();
        }
    }
    return entiere + frac;
}

/* ---- xpuissance : renvoie l'exposant ---- */
static int lireXpuissance() {
    if (courant() != 'X') erreurSyntaxe("'X' attendu");
    avancer();
    if (courant() == '^') {
        avancer();
        return lireNaturel();
    }
    return 1; /* X seul → exposant 1 */
}

/* ---- monôme : renvoie (coef, exp) via pointeurs ---- */
static void lireMonome(double *coef, int *exp) {
    skipEspaces();
    /* cas : nombre '*' xpuissance  OU  nombre seul */
    if (isdigit((unsigned char)courant())) {
        double nb = lireNombre();
        skipEspaces();
        if (courant() == '*') {
            avancer();
            skipEspaces();
            *coef = nb;
            *exp  = lireXpuissance();
        } else {
            /* nombre seul → exposant 0 */
            *coef = nb;
            *exp  = 0;
        }
    } else if (courant() == 'X') {
        /* xpuissance seule → coef implicite 1 */
        *coef = 1.0;
        *exp  = lireXpuissance();
    } else {
        erreurSyntaxe("monôme attendu (nombre ou 'X')");
    }
}
