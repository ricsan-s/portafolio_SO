// Sistemas operativos 4CV4
//Hernandez Bernal Angel Said, Sanchez Aguilar Ricardo, Lancón Barragán Erick Aarón

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_USUARIOS 10 // Usuarios 
#define NUM_RECURSOS 1 // Recursos 
#define TIEMPO_MAX_ESPERA 2 // Tiempo de espera. 
#define TIEMPO_MAX_USO 3 // Tiempo que puede usar el recurso

typedef struct {
    int id;
    int en_uso;
    sem_t semaforo; // Creación del semaforo que llamaremos "semaforo"
} Recurso;

// Estructura para pasar datos a los hilos de usuario
typedef struct {
    int id_usuario;
    Recurso *recursos;
} DatosUsuario;

// Generacion de datos ramdon para crear un entorno de recursos compartidos global 
int recurso_global = 0;
sem_t sem_recurso_global;

// Simulacion de interacciones 
void simular_actividad(const char* mensaje, int segundos) {
    printf("%s\n", mensaje);
    sleep(segundos);
}

// Función que ejecuta cada hilo de usuario
void* usuario(void* arg) {
    DatosUsuario* datos = (DatosUsuario*)arg;
    int id_usuario = datos->id_usuario;
    Recurso* recursos = datos->recursos;
    
    // Tiempo aleatorio antes de intentar acceder al recurso
    int tiempo_espera = 1 + rand() % TIEMPO_MAX_ESPERA; // Definims un rango al usar un Maximo de tiempo para no hacer una ejecucion tan larga.
    printf("Usuario %d: Esperando %d segundos antes de intentar acceder\n", // Generamos el mensaje de uso
           id_usuario, tiempo_espera); // recopilamos la informacion de quien esta en el proceso
    sleep(tiempo_espera); //Espera 
    
    // Intentar acceder a un recurso aleatorio
    int recurso_id = rand() % NUM_RECURSOS;
    //printf("Usuario %d: Intentando acceder al recurso %d\n", id_usuario, recurso_id);
    
    // Esperar por el semáforo del recurso específico
    sem_wait(&recursos[recurso_id].semaforo);
    recursos[recurso_id].en_uso = 1;
    
    printf("Usuario %d: Obtuvo acceso al recurso %d\n", //Tenemos el numero de recursos definidos al inicio, se puede modificar 
           id_usuario, recurso_id);
    sem_wait(&sem_recurso_global);
    printf("Usuario %d: Accediendo al recurso global (valor anterior: %d)\n", id_usuario, recurso_global);

    recurso_global += id_usuario;
    //printf("Usuario %d: Modificó recurso global (nuevo valor: %d)\n", id_usuario, recurso_global);
    
    // Simular una espera de uso
    int tiempo_uso = 1 + rand() % TIEMPO_MAX_USO;
    printf("Usuario %d: Usando recurso por %d segundos\n", id_usuario, tiempo_uso);
    sleep(tiempo_uso);
    
    // Liberar el recurso global
    printf("Usuario %d: Liberando recurso \n", id_usuario);
    sem_post(&sem_recurso_global);
    
    // Liberar el recurso específico
    printf("Usuario %d: Liberando recurso %d\n", id_usuario, recurso_id);
    recursos[recurso_id].en_uso = 0;
    sem_post(&recursos[recurso_id].semaforo);
    
    printf("Usuario %d: Terminó todas sus actividades\n", id_usuario);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t hilos_usuarios[NUM_USUARIOS];
    DatosUsuario datos_usuarios[NUM_USUARIOS];
    Recurso recursos[NUM_RECURSOS];
    
    // Números aleatorios
    srand(time(NULL));
    
    // Semáforo del recurso global
    sem_init(&sem_recurso_global, 0, 1);
    
    // Recursos
    for (int i = 0; i < NUM_RECURSOS; i++) {
        recursos[i].id = i;
        recursos[i].en_uso = 0;
        sem_init(&recursos[i].semaforo, 0, 1); // Semáforos binarios
    }
    
    printf("Sistema iniciado con %d usuarios y %d recursos\n\n", 
           NUM_USUARIOS, NUM_RECURSOS);
    
    // Creación de hilos simulando los usuarios 
    for (int i = 0; i < NUM_USUARIOS; i++) {
        datos_usuarios[i].id_usuario = i + 1;
        datos_usuarios[i].recursos = recursos;

        if (pthread_create(&hilos_usuarios[i], NULL, usuario, &datos_usuarios[i]) != 0) {
            perror("Error al crear hilo de usuario");
            exit(EXIT_FAILURE);
        }
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_USUARIOS; i++) {
        pthread_join(hilos_usuarios[i], NULL);
    }
    
    // Liberaciómn de recursos de los semáforos
    sem_destroy(&sem_recurso_global);
    for (int i = 0; i < NUM_RECURSOS; i++) {
        sem_destroy(&recursos[i].semaforo);
    }
    
    printf("\nTodos los usuarios han terminado. Valor final del recurso global: %d\n", 
           recurso_global);
    
    return 0;
}