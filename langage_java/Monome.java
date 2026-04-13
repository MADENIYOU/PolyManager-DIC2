/**
 * Monome.java
 *
 * Représente un maillon de la liste chaînée d'un polynôme.
 *
 * Champs supplémentaires pour le Garbage Collector (Question 7) :
 *   - general : chaîne tous les maillons jamais alloués
 *   - utile   : marqueur de vie (true = à garder, false = à libérer)
 */
public class Monome {

    /* ---- Champs polynôme ---- */
    double coefficient;
    int    exposant;
    Monome suivant;   /* maillon suivant dans le polynôme (ordre décroissant) */

    /* ---- Champs Garbage Collector (Question 7) ---- */
    Monome  general;  /* maillon suivant dans la liste GLOBALE de tous les maillons */
    boolean utile;    /* true = maillon marqué comme utile lors du GC               */

    /* ---- Constructeur ---- */
    Monome(double coefficient, int exposant) {
        this.coefficient = coefficient;
        this.exposant    = exposant;
        this.suivant     = null;
        this.general     = null;
        this.utile       = false;
    }
}
