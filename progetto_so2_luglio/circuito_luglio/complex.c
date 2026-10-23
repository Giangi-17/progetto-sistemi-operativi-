#include <stdio.h>
#include <math.h>
#include "complex.h"
// la struttura complesso si trova in complex.h e contiene due double per la parte reale e immaginaria


// tre funzioni di base per la gestione dei numeri complessi
complesso somma(complesso c1,complesso c2){
    complesso c3;
    c3.reale=c1.reale+c2.reale;
    c3.immaginario=c1.immaginario+c2.immaginario;
    return c3;
}

complesso prodotto(complesso c1, complesso c2){
    complesso c3;
    c3.reale= c1.reale*c2.reale-c1.immaginario*c2.immaginario;
    c3.immaginario=c1.reale*c2.immaginario+c1.immaginario*c2.reale;
    return c3;
}
complesso modulo(complesso c){
    complesso c1;
    c1.reale=sqrt(c.reale*c.reale+c.immaginario*c.immaginario);
    c1.immaginario=0;
    return c1;

}

