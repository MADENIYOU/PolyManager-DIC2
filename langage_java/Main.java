// Fichier : Main.java
import java.util.Scanner;

/**
 * Programme principal — démo Q1-8 en Java.
 */
public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        /* ---- Q1-5 : analyse, affichage, évaluation ---- */
        System.out.println("=== PolyManager DIC2 — Java Q1-8 ===\n");

        System.out.println("Entrez le polynôme P (ex: -4.5*X^5 + 2*X^2 - 7) :");
        Polynome p = new Polynome();
        p.analyser(sc.nextLine());

        System.out.println("Entrez le polynôme Q :");
        Polynome q = new Polynome();
        q.analyser(sc.nextLine());

        System.out.print("P = "); p.afficher();
        System.out.print("Q = "); q.afficher();

        System.out.print("Valeur de X pour l'évaluation : ");
        double x = sc.nextDouble(); sc.nextLine();
        System.out.printf("P(%g) = %g%n", x, p.evaluer(x));
        System.out.printf("Q(%g) = %g%n", x, q.evaluer(x));

        /* ---- Q6 : opérations arithmétiques ---- */
        System.out.println("\n=== Q6 : Opérations arithmétiques ===");
        System.out.print("P + Q = "); Polynome.plus(p, q).afficher();
        System.out.print("P - Q = "); Polynome.moins(p, q).afficher();
        System.out.print("P * Q = "); Polynome.fois(p, q).afficher();

        Polynome[] reste = new Polynome[1];
        Polynome quot = Polynome.quotient(p, q, reste);
        System.out.print("P / Q = "); quot.afficher();
        System.out.print("reste = "); reste[0].afficher();

        /* ---- Q8 : versions récursives ---- */
        System.out.println("\n=== Q8 : Versions récursives ===");
        System.out.print("P + Q (récursif) = ");
        Polynome.plusRec(p.getTete(), q.getTete()).afficher();
        System.out.print("P - Q (récursif) = ");
        Polynome.moinsRec(p.getTete(), q.getTete()).afficher();
    }
}
