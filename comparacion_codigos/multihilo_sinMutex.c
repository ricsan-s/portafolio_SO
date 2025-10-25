// multihilos_atomico.c
// Compilar: gcc -std=c11 -O2 -pthread multihilos_atomico.c -o multihilos_atomico -lm
// Ejecutar: ./multihilos_atomico 1 200000 8

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <stdatomic.h> // Necesario para C11 Atomics

// --- Definiciones y Variables Globales ---
#define CHUNK_SIZE 1000

// Variables globales para la cola de tareas
static pthread_mutex_t mtx_work = PTHREAD_MUTEX_INITIALIZER;
static long g_current_start;
static long g_end_range;

// Contador global atómico
static _Atomic long total_primes = 0;

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

// --- Función Worker con Balanceo Dinámico y Acumulación Atómica ---
static void* worker(void* arg) {
    int thread_id = *(int*)arg;
    long local_total = 0;
    long start, end;

    while (1) {
        // Pedir un nuevo bloque de trabajo
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

        // Procesar el bloque y acumular localmente
        for (long x = start; x <= end; ++x) {
            local_total += es_primo(x);
        }
    }
    
    // Sumar el total local al global de forma atómica
    if (local_total > 0) {
        atomic_fetch_add(&total_primes, local_total);
    }
    
    // (Opcional) Guardar el resultado para imprimirlo
    long* result = malloc(sizeof(long));
    *result = local_total;
    return result;
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

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    int* thread_ids = malloc(sizeof(int) * num_threads);

    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        void* res;
        pthread_join(threads[i], &res);
        long local_count = *(long*)res;
        printf("Hilo %d -> primos encontrados: %ld\n", i, local_count);
        free(res);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = end_time.tv_sec - start_time.tv_sec;
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    printf("\nTotal de primos en [%ld, %ld] = %ld (con %d hilos)\n", A, B, total_primes, num_threads);
    printf("Tiempo de ejecución: %.4f segundos\n", elapsed_time);

    free(threads);
    free(thread_ids);
    pthread_mutex_destroy(&mtx_work);
    return 0;
}