/**
 * GarbageCollector.java
 *
 * Question 7 — Mécanisme de recyclage mémoire.
 *
 * Principe identique à la version C :
 *
 *   - Chaque maillon (Monome) alloué est ajouté à la liste
 *     globale tousLesMaillons (via le champ Monome.general).
 *
 *   - Le tableau polyUtile[] référence les polynômes dont les
 *     maillons ne doivent PAS être libérés.
 *
 *   - recycler() effectue deux passes :
 *       1. MARQUAGE  : parcourir polyUtile[], mettre utile = true
 *          sur chaque maillon rencontré.
 *       2. BALAYAGE  : parcourir tousLesMaillons, retirer de la
 *          liste et "libérer" (annuler les références) les maillons
 *          non marqués ; effacer les marques des maillons conservés.
 *
 * Note : En Java, le ramasse-miettes natif de la JVM récupère la
 * mémoire réelle dès qu'il n'y a plus de référence vers un objet.
 * Ce GC applicatif reproduit le mécanisme décrit dans l'énoncé
 * (mark-and-sweep) à titre pédagogique.
 */
public class GarbageCollector {

    /* ---- Liste de TOUS les maillons jamais alloués ---- */
    static Monome tousLesMaillons = null;

    /* ---- Tableau des polynômes utiles (têtes de listes) ---- */
    static final int MAX_POLY = 100;
    static Monome[]  polyUtile  = new Monome[MAX_POLY];
    static int       nbPolyUtile = 0;

    /* ------------------------------------------------------------------ */

    /**
     * Allouer un maillon et l'enregistrer dans la liste générale.
     * À utiliser à la place de « new Monome(...) » partout dans le code.
     */
    public static Monome allouer(double coef, int exp) {
        Monome m = new Monome(coef, exp);
        /* chaîner en tête de la liste générale */
        m.general = tousLesMaillons;
        tousLesMaillons = m;
        return m;
    }

    /* ------------------------------------------------------------------ */

    /**
     * Déclarer un polynôme comme utile pour qu'il soit protégé par le GC.
     * Doit être appelé pour chaque polynôme que l'on souhaite conserver.
     */
    public static void enregistrerPoly(Monome tete) {
        if (nbPolyUtile >= MAX_POLY) {
            System.err.println("Avertissement : polyUtile plein, polynôme non enregistré.");
            return;
        }
        polyUtile[nbPolyUtile++] = tete;
    }

    /* ------------------------------------------------------------------ */

    /**
     * recycler() — Garbage Collector en deux passes.
     *
     * Passe 1 — MARQUAGE
     *   Pour chaque polynôme enregistré dans polyUtile[],
     *   parcourir sa liste chaînée et mettre utile = true.
     *
     * Passe 2 — BALAYAGE
     *   Parcourir tousLesMaillons (via Monome.general) :
     *     • utile == true  → effacer la marque, conserver le maillon.
     *     • utile == false → retirer de la liste générale et annuler
     *                        les références (le GC JVM récupère ensuite
     *                        la mémoire réelle).
     */
    public static void recycler() {
        /* ---- Passe 1 : marquage ---- */
        for (int i = 0; i < nbPolyUtile; i++) {
            Monome p = polyUtile[i];
            while (p != null) {
                p.utile = true;
                p = p.suivant;
            }
        }

        /* ---- Passe 2 : balayage ---- */
        /*
         * On manipule la liste via un tableau d'une case pour simuler
         * le « pointeur sur pointeur » du C (POINTEUR *courant).
         * holder[0] joue le rôle de *courant.
         */
        // Utilisation d'un nœud sentinelle pour simplifier le chaînage
        Monome sentinelle = new Monome(0, 0);
        sentinelle.general = tousLesMaillons;

        Monome pred = sentinelle;
        Monome courant = tousLesMaillons;
        int liberes = 0;

        while (courant != null) {
            Monome suivantGeneral = courant.general;
            if (courant.utile) {
                /* Maillon utile : effacer la marque, garder dans la liste */
                courant.utile = false;
                pred = courant;
            } else {
                /* Maillon inutile : retirer de la liste générale */
                pred.general = suivantGeneral;
                /* Annuler les références pour aider le GC JVM */
                courant.suivant = null;
                courant.general = null;
                liberes++;
            }
            courant = suivantGeneral;
        }

        tousLesMaillons = sentinelle.general;

        System.out.println("[GC] " + liberes + " maillon(s) libéré(s).");
    }
}
