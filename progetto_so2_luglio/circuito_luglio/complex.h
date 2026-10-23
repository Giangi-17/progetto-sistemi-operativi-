#ifndef COMPLEX_H
#define COMPLEX_H
#include <math.h>


typedef struct{
    double reale;
    double immaginario;
    int ha_immaginario;
}complesso;

complesso somma(complesso c1,complesso c2);
complesso prodotto(complesso c1, complesso c2);
complesso modulo(complesso c);
#endif