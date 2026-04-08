// Fichier : Monome.java
public class Monome {
    double coefficient;
    int exposant;
    Monome suivant;

    // Pour la Question 7 (Recyclage spécifique au projet)
    Monome general;
    boolean utile;

    public Monome(double coefficient, int exposant) {
        this.coefficient = coefficient;
        this.exposant = exposant;
        this.suivant = null;
        this.utile = false;
    }
}