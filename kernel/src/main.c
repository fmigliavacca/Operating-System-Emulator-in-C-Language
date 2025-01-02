//------------BIBLIOTECAS------------

#include <../include/consola.h>
#include <../include/inicioYFin.h>
#include <../include/estructuras.h>
#include <../include/planificador.h>

//------------CODIGO MAIN KERNEL------------

int main(int argc, char** argv) 
{
    //------------INCIALIZACION DE VARIABLES Y ESTRUCTURAS NECESARIAS------------

    //Inicializo un logger
    t_log* logger = iniciarLogger();
    
    log_info(logger, "Arranco el programa");

    //Creo un config y le cargo los valores
    t_config* config = iniciarConfig(argv[1]);
    t_valores valores = cargarConfig(config);

    //Creo la estructura de argumentos para hacer los hilos
    argsEsperarMensaje* argumentos = malloc(sizeof(argsEsperarMensaje));

    //Creo las conexiones
    t_conexiones conexiones = crearConexiones(valores, logger, argumentos);

    //Inicializo semaforos y cola ready
    sem_init(&semPCBNew, 0, 0);
    sem_init(&semPCBReady, 0, 0);
    sem_init(&semAtenderIO, 0, 1);
    sem_init(&semPCBBlocked, 0, 0);
    sem_init(&semPlanificador, 0, 1);
    sem_init(&semPCBExecuting, 0, 1);
    sem_init(&semPCBMultiProgramacion, 0, valores.gradoMultiProgramacion);

    //------------PLANIFICADOR------------

    //Obtengo los argumentos para el planificador y creo el hilo
    argsPlanificacion* argsPlanificacion = crearHiloPlanificador(logger, valores, conexiones, argumentos);

    //------------CONSOLA------------

    //Llamo a la consola interactiva
    consola(argsPlanificacion);

    //Libero los recursos
    terminarPrograma(conexiones.conexionMemoria, conexiones.conexionInterrupt, conexiones.conexionDispatch, conexiones.conexionInterfazIO, logger, config);

    return 0;
}