#include <stdio.h>
#include <stdlib.h>
#include "complex.h"
#include "vector.h"

// la struttura vector si trova in vector.h e contiene un array di complessi, la dimensione del vettore e il numero di qubit
//funzione che crea un vettore
vector crea_vector(int n_componenti){
    vector v;
    v.num_componenti=n_componenti;

    v.dimensione=1<<n_componenti;//2^n_componenti
    //alloco lo spazione per le componenti del vettore
    v.componenti=calloc(v.dimensione,sizeof(complesso));
    return v;   

}
