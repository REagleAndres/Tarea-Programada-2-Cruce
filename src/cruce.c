#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define N_CARRILES 4
#define N_VEHICULOS 10

int vehiculos_cruzados = 0;
int accidentes = 0;
int en_cruce = 0;

int cola[N_CARRILES] = {0};
int cruzados_por_carril[N_CARRILES] = {0};

const char *nombres_carriles[N_CARRILES] = {
    "Norte",
    "Sur",
    "Este",
    "Oeste"
};

typedef struct {
    int id_carril;
} DatosCarril;

/* Función para calcular diferencia de tiempo */
double diferencia_tiempo(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) +
           (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

/* Función ejecutada por cada hilo */
void *simular_carril(void *arg) {
    DatosCarril *datos = (DatosCarril *)arg;
    int id = datos->id_carril;

    unsigned int seed = time(NULL) ^ pthread_self();

    for (int i = 1; i <= N_VEHICULOS; i++) {

        /* Generar vehículo */
        cola[id]++;

        char vehiculo[32];
        snprintf(vehiculo, sizeof(vehiculo),
                 "%s-%03d", nombres_carriles[id], i);

        /*
         * FASE 1:
         * SIN sincronización
         * Race condition intencional
         */

        if (en_cruce == 1) {
            accidentes++;
            printf("[%s] entrando al cruce <- ACCIDENTE detectado\n",
                   vehiculo);
        } else {
            printf("[%s] entrando al cruce\n", vehiculo);
        }

        /* Vehículo entra al cruce */
        en_cruce = 1;

        /* Simular tiempo de cruce */
        usleep((rand_r(&seed) % 3000) + 2000);

        printf("[%s] cruce completado\n", vehiculo);

        /* Salir del cruce */
        en_cruce = 0;

        /* Actualizar contadores */
        vehiculos_cruzados++;
        cruzados_por_carril[id]++;
        cola[id]--;

        /* Pequeña pausa aleatoria */
        usleep((rand_r(&seed) % 2000) + 1000);
    }

    return NULL;
}

int main() {

    pthread_t hilos[N_CARRILES];
    DatosCarril datos[N_CARRILES];

    struct timespec inicio, fin;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    printf("========================================================\n");
    printf(" SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117\n");
    printf(" Carriles: 4 | Vehiculos por carril: %d\n", N_VEHICULOS);
    printf("========================================================\n");

    printf("--- FASE 1: Sin sincronizacion ---\n");

    /* Crear los 4 hilos */
    for (int i = 0; i < N_CARRILES; i++) {
        datos[i].id_carril = i;

        if (pthread_create(&hilos[i], NULL,
                           simular_carril, &datos[i]) != 0) {
            perror("Error creando hilo");
            return EXIT_FAILURE;
        }
    }

    /* Esperar todos los hilos */
    for (int i = 0; i < N_CARRILES; i++) {
        pthread_join(hilos[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_total = diferencia_tiempo(inicio, fin);

    /* Reporte final */
    printf("\n======== REPORTE FINAL ========\n");

    printf("FASE 1 (sin sincronizacion):\n");
    printf(" Total vehiculos: %d\n", vehiculos_cruzados);
    printf(" Accidentes: %d\n", accidentes);

    printf(" Vehiculos/carril: ");
    printf("Norte=%d ",
           cruzados_por_carril[0]);

    printf("Sur=%d ",
           cruzados_por_carril[1]);

    printf("Este=%d ",
           cruzados_por_carril[2]);

    printf("Oeste=%d\n",
           cruzados_por_carril[3]);

    printf(" Tiempo simulacion: %.3f segundos\n",
           tiempo_total);

    printf("========================================================\n");

    return 0;
}