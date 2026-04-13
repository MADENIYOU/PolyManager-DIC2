import java.util.ArrayList;
import java.util.List;

/**
 * Classe gérant la logique des polynômes (Questions 1 à 5).
 */
public class Polynome {
    private Monome tete;

    // --- Question 7 : "En sous-sol" ---
    // Liste globale de tous les maillons créés pour simulation du Garbage Collector
    public static List<Monome> tousLesMaillons = new ArrayList<>();

    public Polynome() {
        this.tete = null;
    }

    /**
     * Question 2 : Crée un maillon et l'enregistre dans la liste globale.
     */
    private Monome creerMonome(double coef, int exp) {
        Monome m = new Monome(coef, exp);
        tousLesMaillons.add(m); // Ajout automatique au registre global
        return m;
    }

    /**
     * Question 4 : Insertion triée par degré décroissant.
     * Correction appliquée : Fusionne les coefficients si le degré existe déjà.
     */
    public void insererTrie(double coef, int exp) {
        if (coef == 0) return;

        // 1. Correction : Vérifier si le degré existe déjà pour additionner
        Monome courant = tete;
        while (courant != null) {
            if (courant.exposant == exp) {
                courant.coefficient += coef;
                return;
            }
            courant = courant.suivant;
        }

        // 2. Sinon, créer un nouveau maillon et l'insérer au bon endroit
        Monome nouveau = creerMonome(coef, exp);
        if (tete == null || exp > tete.exposant) {
            nouveau.suivant = tete;
            tete = nouveau;
        } else {
            Monome prec = tete;
            while (prec.suivant != null && prec.suivant.exposant > exp) {
                prec = prec.suivant;
            }
            nouveau.suivant = prec.suivant;
            prec.suivant = nouveau;
        }
    }

    /**
     * Question 1 & 2 : Analyseur Syntaxique.
     * Parcourt la chaîne et extrait les monômes selon la grammaire.
     */
    public void analyser(String chaine) {
        if (chaine == null || chaine.trim().isEmpty()) return;

        // Nettoyage de la chaîne
        String s = chaine.replaceAll("\\s+", ""); 
        int i = 0;

        while (i < s.length()) {
            int signe = 1;

            // Gestion du signe du monôme
            if (s.charAt(i) == '+') {
                i++;
            } else if (s.charAt(i) == '-') {
                signe = -1;
                i++;
            }

            // Extraction du coefficient
            double coef = 1.0;
            boolean coefLu = false;
            int debut = i;
            while (i < s.length() && (Character.isDigit(s.charAt(i)) || s.charAt(i) == '.')) {
                i++;
                coefLu = true;
            }
            if (coefLu) {
                coef = Double.parseDouble(s.substring(debut, i));
            }

            // Gestion du '*' optionnel (ex: 4.5 * X^2)
            if (i < s.length() && s.charAt(i) == '*') {
                i++;
            }

            // Extraction de l'exposant
            int expo = 0;
            if (i < s.length() && (s.charAt(i) == 'X' || s.charAt(i) == 'x')) {
                i++;
                if (i < s.length() && s.charAt(i) == '^') {
                    i++;
                    int debutExp = i;
                    while (i < s.length() && Character.isDigit(s.charAt(i))) {
                        i++;
                    }
                    expo = Integer.parseInt(s.substring(debutExp, i));
                } else {
                    expo = 1; // X seul
                }
            } else {
                expo = 0; // Nombre seul
            }

            // Insertion dans la liste chaînée
            this.insererTrie(coef * signe, expo);
        }
    }

    /**
     * Question 3 : Affichage du polynôme.
     */
    public void afficher() {
        if (tete == null) {
            System.out.println("0");
            return;
        }

        Monome p = tete;
        boolean premier = true;
        while (p != null) {
            double c = p.coefficient;
            if (c == 0) {
                p = p.suivant;
                continue;
            }

            // Signe
            if (!premier && c > 0) System.out.print(" + ");
            else if (c < 0) System.out.print(" - ");
            else if (premier && c < 0) System.out.print("-");

            // Valeur absolue du coefficient
            double absC = Math.abs(c);
            if (absC != 1 || p.exposant == 0) {
                // Affiche l'entier si pas de décimales, sinon le double
                if (absC == (long) absC) System.out.print((long) absC);
                else System.out.print(absC);
            }

            // Partie X
            if (p.exposant > 0) {
                System.out.print("X");
                if (p.exposant > 1) System.out.print("^" + p.exposant);
            }

            premier = false;
            p = p.suivant;
        }
        System.out.println();
    }

    /**
     * Question 5 : Évaluation du polynôme pour une valeur x.
     */
    public double evaluer(double x) {
        double resultat = 0;
        Monome p = tete;
        while (p != null) {
            resultat += p.coefficient * Math.pow(x, p.exposant);
            p = p.suivant;
        }
        return resultat;
    }

    // Getter pour la tête (utile pour les opérations futures)
    public Monome getTete() {
        return tete;
    }
}