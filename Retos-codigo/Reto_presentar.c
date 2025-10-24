/*
Reto de Programación: Simulación de Estacionamiento FCFS
Autor: Equipo A - ESCOM IPN (4CV4)
Objetivo: Explicar la lógica de hilos, semáforos y FCFS
*/
//          Simulación de Estacionamiento con Algoritmo FCFS
//          (Primero en Llegar, Primero en Ser Servido)
// Descripción:
// Este programa simula el comportamiento de un estacionamiento con capacidad limitada.
// Cada coche es un hilo (thread) que compite por un lugar. El orden de entrada
// se gestiona con un sistema de tickets para garantizar la justicia (FCFS).

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// -------- Parámetros de la Simulación --------
#define NUM_PLAZAS 5      // Capacidad total del estacionamiento
#define MAX_COCHES 15     // Número total de coches que llegarán
#define TIEMPO_MAX_ESTACIONADO 5  // Tiempo máximo en segundos que un coche se queda
#define LLEGADA_MAX_MS 500        // Tiempo máximo de espera en milisegundos entre la llegada de un coche y el siguiente

// Herramienta para medir el tiempo 
// Devuelve el tiempo actual en segundos.
static inline double tiempo_actual_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Variables Globales y Mecanismos de Sincronización 
// herramientas que todos los hilos (coches) compartirán.
// Control de Plazas 
sem_t semaforo_plazas; // Semáforo que cuenta las plazas libres. Si llega a 0, los coches deben esperar.

// Sistema de Turnos FCFS 
// Usamos un "Ticket Lock": cada coche toma un número y espera su turno.
pthread_mutex_t mutex_turno = PTHREAD_MUTEX_INITIALIZER;  // Candado para proteger el acceso a los tickets.
pthread_cond_t condicion_turno = PTHREAD_COND_INITIALIZER; // Alarma para despertar a los coches cuando el turno avanza.
unsigned long siguiente_ticket = 0; // El número del próximo ticket a repartir.
unsigned long ticket_actual = 0;    // El número del ticket que tiene permiso para entrar.

// Estadísticas 
pthread_mutex_t mutex_estadisticas = PTHREAD_MUTEX_INITIALIZER; // Candado para proteger los datos de las estadísticas.
int coches_dentro = 0;              // Contador de coches actualmente en el estacionamiento.
int max_longitud_fila = 0;          // Para registrar el tamaño máximo que alcanzó la fila de espera.
int coches_atendidos = 0;           // Contador total de coches que lograron estacionarse.
double suma_total_espera_s = 0.0;     // Acumulador para calcular el tiempo de espera promedio.

// Estructura para pasar información a cada hilo/coche 
typedef struct {
    int id; // El identificador único de cada coche.
} InfoCoche;

// Función Principal de cada Hilo (Coche) 
void* rutina_coche(void* argumento) {
    InfoCoche* info = (InfoCoche*)argumento;
    const int id_coche = info->id;

    // PASO 1: LLEGADA Y TOMA DE TICKET 
    // El coche llega y se forma en la fila virtual tomando un número.
    pthread_mutex_lock(&mutex_turno); // Cierra el candado para tomar un ticket sin que nadie más interfiera.

    unsigned long mi_ticket = siguiente_ticket++;
    int posicion_en_fila = (int)(mi_ticket - ticket_actual + 1);
    if (posicion_en_fila < 1) posicion_en_fila = 1;
    
    // Actualizamos la estadística de la longitud máxima de la fila.
    if (posicion_en_fila > max_longitud_fila) max_longitud_fila = posicion_en_fila;

    double tiempo_llegada = tiempo_actual_s();
    printf("Coche %d ha LLEGADO. Ticket #%lu, es el numero %d en la fila.\n", id_coche, mi_ticket, posicion_en_fila);

    // PASO 2: ESPERAR SU TURNO 
    // El coche espera hasta que su ticket sea llamado.
    // `pthread_cond_wait` pone a "dormir" al hilo hasta que reciba una señal.
    // Esto evita que el hilo consuma CPU mientras espera.
    while (mi_ticket != ticket_actual) {
        pthread_cond_wait(&condicion_turno, &mutex_turno);
    }
    //  Libero el candado de los tickets para que otros coches puedan llegar y formarse.
    pthread_mutex_unlock(&mutex_turno);


    // PASO 3: ESPERAR UNA PLAZA LIBRE 
    // Ahora que es el primero en la fila, espera en la barrera (semáforo) a que haya una plaza.
    // `sem_wait` decrementa el contador de plazas. Si es 0, el hilo se bloquea aquí.
    sem_wait(&semaforo_plazas);

    // PASO 4: ENTRAR AL ESTACIONAMIENTO 
    // El coche puede entrar.
    double tiempo_entrada = tiempo_actual_s();
    double tiempo_espera = tiempo_entrada - tiempo_llegada;

    // Actualizamos las estadísticas de forma segura.
    pthread_mutex_lock(&mutex_estadisticas);
    coches_dentro++;
    coches_atendidos++;
    suma_total_espera_s += tiempo_espera;
    pthread_mutex_unlock(&mutex_estadisticas);

    // PASO 5: CEDER EL TURNO AL SIGUIENTE
    // Una vez dentro, avisa al siguiente en la fila que ya puede avanzar hacia la barrera.
    pthread_mutex_lock(&mutex_turno);
    ticket_actual++; // Avanzamos el ticket en la "pantalla".
    pthread_cond_broadcast(&condicion_turno); // Gritamos a todos los que esperan que revisen si es su turno.
    pthread_mutex_unlock(&mutex_turno);

    printf("Coche %d ENTRA (espero %.2fs). Ocupados: %d / %d\n",
           id_coche, tiempo_espera, coches_dentro, NUM_PLAZAS);

    // PASO 6: PERMANECER ESTACIONADO
    // El coche simula su estancia con una pausa aleatoria.
    int tiempo_estacionado = (rand() % TIEMPO_MAX_ESTACIONADO) + 1;
    sleep(tiempo_estacionado);

    // PASO 7: SALIR DEL ESTACIONAMIENTO
    pthread_mutex_lock(&mutex_estadisticas);
    coches_dentro--;
    pthread_mutex_unlock(&mutex_estadisticas);

    // `sem_post` incrementa el contador de plazas, liberando un espacio para alguien más.
    sem_post(&semaforo_plazas);
    printf("Coche %d SALE tras %ds. Plazas libres ahora: %d\n",
           id_coche, tiempo_estacionado, (int)(NUM_PLAZAS - coches_dentro));

    return NULL; // El hilo termina su ejecución.
}

//    Programa Principal
int main(void) {
    srand((unsigned)time(NULL)); // Inicializar la semilla para números aleatorios.

    // Inicializamos el semáforo con el número de plazas disponibles.
    if (sem_init(&semaforo_plazas, 0, NUM_PLAZAS) != 0) {
        perror("Error al inicializar el semáforo");
        return 1;
    }

    pthread_t hilos[MAX_COCHES];
    InfoCoche infos[MAX_COCHES];

    printf("\n INICIO DE LA SIMULACIÓN: %d plazas, %d coches \n\n", NUM_PLAZAS, MAX_COCHES);

    // Creamos los hilos (coches) con una llegada escalonada.
    for (int i = 0; i < MAX_COCHES; ++i) {
        infos[i].id = i + 1;
        if (pthread_create(&hilos[i], NULL, rutina_coche, &infos[i]) != 0) {
            perror("Error al crear el hilo");
        }
        // Pequeña pausa aleatoria para simular que no todos llegan al mismo tiempo.
        usleep((useconds_t)(rand() % (LLEGADA_MAX_MS * 1000)));
    }

    // Esperamos a que todos los hilos (coches) terminen su ciclo.
    for (int i = 0; i < MAX_COCHES; ++i) {
        pthread_join(hilos[i], NULL);
    }

    // Mostramos las estadísticas finales 
    double prom_espera = coches_atendidos ? (suma_total_espera_s / coches_atendidos) : 0.0;
    printf("\n--- RESUMEN DE LA SIMULACIÓN ---\n");
    printf("Total de coches atendidos: %d\n", coches_atendidos);
    printf("Tiempo promedio de espera en la fila: %.2f segundos\n", prom_espera);
    printf("Máxima longitud de la fila observada: %d coches\n", max_longitud_fila);
    printf("\n--- FIN DE LA SIMULACIÓN ---\n");

    // Liberamos los recursos utilizados.
    sem_destroy(&semaforo_plazas);
    pthread_mutex_destroy(&mutex_turno);
    pthread_mutex_destroy(&mutex_estadisticas);
    pthread_cond_destroy(&condicion_turno);

    return 0;
}
