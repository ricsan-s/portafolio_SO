#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // fork()
#include <pthread.h>     // hilos
#include <sys/wait.h>    // wait()

#define NUM_THREADS 5
#define NUM_ITERATIONS 10

int contador = 0; // recurso compartido

// Función que ejecutarán los hilos
void* tarea_hilo(void* arg) {
    int id = (int)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // La sección crítica ya no está protegida
        contador++;
        printf("Hilo %d incrementó el contador a %d\n", id, contador);
        usleep(100000); // Pausa para simular trabajo
    }

    pthread_exit(NULL);
}

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("Error en fork");
        exit(1);
    }

    if (pid == 0) { 
        // PROCESO HIJO
        printf("Proceso hijo PID=%d\n", getpid());
        pthread_t hilos[NUM_THREADS];
        int ids[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; i++) {
            ids[i] = i + 1;
            pthread_create(&hilos[i], NULL, tarea_hilo, &ids[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(hilos[i], NULL);
        }

        printf("Proceso hijo terminó. Contador final = %d\n", contador);
        exit(0);
    } else { 
        // PROCESO PADRE
        printf("Proceso padre PID=%d, esperando al hijo...\n", getpid());
        wait(NULL);
        printf("Proceso padre: hijo terminó correctamente.\n");
    }

    return 0;
}