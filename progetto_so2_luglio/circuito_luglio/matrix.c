#include <stdio.h>
#include <stdlib.h>
#include "complex.h"
#include "vector.h"
#include "matrix.h"


//la struttura matrix si trova in matrix.h e contiene un array bidimensionale di complessi, la dimensione della matrice e il numero di qubit

//funzione che crea una matrice di dimensione 2^n_componenti e alloca la memoria per gli elementi
matrix crea_matrix(int n_componenti){
    matrix m;
    m.num=n_componenti;
    m.dimensione=1<<n_componenti;//2^n_componenti
    m.elementi=calloc(m.dimensione,sizeof(complesso*));

    for(int i=0;i<m.dimensione;i++){
        m.elementi[i]=calloc(m.dimensione,sizeof(complesso));
    }
    return m;   
}

complesso get_elemento(matrix m, int i , int j){
    return m.elementi[i][j];
}


matrix prodotto_scalare(matrix m, complesso c){
    matrix m2;
    m2.dimensione=m.dimensione;
    m2.elementi=calloc(m2.dimensione,sizeof(complesso*));

    for (int i=0; i<m.dimensione; i++){
        for (int j=0; j<m.dimensione; j++){
            m2.elementi[i][j]=prodotto(m.elementi[i][j],c);
        }
    }
    return m2;
}   



