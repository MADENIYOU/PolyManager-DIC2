// ============================================================
//  PolynomeRecursif.java : Versions récursives de plus() et moins() sur les polynômes
//  représentés par des listes chaînées de monômes.
// ============================================================

public class PolynomeRecursif {

    static class Maillon {
        double coefficient;
        int    exposant;
        Maillon suivant;

        Maillon(double coef, int expo) {
            this.coefficient = coef;
            this.exposant    = expo;
            this.suivant     = null;
        }
    }

    static void afficher(Maillon p) {
        if (p == null) { System.out.println("0"); return; }
        StringBuilder sb = new StringBuilder();
        while (p != null) {
            if (p.exposant == 0)
                sb.append(String.format("%.2f", p.coefficient));
            else if (p.exposant == 1)
                sb.append(String.format("%.2f*X", p.coefficient));
            else
                sb.append(String.format("%.2f*X^%d", p.coefficient, p.exposant));
            if (p.suivant != null) sb.append(" + ");
            p = p.suivant;
        }
        System.out.println(sb);
    }


    static Maillon plus(Maillon a, Maillon b) {

        if (a == null) return b;
        if (b == null) return a;

        Maillon result;

        if (a.exposant > b.exposant) {
            result         = new Maillon(a.coefficient, a.exposant);
            result.suivant = plus(a.suivant, b);   
        } else if (b.exposant > a.exposant) {
            result         = new Maillon(b.coefficient, b.exposant);
            result.suivant = plus(a, b.suivant); 

        } else {
            double somme = a.coefficient + b.coefficient;

            if (somme != 0.0) {
                result         = new Maillon(somme, a.exposant);
                result.suivant = plus(a.suivant, b.suivant);  
            } else {
                result = plus(a.suivant, b.suivant);
            }
        }

        return result;
    }


    static Maillon moins(Maillon a, Maillon b) {

        if (b == null) return a; 

        if (a == null) {
            Maillon neg    = new Maillon(-b.coefficient, b.exposant);
            neg.suivant    = moins(null, b.suivant);   
            return neg;
        }

        Maillon result;

        if (a.exposant > b.exposant) {
            result         = new Maillon(a.coefficient, a.exposant);
            result.suivant = moins(a.suivant, b);    
        } else if (b.exposant > a.exposant) {
            result         = new Maillon(-b.coefficient, b.exposant);
            result.suivant = moins(a, b.suivant);   

        } else {
            double diff = a.coefficient - b.coefficient;

            if (diff != 0.0) {
                result         = new Maillon(diff, a.exposant);
                result.suivant = moins(a.suivant, b.suivant);
            } else {
                result = moins(a.suivant, b.suivant);   
            }
        }

        return result;
    }


    public static void main(String[] args) {

        Maillon P = new Maillon(3.0, 3);
        P.suivant = new Maillon(2.0, 2);
        P.suivant.suivant = new Maillon(5.0, 0);

        Maillon Q = new Maillon(4.0, 3);
        Q.suivant = new Maillon(-2.0, 2);
        Q.suivant.suivant = new Maillon(7.0, 1);

        System.out.print("P     = "); afficher(P);
        System.out.print("Q     = "); afficher(Q);

        Maillon S = plus(P, Q);
        System.out.print("P + Q = "); afficher(S);  

        Maillon D = moins(P, Q);
        System.out.print("P - Q = "); afficher(D);  
    }
}
