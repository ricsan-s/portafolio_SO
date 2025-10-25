#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // fork()
#include <pthread.h>     // hilos
#include <semaphore.h>   // semáforos
#include <sys/wait.h>    // wait()
#include <sys/mman.h>    // mmap() para memoria compartida

// --- Configuración ---
#define BUFFER_SIZE     10
#define NUM_PRODUCERS   2
#define NUM_CONSUMERS   3
#define ITEMS_PER_PRODUCER 5

// Estructura que vivirá en la memoria compartida
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;  // Índice para escribir (productor)
    int out; // Índice para leer (consumidor)

    // Semáforos para la sincronización
    sem_t mutex;        // Para exclusión mutua al acceder al buffer
    sem_t empty_slots;  // Cuenta los espacios vacíos
    sem_t full_slots;   // Cuenta los espacios llenos
} SharedData;

SharedData *shared_data; // Puntero a la memoria compartida

// --- Lógica del Productor ---
void* producer_task(void* arg) {
    int producer_id = (int)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = rand() % 100; // Producir un ítem (número aleatorio)

        sem_wait(&shared_data->empty_slots); // Esperar si el buffer está lleno
        sem_wait(&shared_data->mutex);       // Bloquear el buffer para acceso exclusivo

        // Sección Crítica: Añadir ítem al buffer
        shared_data->buffer[shared_data->in] = item;
        printf("Productor %d -> produce %d en la posición %d\n", producer_id, item, shared_data->in);
        shared_data->in = (shared_data->in + 1) % BUFFER_SIZE;

        sem_post(&shared_data->mutex);     // Liberar el buffer
        sem_post(&shared_data->full_slots);  // Señalar que hay un nuevo ítem disponible

        usleep(100000 + rand() % 200000); // Simular trabajo
    }
    pthread_exit(NULL);
}

// --- Lógica del Consumidor ---
void* consumer_task(void* arg) {
    int consumer_id = (int)arg;
    // Cada consumidor tratará de consumir un número proporcional de ítems
    int items_to_consume = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        sem_wait(&shared_data->full_slots);  // Esperar si el buffer está vacío
        sem_wait(&shared_data->mutex);       // Bloquear el buffer para acceso exclusivo

        // Sección Crítica: Sacar ítem del buffer
        int item = shared_data->buffer[shared_data->out];
        printf("Consumidor %d <- consume %d de la posición %d\n", consumer_id, item, shared_data->out);
        shared_data->out = (shared_data->out + 1) % BUFFER_SIZE;

        sem_post(&shared_data->mutex);     // Liberar el buffer
        sem_post(&shared_data->empty_slots); // Señalar que hay un nuevo espacio vacío

        usleep(200000 + rand() % 300000); // Simular procesamiento
    }
    pthread_exit(NULL);
}


int main() {
    // 1. Crear la región de memoria compartida
    shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        perror("Error en mmap");
        exit(1);
    }

    // 2. Inicializar el buffer y los semáforos en la memoria compartida
    shared_data->in = 0;
    shared_data->out = 0;
    // El segundo argumento '1' es crucial para que los semáforos funcionen entre procesos
    sem_init(&shared_data->mutex, 1, 1);
    sem_init(&shared_data->empty_slots, 1, BUFFER_SIZE);
    sem_init(&shared_data->full_slots, 1, 0);

    // 3. Crear los procesos
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
        exit(1);
    }

    if (pid > 0) {
        // --- PROCESO PADRE (PRODUCTOR) ---
        printf("Proceso Padre (PRODUCTOR) PID=%d\n", getpid());
        pthread_t producers[NUM_PRODUCERS];
        int producer_ids[NUM_PRODUCERS];

        for (int i = 0; i < NUM_PRODUCERS; i++) {
            producer_ids[i] = i + 1;
            pthread_create(&producers[i], NULL, producer_task, &producer_ids[i]);
        }
        for (int i = 0; i < NUM_PRODUCERS; i++) {
            pthread_join(producers[i], NULL);
        }

        wait(NULL); // Esperar a que el proceso hijo (consumidor) termine
        printf("Proceso Padre: El hijo terminó.\n");

    } else {
        // --- PROCESO HIJO (CONSUMIDOR) ---
        printf("Proceso Hijo (CONSUMIDOR) PID=%d\n", getpid());
        pthread_t consumers[NUM_CONSUMERS];
        int consumer_ids[NUM_CONSUMERS];

        for (int i = 0; i < NUM_CONSUMERS; i++) {
            consumer_ids[i] = i + 1;
            pthread_create(&consumers[i], NULL, consumer_task, &consumer_ids[i]);
        }
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            pthread_join(consumers[i], NULL);
        }

        printf("Proceso Hijo terminó.\n");
        exit(0);
    }

    // 4. Limpieza (solo el padre llega aquí)
    sem_destroy(&shared_data->mutex);
    sem_destroy(&shared_data->empty_slots);
    sem_destroy(&shared_data->full_slots);
    munmap(shared_data, sizeof(SharedData));

    return 0;
}