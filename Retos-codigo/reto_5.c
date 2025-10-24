// RETO 5 - 4CV4
//Hernandez Bernal Angel Said, Sanchez Aguilar Ricardo, Lancón Barragán Erick Aarón
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Para sleep()

// --- Constantes de configuración
#define NUM_LECTORES 5
#define NUM_ESCRITORES 2
#define MAX_ITERACIONES 5

// --- Variable compartida y el lock de lectura/escritura
int dato_compartido = 0;
pthread_rwlock_t rwlock; // El cerrojo de lectura/escritura

// --- Función para los hilos Lectores
void *lector(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < MAX_ITERACIONES; i++) {
        // 1. Bloquear para lectura
        printf("Lector %d: Intentando obtener bloqueo de lectura...\n", id);
        if (pthread_rwlock_rdlock(&rwlock) != 0) {
            perror("Error al obtener bloqueo de lectura");
            exit(EXIT_FAILURE);
        }
        
        // --- Sección Crítica (Lectura)
        printf("Lector %d: Leyendo dato: %d\n", id, dato_compartido);
        // Simular tiempo de lectura
        usleep((rand() % 500 + 100) * 1000); // 100ms a 600ms
        // --- Fin Sección Crítica
        
        // 2. Desbloquear
        if (pthread_rwlock_unlock(&rwlock) != 0) {
            perror("Error al liberar bloqueo de lectura");
            exit(EXIT_FAILURE);
        }
        printf("Lector %d: Terminó de leer.\n", id);

        // Simular trabajo fuera de la sección crítica
        usleep((rand() % 1000 + 500) * 1000); // 500ms a 1500ms
    }
    return NULL;
}

// --- Función para los hilos Escritores
void *escritor(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < MAX_ITERACIONES; i++) {
        // 1. Bloquear para escritura
        printf("Escritor %d: Intentando obtener bloqueo de escritura...\n", id);
        if (pthread_rwlock_wrlock(&rwlock) != 0) {
            perror("Error al obtener bloqueo de escritura");
            exit(EXIT_FAILURE);
        }
        
        // --- Sección Crítica (Escritura)
        dato_compartido++;
        printf("Escritor %d: Escribiendo nuevo dato: %d\n", id, dato_compartido);
        // Simular tiempo de escritura
        usleep((rand() % 800 + 200) * 1000); // 200ms a 1000ms
        // --- Fin Sección Crítica
        
        // 2. Desbloquear
        if (pthread_rwlock_unlock(&rwlock) != 0) {
            perror("Error al liberar bloqueo de escritura");
            exit(EXIT_FAILURE);
        }
        printf("Escritor %d: Terminó de escribir.\n", id);

        // Simular trabajo fuera de la sección crítica
        usleep((rand() % 1500 + 800) * 1000); // 800ms a 2300ms
    }
    return NULL;
}

// --- Función Principal
int main() {
    pthread_t lectores_t[NUM_LECTORES];
    pthread_t escritores_t[NUM_ESCRITORES];
    int lector_ids[NUM_LECTORES];
    int escritor_ids[NUM_ESCRITORES];

    // 1. Inicializar el cerrojo de lectura/escritura
    if (pthread_rwlock_init(&rwlock, NULL) != 0) {
        perror("Error al inicializar el rwlock");
        return EXIT_FAILURE;
    }
    
    srand(time(NULL)); // Inicializar la semilla para números aleatorios

    // 2. Crear hilos Lectores
    for (int i = 0; i < NUM_LECTORES; i++) {
        lector_ids[i] = i + 1;
        if (pthread_create(&lectores_t[i], NULL, lector, &lector_ids[i]) != 0) {
            perror("Error al crear hilo lector");
            return EXIT_FAILURE;
        }
    }

    // 3. Crear hilos Escritores
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        escritor_ids[i] = i + 1;
        if (pthread_create(&escritores_t[i], NULL, escritor, &escritor_ids[i]) != 0) {
            perror("Error al crear hilo escritor");
            return EXIT_FAILURE;
        }
    }

    // 4. Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_LECTORES; i++) {
        pthread_join(lectores_t[i], NULL);
    }
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        pthread_join(escritores_t[i], NULL);
    }

    // 5. Destruir el cerrojo de lectura/escritura
    if (pthread_rwlock_destroy(&rwlock) != 0) {
        perror("Error al destruir el rwlock");
        return EXIT_FAILURE;
    }

    printf("\n--- Simulación terminada. Dato final: %d ---\n", dato_compartido);
    return EXIT_SUCCESS;
}