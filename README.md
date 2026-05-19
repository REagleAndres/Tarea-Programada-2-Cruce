# Tarea-Programada-2-Cruce  

## Integrantes:
#### Andres Villegas Baltodano C18578
#### Emanuel Gonzalez Chaves C03361

## Instrucciones:
Para compilar utilize el comando make dentro de la carpeta Tarea-Programada-2-Cruce, para correr el programa, make run correra la fase 3, make run_fase_1 correra unicamente la fase 1 y make run_fase_2 correra unicamente la fase 2 del programa

#### a) Race Condition: Identifique exactamente cuáles líneas del código de Fase 1 constituyen una race condition. ¿Por qué la instrucción en_cruce = 1 sola no es suficiente para garantizar exclusión mutua?  
  
  Si bien la linea, cruce = 1 deberia prevenir la entrada de mas de un carro al cruce, una condicion de carrera entre los hilos ocurre al comprobar cruce == 1, ya que si un hilo realiza la operacion cruce = 1 mientras otro realiza la comprobacion cruce == 1, eset ultimo podra entrar al cruce pese al previo estando en cruce. Ademas al realizar cruce = 1 y cruce = 0 tambien se pueden ocurrir condiciones de carrera.
  

#### b) Invariante del semáforo: ¿Qué invariante garantiza el semáforo binario en Fase 2? Explíquelo con sus propias palabras. ¿Por qué se inicializa en 1 y no en 0?  
  
La invariante 1 debe ser utilizada, al tener un solo recurso esta debe ser 1 para controlar ese solo recurso. Ademas inicializar en 0 causaria inmediatos  problemas al causar que ningun hilo puede bloquear correctamente la seccion critica ademas de no poder entrar a la seccion critica.
  

#### c) Overhead: ¿Por qué la Fase 2 suele tomar más tiempo que la Fase 1? ¿Este overhead es aceptable? ¿En qué tipo de sistema NO sería aceptable?  
  
La fase 2 posee semaforos para controlar el flujo de carros en el cruce, esto efectivamente serializa la region critica del codigo causando que un solo hilo pueda realizar dicha operacion. Para propositos de esta simulacion ya que el cruce de los carros debe ser controlado, este oberhead es aceptable al ser una consecuencia del control de condiciones de carrera. Por otro lado en un sistema que priorize la velocidad sobre la precision, este overhead puede verse como una consecuencia no aceptable.
  
#### d) Experimento: Presente los resultados del experimento de escala (N_VEHICULOS = 5, 20, 50). ¿Cómo cambia la tasa de accidentes al aumentar el número de vehículos? ¿Tiene sentido ese comportamiento?  
  
##### Para N_VEHICULOS = 5:
&emsp; FASE 1 (sin sincronizacion):
&emsp; Total vehiculos:    20
&emsp; Accidentes:         11    <- debe ser > 0
&emsp; Vehiculos/carril:   Norte=5  Sur=5  Este=5  Oeste=5
&emsp; Tiempo simulacion:  0.031 segundos
  
##### Para N_VEHICULOS = 20:
&emsp; FASE 1 (sin sincronizacion):
&emsp; Total vehiculos:    80
&emsp; Accidentes:         42    <- debe ser > 0
&emsp; Vehiculos/carril:   Norte=20  Sur=20  Este=20  Oeste=20
&emsp; Tiempo simulacion:  0.119 segundos
  
##### Para N_VEHICULOS = 50:
FASE 1 (sin sincronizacion):
&emsp; Total vehiculos:    200
&emsp; Accidentes:         84    <- debe ser > 0
&emsp; Vehiculos/carril:   Norte=50  Sur=50  Este=50  Oeste=50
&emsp; Tiempo simulacion:  0.292 segundos
  
Vemos que el numero de accidentes es similar al $\frac{total De Vehiculos}{2}$, lo cual tiene sentido si tomamos en cuenta que las condiciones de carrera dependeran del numero de veces que relizemos un cruce, con mayores vehiculos mayor probabilidad de una condicion de carrera.

#### e) Extensión (opcional, bonus +0.5): ¿Cómo modificaría el sistema para permitir que más de un vehículo cruce simultáneamente (carril de doble vía)? ¿Qué tipo de semáforo usaría?
  
Esto es relativamente sencillo, ya que podemos interpretar un carril de doble via como que tenemos dos recursos en ves de uno, por lo tanto inicializar el semaforo en 2 en vez de en 1 y verificar para cruce == 2 serian los cambios necesarios para dicha simulacion.