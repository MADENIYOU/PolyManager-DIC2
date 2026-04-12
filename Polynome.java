class Monome {
    double coeff;
    int exp;
    Monome next;

    public Monome(double coeff, int exp) {
        this.coeff = coeff;
        this.exp = exp;
    }
}

class Polynome {
    Monome head;

    // --- MULTIPLICATION ---
    public static Polynome fois(Polynome p1, Polynome p2) {
        Polynome res = new Polynome();
        if (p1.head == null || p2.head == null) return res;

        for (Monome m1 = p1.head; m1 != null; m1 = m1.next) {
            for (Monome m2 = p2.head; m2 != null; m2 = m2.next) {
                double c = m1.coeff * m2.coeff;
                int e = m1.exp + m2.exp;
                res = plus(res, new Polynome(new Monome(c, e)));
            }
        }
        return res;
    }

    // --- DIVISION EUCLIDIENNE ---
    public static Polynome[] quotient(Polynome a, Polynome b) {
        if (b.head == null) throw new ArithmeticException("Division par zéro");

        Polynome q = new Polynome();
        Polynome r = a.copier(); // On travaille sur une copie du dividende

        // Tant que le degré du reste est >= degré du diviseur
        while (r.head != null && r.head.exp >= b.head.exp) {
            double c = r.head.coeff / b.head.coeff;
            int e = r.head.exp - b.head.exp;

            Polynome monomeQuotient = new Polynome(new Monome(c, e));
            q = plus(q, monomeQuotient);

            // Reste = Reste - (monomeQuotient * b)
            Polynome soustraire = fois(monomeQuotient, b);
            r = moins(r, soustraire); 
        }

        return new Polynome[] { q, r }; // Retourne [Quotient, Reste]
    }
}
