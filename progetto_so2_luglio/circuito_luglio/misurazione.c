#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "complex.h"
#include "vector.h"

// Struttura dati passata a ogni thread di misurazione
typedef struct {
    int thread_id;
    int num_threads;
    int dimensione;          // N = 2^n_qubit, numero di possibili esiti
    int ripetizioni_totali;  // numero totale di misurazioni richieste (es. 100) in input
    double *cumulativa;      // distribuzione cumulativa delle probabilita', condivisa in sola lettura
    long *conteggi_locali;   // array di N long, SOLO di questo thread (nessuna condivisione in scrittura)
    unsigned int seed;       // seed indipendente per rand_r
} thread_data_misura;

// Trova il primo indice i tale che cumulativa[i] >= r (ricerca binaria)
static int trova_indice(double *cumulativa, int N, double r) {
    int low = 0, high = N - 1;
    while (low < high) {
        int mid = (low + high) / 2;
        if (cumulativa[mid] >= r) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return low;
}
//funzione worker thread per eseguire le misurazioni in parallelo
void* esegui_misurazioni_thread(void *arg) {
    thread_data_misura *data = (thread_data_misura*) arg;
    int N = data->dimensione;

    // Stessa logica di bilanciamento del carico usata per le righe della matrice
    int rip_per_thread = data->ripetizioni_totali / data->num_threads;
    int remainder = data->ripetizioni_totali % data->num_threads;

    int mie_ripetizioni = rip_per_thread + (data->thread_id < remainder ? 1 : 0);

    // Azzeriamo i conteggi locali di questo thread
    for (int i = 0; i < N; i++) {
        data->conteggi_locali[i] = 0;
    }

    for (int r = 0; r < mie_ripetizioni; r++) {
        // rand_r e' thread-safe perche' ogni thread ha il proprio seed
        double campione = (double) rand_r(&data->seed) / ((double) RAND_MAX + 1.0);
        int esito = trova_indice(data->cumulativa, N, campione);
        data->conteggi_locali[esito]++;
    }

    pthread_exit(NULL);
}

// Funzione principale: esegue 'ripetizioni' misurazioni dello stato vfinal,
// usando num_threads thread, e scrive i risultati aggregati in 'conteggi_finali'
// (array di lunghezza vfinal->dimensione, allocato dal chiamante).
void misura_stato(vector *vfinal, int ripetizioni, int num_threads, long *conteggi_finali) {
    int N = vfinal->dimensione;

    // 1. Calcoliamo le probabilita' |ampiezza|^2 e la somma totale (per normalizzare)
    double *prob = (double*) malloc(N * sizeof(double));
    double somma = 0.0;
    for (int i = 0; i < N; i++) {
        double re = vfinal->componenti[i].reale;
        double im = vfinal->componenti[i].immaginario;
        prob[i] = re * re + im * im;
        somma += prob[i];
    }

    // Normalizzazione di sicurezza, in caso di piccoli errori di arrotondamento
    if (somma > 0.0) {
        for (int i = 0; i < N; i++) {
            prob[i] /= somma;
        }
    }
    // calcolo della distribuzione cumulativa per la ricerca binaria
    double *cumulativa = (double*) malloc(N * sizeof(double));
    double acc = 0.0;
    for (int i = 0; i < N; i++) {
        acc += prob[i];
        cumulativa[i] = acc;
    }
    // Forzp l'ultimo valore a 1.0 esatto per evitare problemi di arrotondamento
    // che farebbero "uscire" un campione casuale dall'intervallo coperto
    cumulativa[N - 1] = 1.0;

    // Allocazione strutture per i thread
    pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    thread_data_misura *t_data = (thread_data_misura*) malloc(num_threads * sizeof(thread_data_misura));

    // Ogni thread ha il proprio array di conteggi locali, per non avere
    // nessuna scrittura condivisa durante l'esecuzione parallela
    long **conteggi_per_thread = (long**) malloc(num_threads * sizeof(long*));
    for (int t = 0; t < num_threads; t++) {
        conteggi_per_thread[t] = (long*) malloc(N * sizeof(long));
    }

    //Lancio dei thread
    for (int t = 0; t < num_threads; t++) {
        t_data[t].thread_id = t;
        t_data[t].num_threads = num_threads;
        t_data[t].dimensione = N;
        t_data[t].ripetizioni_totali = ripetizioni;
        t_data[t].cumulativa = cumulativa;
        t_data[t].conteggi_locali = conteggi_per_thread[t];
        // Seed indipendente per ogni thread, basato su tempo + id, per evitare
        // che thread diversi producano la stessa sequenza casuale
        t_data[t].seed = (unsigned int) time(NULL) ^ (unsigned int) (t * 2654435761u);

        pthread_create(&threads[t], NULL, esegui_misurazioni_thread, (void*) &t_data[t]);
    }

    // 5. Attesa di tutti i thread
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    // 6. Riduzione: sommo  i conteggi locali di ogni thread nel risultato finale
    for (int i = 0; i < N; i++) {
        conteggi_finali[i] = 0;
    }
    for (int t = 0; t < num_threads; t++) {
        for (int i = 0; i < N; i++) {
            conteggi_finali[i] += conteggi_per_thread[t][i];
        }
    }

    // 7. Pulizia memoria
    for (int t = 0; t < num_threads; t++) {
        free(conteggi_per_thread[t]);
    }
    free(conteggi_per_thread);
    free(t_data);
    free(threads);
    free(cumulativa);
    free(prob);
}