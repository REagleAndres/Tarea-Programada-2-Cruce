#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define N_CARRILES 4

#ifndef N_VEHICULOS
#define N_VEHICULOS 10
#endif

#define LONGITUD_VEHICULO 32

int vehiculos_cruzados = 0;
int accidentes = 0;
int en_cruce = 0;

int cola[N_CARRILES] = {0};
int cruzados_por_carril[N_CARRILES] = {0};
char vehiculo_en_cruce[LONGITUD_VEHICULO] = "";

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

typedef struct {
    int fase;
    int total_vehiculos;
    int total_accidentes;
    int cruzados[N_CARRILES];
    double tiempo_total;
} ResultadoFase;

typedef enum {
    MODO_AMBAS,
    MODO_FASE_1,
    MODO_FASE_2
} ModoEjecucion;

double diferencia_tiempo(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) +
           (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

void reiniciar_estado(void) {
    vehiculos_cruzados = 0;
    accidentes = 0;
    en_cruce = 0;
    vehiculo_en_cruce[0] = '\0';

    memset(cola, 0, sizeof(cola));
    memset(cruzados_por_carril, 0, sizeof(cruzados_por_carril));
}

void copiar_vehiculo_en_cruce(const char *vehiculo) {
    snprintf(vehiculo_en_cruce, sizeof(vehiculo_en_cruce), "%s", vehiculo);
}

void actualizar_resultado(ResultadoFase *resultado, int fase,
                          double tiempo_total) {
    resultado->fase = fase;
    resultado->total_vehiculos = vehiculos_cruzados;
    resultado->total_accidentes = accidentes;
    resultado->tiempo_total = tiempo_total;

    for (int i = 0; i < N_CARRILES; i++) {
        resultado->cruzados[i] = cruzados_por_carril[i];
    }
}

void *simular_carril_fase_1(void *arg) {
    DatosCarril *datos = (DatosCarril *)arg;
    int id = datos->id_carril;
    unsigned int seed = (unsigned int)time(NULL) ^
                        (unsigned int)((id + 1) * 2654435761u);

    for (int i = 1; i <= N_VEHICULOS; i++) {
        char vehiculo[LONGITUD_VEHICULO];

        cola[id]++;
        snprintf(vehiculo, sizeof(vehiculo), "%s-%03d",
                 nombres_carriles[id], i);

        if (en_cruce == 1) {
            accidentes++;
            printf("[%s] entrando al cruce <- ACCIDENTE con %s\n",
                   vehiculo,
                   vehiculo_en_cruce[0] == '\0' ? "vehiculo desconocido"
                                                : vehiculo_en_cruce);
        } else {
            printf("[%s] entrando al cruce\n", vehiculo);
        }

        copiar_vehiculo_en_cruce(vehiculo);
        en_cruce = 1;

        usleep((rand_r(&seed) % 3000) + 2000);

        printf("[%s] cruce completado\n", vehiculo);

        en_cruce = 0;
        vehiculo_en_cruce[0] = '\0';

        vehiculos_cruzados++;
        cruzados_por_carril[id]++;
        cola[id]--;

        usleep((rand_r(&seed) % 2000) + 1000);
    }

    return NULL;
}

void *simular_carril_fase_2(void *arg) {
    DatosCarril *datos = (DatosCarril *)arg;
    int id = datos->id_carril;
    unsigned int seed = (unsigned int)time(NULL) ^
                        (unsigned int)((id + 1) * 2654435761u);

    for (int i = 1; i <= N_VEHICULOS; i++) {
        char vehiculo[LONGITUD_VEHICULO];

        cola[id]++;
        snprintf(vehiculo, sizeof(vehiculo), "%s-%03d",
                 nombres_carriles[id], i);

        sem_wait(&semaforo_cruce);

        if (en_cruce == 1) {
            sem_wait(&mutex_contadores);
            accidentes++;
            sem_post(&mutex_contadores);
            printf("[%s] sem_wait() -> ACCIDENTE con %s\n",
                   vehiculo,
                   vehiculo_en_cruce[0] == '\0' ? "vehiculo desconocido"
                                                : vehiculo_en_cruce);
        } else {
            printf("[%s] sem_wait() -> cruzando\n", vehiculo);
        }

        copiar_vehiculo_en_cruce(vehiculo);
        en_cruce = 1;

        usleep((rand_r(&seed) % 3000) + 2000);

        en_cruce = 0;
        vehiculo_en_cruce[0] = '\0';

        printf("[%s] sem_post() -> cruce libre\n", vehiculo);
        sem_post(&semaforo_cruce);

        sem_wait(&mutex_contadores);
        vehiculos_cruzados++;
        cruzados_por_carril[id]++;
        cola[id]--;
        sem_post(&mutex_contadores);

        usleep((rand_r(&seed) % 2000) + 1000);
    }

    return NULL;
}

const char *titulo_fase(int fase) {
    return fase == 1 ? "Sin sincronizacion" : "Con semaforos";
}

int ejecutar_fase(int fase, ResultadoFase *resultado) {
    pthread_t hilos[N_CARRILES];
    DatosCarril datos[N_CARRILES];
    struct timespec inicio;
    struct timespec fin;
    void *(*funcion_carril)(void *) =
        fase == 1 ? simular_carril_fase_1 : simular_carril_fase_2;
    int hilos_creados = 0;

    reiniciar_estado();

    printf("--- FASE %d: %s ---\n", fase, titulo_fase(fase));

    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("Error midiendo tiempo inicial");
        return -1;
    }

    for (int i = 0; i < N_CARRILES; i++) {
        datos[i].id_carril = i;

        if (pthread_create(&hilos[i], NULL, funcion_carril,
                           &datos[i]) != 0) {
            perror("Error creando hilo");

            for (int j = 0; j < hilos_creados; j++) {
                pthread_join(hilos[j], NULL);
            }

            return -1;
        }

        hilos_creados++;
    }

    for (int i = 0; i < N_CARRILES; i++) {
        pthread_join(hilos[i], NULL);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &fin) != 0) {
        perror("Error midiendo tiempo final");
        return -1;
    }

    actualizar_resultado(resultado, fase, diferencia_tiempo(inicio, fin));
    printf("\n");

    return 0;
}

int analizar_argumentos(int argc, char *argv[], ModoEjecucion *modo) {
    if (argc == 1) {
        *modo = MODO_AMBAS;
        return 0;
    }

    if (argc != 2) {
        return -1;
    }

    if (strcmp(argv[1], "fase_1") == 0) {
        *modo = MODO_FASE_1;
        return 0;
    }

    if (strcmp(argv[1], "fase_2") == 0) {
        *modo = MODO_FASE_2;
        return 0;
    }

    if (strcmp(argv[1], "ambas") == 0) {
        *modo = MODO_AMBAS;
        return 0;
    }

    return -1;
}

void imprimir_uso(const char *programa) {
    fprintf(stderr, "Uso: %s [fase_1|fase_2|ambas]\n", programa);
}

void imprimir_encabezado(void) {
    printf("========================================================\n");
    printf(" SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117\n");
    printf(" Carriles: %d | Vehiculos por carril: %d\n",
           N_CARRILES, N_VEHICULOS);
    printf("========================================================\n\n");
}

void imprimir_reporte_fase(const ResultadoFase *resultado) {
    const char *descripcion =
        resultado->fase == 1 ? "sin sincronizacion" : "con semaforos";

    printf("FASE %d (%s):\n", resultado->fase, descripcion);
    printf(" Total vehiculos:    %d\n", resultado->total_vehiculos);
    printf(" Accidentes:         %d",
           resultado->total_accidentes);

    if (resultado->fase == 1) {
        printf("    <- debe ser > 0");
    } else {
        printf("    <- debe ser exactamente 0");
    }

    printf("\n");
    printf(" Vehiculos/carril:   Norte=%d  Sur=%d  Este=%d  Oeste=%d\n",
           resultado->cruzados[0],
           resultado->cruzados[1],
           resultado->cruzados[2],
           resultado->cruzados[3]);
    printf(" Tiempo simulacion:  %.3f segundos\n\n",
           resultado->tiempo_total);
}

void imprimir_reporte_final(const ResultadoFase *fase_1,
                            const ResultadoFase *fase_2) {
    printf("======== REPORTE FINAL ========\n\n");

    if (fase_1 != NULL) {
        imprimir_reporte_fase(fase_1);
    }

    if (fase_2 != NULL) {
        imprimir_reporte_fase(fase_2);
    }

    if (fase_1 != NULL && fase_2 != NULL) {
        double overhead = fase_2->tiempo_total - fase_1->tiempo_total;
        double porcentaje =
            fase_1->tiempo_total > 0.0
                ? (overhead / fase_1->tiempo_total) * 100.0
                : 0.0;

        printf("ANALISIS:\n");
        printf(" Overhead de sincronizacion: %+.3f seg (%+.1f%%)\n",
               overhead, porcentaje);
    }

    printf("========================================================\n");
}

int inicializar_semaforos(void) {
    if (sem_init(&semaforo_cruce, 0, 1) != 0) {
        perror("Error inicializando semaforo_cruce");
        return -1;
    }

    if (sem_init(&mutex_contadores, 0, 1) != 0) {
        perror("Error inicializando mutex_contadores");
        sem_destroy(&semaforo_cruce);
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    ModoEjecucion modo;
    ResultadoFase resultado_fase_1;
    ResultadoFase resultado_fase_2;
    ResultadoFase *fase_1 = NULL;
    ResultadoFase *fase_2 = NULL;
    int estado = EXIT_SUCCESS;

    if (analizar_argumentos(argc, argv, &modo) != 0) {
        imprimir_uso(argv[0]);
        return EXIT_FAILURE;
    }

    if (inicializar_semaforos() != 0) {
        return EXIT_FAILURE;
    }

    imprimir_encabezado();

    if (modo == MODO_AMBAS || modo == MODO_FASE_1) {
        if (ejecutar_fase(1, &resultado_fase_1) != 0) {
            estado = EXIT_FAILURE;
        } else {
            fase_1 = &resultado_fase_1;
        }
    }

    if (estado == EXIT_SUCCESS &&
        (modo == MODO_AMBAS || modo == MODO_FASE_2)) {
        if (ejecutar_fase(2, &resultado_fase_2) != 0) {
            estado = EXIT_FAILURE;
        } else {
            fase_2 = &resultado_fase_2;
        }
    }

    if (estado == EXIT_SUCCESS) {
        imprimir_reporte_final(fase_1, fase_2);
    }

    sem_destroy(&mutex_contadores);
    sem_destroy(&semaforo_cruce);

    return estado;
}
