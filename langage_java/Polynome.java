public class Polynome {
    private static final double EPS = 1e-12;

    private static final class Node {
        final double coefficient;
        final int exposant;
        Node suivant;

        Node(double coefficient, int exposant) {
            this.coefficient = coefficient;
            this.exposant = exposant;
        }
    }

    public static final class DivisionResult {
        public final Node quotient;
        public final Node reste;

        DivisionResult(Node quotient, Node reste) {
            this.quotient = quotient;
            this.reste = reste;
        }
    }

    private static boolean estNul(double x) {
        return Math.abs(x) < EPS;
    }

    private static Node copier(Node p) {
        Node tete = null;
        Node queue = null;
        while (p != null) {
            Node n = new Node(p.coefficient, p.exposant);
            if (tete == null) {
                tete = n;
            } else {
                queue.suivant = n;
            }
            queue = n;
            p = p.suivant;
        }
        return tete;
    }

    private static Node append(Node tete, Node[] queueRef, double coefficient, int exposant) {
        if (estNul(coefficient)) return tete;
        Node n = new Node(coefficient, exposant);
        if (tete == null) {
            tete = n;
        } else {
            queueRef[0].suivant = n;
        }
        queueRef[0] = n;
        return tete;
    }

    public static Node plus(Node a, Node b) {
        Node tete = null;
        Node[] queue = new Node[1];
        while (a != null && b != null) {
            if (a.exposant == b.exposant) {
                tete = append(tete, queue, a.coefficient + b.coefficient, a.exposant);
                a = a.suivant;
                b = b.suivant;
            } else if (a.exposant > b.exposant) {
                tete = append(tete, queue, a.coefficient, a.exposant);
                a = a.suivant;
            } else {
                tete = append(tete, queue, b.coefficient, b.exposant);
                b = b.suivant;
            }
        }
        while (a != null) {
            tete = append(tete, queue, a.coefficient, a.exposant);
            a = a.suivant;
        }
        while (b != null) {
            tete = append(tete, queue, b.coefficient, b.exposant);
            b = b.suivant;
        }
        return tete;
    }

    private static Node oppose(Node p) {
        Node tete = null;
        Node[] queue = new Node[1];
        while (p != null) {
            tete = append(tete, queue, -p.coefficient, p.exposant);
            p = p.suivant;
        }
        return tete;
    }

    public static Node moins(Node a, Node b) {
        return plus(a, oppose(b));
    }

    private static Node multParMonome(Node p, double coefficient, int exposant) {
        Node tete = null;
        Node[] queue = new Node[1];
        while (p != null) {
            tete = append(tete, queue, p.coefficient * coefficient, p.exposant + exposant);
            p = p.suivant;
        }
        return tete;
    }

    public static Node fois(Node a, Node b) {
        Node resultat = null;
        while (a != null) {
            Node terme = multParMonome(b, a.coefficient, a.exposant);
            resultat = plus(resultat, terme);
            a = a.suivant;
        }
        return resultat;
    }

    public static DivisionResult quotient(Node a, Node b) {
        if (b == null) throw new IllegalArgumentException("Division par le polynome nul");

        Node q = null;
        Node r = copier(a);

        while (r != null && r.exposant >= b.exposant) {
            double c = r.coefficient / b.coefficient;
            int e = r.exposant - b.exposant;
            Node t = new Node(c, e);
            q = plus(q, t);
            Node produit = multParMonome(b, c, e);
            r = moins(r, produit);
        }
        return new DivisionResult(q, r);
    }

    private static Node depuisTableaux(double[] coeffs, int[] exposants) {
        Node tete = null;
        Node[] queue = new Node[1];
        for (int i = 0; i < coeffs.length; i++) {
            tete = append(tete, queue, coeffs[i], exposants[i]);
        }
        return tete;
    }

    private static String format(Node p) {
        if (p == null) return "0";
        StringBuilder sb = new StringBuilder();
        boolean premier = true;
        while (p != null) {
            double c = p.coefficient;
            int e = p.exposant;
            if (!premier) {
                sb.append(c < 0 ? " - " : " + ");
            } else if (c < 0) {
                sb.append("-");
            }

            double abs = Math.abs(c);
            if (e == 0) {
                sb.append(String.format("%.2f", abs));
            } else if (e == 1) {
                if (!estNul(abs - 1.0)) sb.append(String.format("%.2f*", abs));
                sb.append("X");
            } else {
                if (!estNul(abs - 1.0)) sb.append(String.format("%.2f*", abs));
                sb.append("X^").append(e);
            }

            premier = false;
            p = p.suivant;
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        Node a = depuisTableaux(new double[] {4, -2, 1}, new int[] {3, 1, 0});
        Node b = depuisTableaux(new double[] {1, 5, -3}, new int[] {2, 1, 0});

        Node s = plus(a, b);
        Node d = moins(a, b);
        Node m = fois(a, b);
        DivisionResult div = quotient(m, a);

        System.out.println("A = " + format(a));
        System.out.println("B = " + format(b));
        System.out.println("plus(A,B) = " + format(s));
        System.out.println("moins(A,B) = " + format(d));
        System.out.println("fois(A,B) = " + format(m));
        System.out.println("quotient(fois(A,B),A) = " + format(div.quotient));
        System.out.println("reste = " + format(div.reste));
    }
}
