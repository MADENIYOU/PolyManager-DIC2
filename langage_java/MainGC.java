/**
 * MainGC.java
 *
 * Démonstration du Garbage Collector (Question 7).
 *
 * On construit manuellement deux polynômes P et Q via GarbageCollector.allouer(),
 * puis un polynôme temporaire non enregistré, et on appelle recycler().
 */
public class MainGC {

    /* Petit utilitaire : afficher un polynôme */
    static void afficher(Monome tete) {
        if (tete == null) { System.out.println("0"); return; }
        boolean premier = true;
        Monome p = tete;
        while (p != null) {
            if (!premier) {
                System.out.print(p.coefficient >= 0 ? " + " : " - ");
            } else {
                if (p.coefficient < 0) System.out.print("-");
            }
            double c = Math.abs(p.coefficient);
            if (c != 1.0 || p.exposant == 0) System.out.printf("%g", c);
            if (p.exposant > 0) {
                System.out.print("X");
                if (p.exposant > 1) System.out.print("^" + p.exposant);
            }
            premier = false;
            p = p.suivant;
        }
        System.out.println();
    }

    /* Insérer un monôme en maintenant l'ordre décroissant des degrés */
    static Monome inserer(Monome tete, double coef, int exp) {
        if (coef == 0.0) return tete;
        Monome nouveau = GarbageCollector.allouer(coef, exp);
        if (tete == null || exp > tete.exposant) {
            nouveau.suivant = tete;
            return nouveau;
        }
        Monome prec = tete;
        while (prec.suivant != null && prec.suivant.exposant > exp)
            prec = prec.suivant;
        nouveau.suivant = prec.suivant;
        prec.suivant = nouveau;
        return tete;
    }

    public static void main(String[] args) {
        System.out.println("=== PolyManager DIC2 — Q7 Garbage Collector (Java) ===\n");

        /* --- Construction de P = X^3 + 2X + 1 --- */
        Monome p = null;
        p = inserer(p, 1.0, 3);
        p = inserer(p, 2.0, 1);
        p = inserer(p, 1.0, 0);

        /* --- Construction de Q = X + 1 --- */
        Monome q = null;
        q = inserer(q, 1.0, 1);
        q = inserer(q, 1.0, 0);

        System.out.print("P = "); afficher(p);
        System.out.print("Q = "); afficher(q);

        /* --- Polynôme temporaire non enregistré (simule un résultat intermédiaire) --- */
        Monome tmp = null;
        tmp = inserer(tmp, 3.0, 2);
        tmp = inserer(tmp, 1.0, 0);
        System.out.print("\n[Q7] Polynôme temporaire (non enregistré) = ");
        afficher(tmp);
        /* tmp n'est PAS enregistré → ses maillons seront libérés par le GC */

        /* --- Enregistrement des polynômes à conserver --- */
        System.out.println("[Q7] Enregistrement de P et Q comme utiles...");
        GarbageCollector.enregistrerPoly(p);
        GarbageCollector.enregistrerPoly(q);

        /* --- Lancement du GC --- */
        System.out.println("[Q7] Lancement du GC...");
        GarbageCollector.recycler();

        /* --- Vérification : P et Q intacts --- */
        System.out.println("\nAprès GC — P et Q intacts :");
        System.out.print("P = "); afficher(p);
        System.out.print("Q = "); afficher(q);
    }
}
