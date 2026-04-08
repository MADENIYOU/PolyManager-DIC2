// Fichier : Main.java
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        Polynome p = new Polynome();

        System.out.println("Entrez un polynôme (ex: -4.5X^5 + 2X^2 - 7):");
        String saisie = sc.nextLine();

        p.analyser(saisie);

        System.out.print("Polynôme reconnu : ");
        p.afficher();

        System.out.print("Entrez la valeur de X pour l'évaluation : ");
        double x = sc.nextDouble();
        System.out.println("Résultat P(" + x + ") = " + p.evaluer(x));
    }
}