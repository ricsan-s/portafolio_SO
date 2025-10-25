// multihilos_dinamico.c
// Compilar: gcc -O2 -pthread multihilos_dinamico.c -o multihilos_dinamico -lm
// Ejecutar: ./multihilos_dinamico 1 200000 8

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

// --- Definiciones y Variables Globales ---
#define CHUNK_SIZE 1000 // Tamaño de cada bloque de trabajo

// Estructura para el resultado de cada hilo
typedef struct {
    long count_local;
    int thread_id;
} ThreadResult;

// Variables globales para la cola de tareas
static pthread_mutex_t mtx_work = PTHREAD_MUTEX_INITIALIZER;
static long g_current_start;
static long g_end_range;

// Variables para la suma total
static pthread_mutex_t mtx_total = PTHREAD_MUTEX_INITIALIZER;
static long total_primes = 0;

// --- Función de Verificación de Primos (sin cambios) ---
static int es_primo(long n) {
    if (n < 2) return 0;
    if (n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    long r = (long) sqrt((double) n);
    for (long i = 5; i <= r; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// --- Función Worker con Balanceo Dinámico ---
static void* worker(void* arg) {
    ThreadResult* result = (ThreadResult*)arg;
    result->count_local = 0;
    long start, end;

    while (1) {
        // Pedir un nuevo bloque de trabajo de forma segura
        pthread_mutex_lock(&mtx_work);
        if (g_current_start > g_end_range) {
            pthread_mutex_unlock(&mtx_work);
            break; // No hay más trabajo
        }
        start = g_current_start;
        end = start + CHUNK_SIZE - 1;
        if (end > g_end_range) {
            end = g_end_range;
        }
        g_current_start += CHUNK_SIZE;
        pthread_mutex_unlock(&mtx_work);

        // Procesar el bloque de trabajo
        long local_chunk_primes = 0;
        for (long x = start; x <= end; ++x) {
            local_chunk_primes += es_primo(x);
        }
        result->count_local += local_chunk_primes;
    }

    // Acumular de forma segura en el total global
    pthread_mutex_lock(&mtx_total);
    total_primes += result->count_local;
    pthread_mutex_unlock(&mtx_total);

    return NULL;
}

// --- Main ---
int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <inicio> <fin> <num_hilos>\n", argv[0]);
        return 1;
    }

    long A = atol(argv[1]);
    long B = atol(argv[2]);
    int num_threads = atoi(argv[3]);

    g_current_start = A;
    g_end_range = B;

    // Medición de tiempo
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    ThreadResult* results = malloc(sizeof(ThreadResult) * num_threads);

    for (int i = 0; i < num_threads; ++i) {
        results[i].thread_id = i;
        results[i].count_local = 0;
        pthread_create(&threads[i], NULL, worker, &results[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
        printf("Hilo %d -> primos encontrados: %ld\n", i, results[i].count_local);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = end_time.tv_sec - start_time.tv_sec;
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    printf("\nTotal de primos en [%ld, %ld] = %ld (con %d hilos)\n", A, B, total_primes, num_threads);
    printf("Tiempo de ejecución: %.4f segundos\n", elapsed_time);

    free(threads);
    free(results);
    pthread_mutex_destroy(&mtx_work);
    pthread_mutex_destroy(&mtx_total);
    return 0;
}