#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "complex.h"
#include "vector.h"
#include "matrix.h"

// Struttura dati passata a ogni thread per la moltiplicazione matrice-vettore
typedef struct {
    int thread_id;
    int num_threads;
    int dimensione;       
    matrix *matrice;       
    vector *vettoreprec;   
    vector *vettoreresult; 
} thread_data;


//funzione worker dei thread per il calcolo delle righe della matrice
void* moltiplica_riga(void* arg) {
    thread_data *data = (thread_data*)arg;
    int thread_id = data->thread_id;
    int num_threads = data->num_threads;
    int N = data->dimensione; 

    // Ripartisco il lavoro tra i thread 
    int rows_per_thread = N / num_threads;
    int remainder = N % num_threads;

    int start_row, end_row;
    if (thread_id < remainder) {
        start_row = thread_id * (rows_per_thread + 1);
        end_row = start_row + (rows_per_thread + 1);
    } else {
        start_row = thread_id * rows_per_thread + remainder;
        end_row = start_row + rows_per_thread;
    }

    // Moltiplicazione Matrice-Vettore
    for (int i = start_row; i < end_row; i++) {
        complesso sum = {0.0, 0.0};
        
        for (int j = 0; j < N; j++) {
            // Accesso tramite il puntatore corretto della matrice passata
            complesso prod = prodotto(data->matrice->elementi[i][j], data->vettoreprec->componenti[j]);
            sum = somma(sum, prod);
        }
        data->vettoreresult->componenti[i] = sum;
    }

    pthread_exit(NULL);
}
// funzione principale per l'applicazione del circuito  e la sincronizzazione dei thread
void applica_circuito(matrix *porte, int k, int n, vector *vinit, vector *vfin, int num_threads) {
    int N = n; 

    // Se non ci sono porte (mai) ho finito e termino
    if (k == 0) {
        for (int i = 0; i < N; i++) vfin->componenti[i] = vinit->componenti[i];
        return;
    }

    // Creo il vettore di stato corrente e lo inizializziamo con i valori di  vinit (ogni volta che applichiamo una porta il risultato diventa il nuovo stato della porta successiva)
    vector stato_corrente;
    stato_corrente.dimensione = N;
    stato_corrente.componenti = (complesso*) malloc(N * sizeof(complesso));
    for (int i = 0; i < N; i++) {
        stato_corrente.componenti[i] = vinit->componenti[i];
    }

    // Creo un secondo vettore di appoggio che conterrà il risultato del singolo step
    vector stato_successivo;
    stato_successivo.dimensione = N;
    stato_successivo.componenti = (complesso*) malloc(N * sizeof(complesso));

    pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    
    // Allocazione dinamica dei dati dei thread per evitare conflitti di memoria
    thread_data *t_data = (thread_data*) malloc(num_threads * sizeof(thread_data));

    // Ciclo principale sulle porte del circuito
    for (int step = 0; step < k; step++) {
        
        // 1. inizializziamo lo stato successivo a 0
        for (int i = 0; i < N; i++) {
            stato_successivo.componenti[i].reale = 0.0;
            stato_successivo.componenti[i].immaginario = 0.0;
        }

        // 2. Lanciamo i thread per calcolare la moltiplicazione delle righe 
        for (int t = 0; t < num_threads; t++) {
            t_data[t].thread_id = t;
            t_data[t].num_threads = num_threads;
            t_data[t].dimensione = N;
            t_data[t].matrice = &porte[step]; // Indirizzo fisso della porta corrente
            t_data[t].vettoreprec = &stato_corrente; // Leggono SEMPRE da stato_corrente
            t_data[t].vettoreresult = &stato_successivo; // Scrivono SEMPRE in stato_successivo

            pthread_create(&threads[t], NULL, moltiplica_riga, (void*)&t_data[t]);
        }

        // 3. BARRIERA : Attendiamo che TUTTI i thread abbiano finito lo step attuale prima di continuare altrimenti potremmo ottenere risultati errati
        for (int t = 0; t < num_threads; t++) {
            pthread_join(threads[t], NULL);
        }

        // 4. aggiorno  lo stato corrente con il risultato appena calcolato.
        // In questo modo il prossimo step leggerà il vettore aggiornato.
        for (int i = 0; i < N; i++) {
            stato_corrente.componenti[i] = stato_successivo.componenti[i];
        }
    }

    // Al termine di tutte le porte, il risultato finale si trova in stato_corrente.
    // Lo copiamo nel vettore vfin atteso dal main
    for (int i = 0; i < N; i++) {
        vfin->componenti[i] = stato_corrente.componenti[i];
    }

    // Liberazione della memoria 
    free(threads);
    free(t_data);
    free(stato_corrente.componenti);
    free(stato_successivo.componenti);
}
