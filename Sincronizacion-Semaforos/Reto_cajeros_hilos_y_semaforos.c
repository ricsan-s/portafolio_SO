/* Hernandez Bernal Ángel Said, Sanchez Aguilar Ricardo,Lancon Barragan Erick Aaron
Sistemas operativos 4cv4
Diseñar codigo Reto Diseñar programa en c que simule 3 cajeros automaticos usando hilos y semáforos*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>//tiempos de descanso
#include <time.h>
//
#define cajeros 3   // Cantidad de cajeros, según los requisitos.
#define transisciones 4 // Cada cajero realizará 4 transacciones.
#define saldo_ini 500.0 // Saldo inicial para cada uno de los cajeros.
//definimos montos de ejemplos
#define MONTO_MIN 10.0  // Monto mínimo para una transacción.
#define MONTO_MAX 40.0  // Monto máximo para una transacción.

void* funcion_cajero(void* arg);

double saldos_cajeros[cajeros];             // Saldo de cada cajero.
pthread_mutex_t mutex_saldos[cajeros];     

// Estructura para pasar el ID del cajero  .
typedef struct {
    int id_Cajero;
} cajero_data_t;


int main() {
    //  variables locales
    pthread_t hilos_cajeros[cajeros];
    cajero_data_t datos_cajeros[cajeros];
    int i; 
    double saldo_global = 0.0;

    // Inicializar la semilla para números aleatorios para que cada ejecución sea diferente.
    srand(time(NULL));

    printf("Iniciando con %d cajeros\n", cajeros);
    for (i = 0; i < cajeros; i++) {
        saldos_cajeros[i] = saldo_ini;
        
        // Inicializar el mutex para el saldo de cada cajero.
        if (pthread_mutex_init(&mutex_saldos[i], NULL) != 0) {
            printf("Error al inicializar el mutex para el cajero %d\n", i + 1);
            return 1; // Terminar con error
        }
        printf("Cajero %d inicializado con saldo: $%.2f\n", i + 1, saldos_cajeros[i]);
    }

    // HILOS 
    for (i = 0; i < cajeros; i++) {
        datos_cajeros[i].id_Cajero = i + 1;
        if (pthread_create(&hilos_cajeros[i], NULL, funcion_cajero, (void*)&datos_cajeros[i]) != 0) {
            printf("Error al crear el hilo para el cajero %d\n", i + 1);
            return 1; // Terminar con error
        }
    }

    // Espera a que todos los hilos de los cajeros terminen su ejecución.
    for (i = 0; i < cajeros; i++) {
        pthread_join(hilos_cajeros[i], NULL);
    }

    // RESULTADOS FINALES
    printf("--- Saldos finales de cada cajero ---\n");
    for (i = 0; i < cajeros; i++) {
        printf("Saldo final Cajero %d: $%.2f\n", i + 1, saldos_cajeros[i]);
        saldo_global += saldos_cajeros[i];
    }

    printf("SALDO TOTAL DEL SISTEMA: $%.2f\n", saldo_global);


    for (i = 0; i < cajeros; i++) {
        pthread_mutex_destroy(&mutex_saldos[i]);
    }

    return 0; 
}

void* funcion_cajero(void* arg) {
    // Se obtiene el ID del cajero a partir del argumento recibido.
    cajero_data_t* data = (cajero_data_t*)arg;
    int id_Cajero = data->id_Cajero;
    int indice_cajero = id_Cajero - 1;

    for (int i = 0; i < transisciones; i++) {
        usleep((rand() % 800 + 200) * 1000);
        int tipo_operacion = rand() % 3; // 0: Depósito, 1: Retiro, 2: Pago
        double monto = MONTO_MIN + ((double)rand() / RAND_MAX) * (MONTO_MAX - MONTO_MIN);
        
        char* nombre_operacion;
        pthread_mutex_lock(&mutex_saldos[indice_cajero]);

        // verifica si el cajero tiene saldo.
        if (saldos_cajeros[indice_cajero] <= 0 && (tipo_operacion == 1 || tipo_operacion == 2)) {
            printf("Cajero %d SIN SALDO. Transacción cancelada.\n", id_Cajero);
            // Se libera el mutex antes de saltar a la siguiente iteración.
            pthread_mutex_unlock(&mutex_saldos[indice_cajero]);
            continue; 
        }
        //Seccion de casos de operaciones 
        switch (tipo_operacion) {
            case 0: // Depósito
                nombre_operacion = "Deposito";
                saldos_cajeros[indice_cajero] += monto;
                break;
            case 1: // Retiro
                nombre_operacion = "Retiro";
                if (saldos_cajeros[indice_cajero] >= monto) {
                    saldos_cajeros[indice_cajero] -= monto;
                } else {
                    monto = 0; 
                }
                break;
            case 2: // Pago
                nombre_operacion = "Pago";
                if (saldos_cajeros[indice_cajero] >= monto) {
                    saldos_cajeros[indice_cajero] -= monto;
                } else {
                    monto = 0; // No se paga nada.
                }
                break;
        }

        // Resultado de la transacción.
        printf("Cajero %d | %s: $%.2f | Saldo Actual: $%.2f\n", 
               id_Cajero, 
               nombre_operacion, 
               monto, 
               saldos_cajeros[indice_cajero]);
        
              pthread_mutex_unlock(&mutex_saldos[indice_cajero]);
    }

    printf("El cajero %d ha finalizado las %d operaciones\n", id_Cajero, transisciones);
    return NULL;
}