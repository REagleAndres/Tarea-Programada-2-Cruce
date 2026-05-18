#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>

#define N_CARRILES 4
#define N_VEHICULOS 10
#define SIMULAR_CARRIL(a) ((a) == 1 ? simular_carril : simular_carril_sem)

int vehiculos_cruzados = 0;
int accidentes = 0;
int en_cruce = 0;

int cola[N_CARRILES] = {0};
int cruzados_por_carril[N_CARRILES] = {0};
sem_t semaforo_cruce;
sem_t mutex_contadores;

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

void *simular_carril_sem(void *arg) {
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
         * FASE 2:
         * CON sincronización
         * Race condition evitadas
         */

        //Check y cruce 
        sem_wait(&semaforo_cruce);

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
        sem_post(&semaforo_cruce);

        /* Actualizar contadores */
        sem_wait(&mutex_contadores);
        vehiculos_cruzados++;
        cruzados_por_carril[id]++;
        cola[id]--;
        sem_post(&mutex_contadores);

        /* Pequeña pausa aleatoria */
        usleep((rand_r(&seed) % 2000) + 1000);
    }

    return NULL;
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

int analizar_argumentos(int argc, char *argv[]){
    if(argc > 2){
        return -1;
    }
    if(strcmp(argv[1], "fase_1") == 0){
        return 1;
    } else if (strcmp(argv[1], "fase_2") == 0){
        return 2;
    } else{
        return -1;
    }
}

void reporte_final(double tiempo_total, int fase){
        /* Reporte final */
    printf("\n========================================================\n");
    printf(" SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117\n");
    printf(" Carriles: 4 | Vehiculos por carril: %d\n", N_VEHICULOS);
    printf("========================================================\n");

    char *sincro = (fase) == 1 ? "Sin" : "Con";

    printf("--- FASE %d: %s sincronizacion ---\n", fase, sincro);

    printf("\n======== REPORTE FINAL ========\n");

    printf("FASE %d (sin sincronizacion):\n", fase);
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
}

int main(int argc, char *argv[]) {
    sem_init(&semaforo_cruce, 0, 1);
    sem_init(&mutex_contadores, 0, 1);
    int argumento = analizar_argumentos(argc, argv);
    if(argumento == -1){
        perror("Error, demasiados argumentos, asegurese de mandar un solo argumento, numero de aargumentos");
        return EXIT_FAILURE;
    }


    pthread_t hilos[N_CARRILES];
    DatosCarril datos[N_CARRILES];

    struct timespec inicio, fin;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    /* Crear los 4 hilos */
    for (int i = 0; i < N_CARRILES; i++) {
        datos[i].id_carril = i;

        if (pthread_create(&hilos[i], NULL,
                           SIMULAR_CARRIL(argumento), &datos[i]) != 0) {
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

    reporte_final(tiempo_total, argumento);

    return 0;
}