#include "polynome.h"

int main() {
    char buffer[256];
    POINTEUR monPoly = NULL;

    printf("Entrez un polynome : ");
    fgets(buffer, sizeof(buffer), stdin);

    analyserPolynome(buffer, &monPoly);

    printf("Polynome stocke : ");
    afficherPolynome(monPoly);

    double x = 2.0;
    printf("Valeur pour x = %g : %g\n", x, eval(monPoly, x));

    return 0;
}