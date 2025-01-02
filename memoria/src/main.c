#include "../include/inicioYFin.h"
#include "../include/estructuras.h"
#include "../include/paginacion.h"
#include <../../utils/include/comunicacion.h>

int main(int argc, char** argv) 
{
    //inicializamos el logger
    t_log* logger = iniciarLogger();

    //Cargamos los valores del config
    t_config* config = iniciarConfig(argv[1]);
    t_valoresM valores = cargarConfig(config);

    //Inicializo la memoria void*
    void* memoria = iniciarMemoria(valores);

    // Cadena donde se guarda el bit array de frames
    static char* cadena;

    //Inicializo el bitarray de frames
    t_bitarray* framesMemoria= iniciarBitArray(valores.tamanoMemoria, valores.tamanoPagina , cadena, logger);

    //Creo la lista de los procesos y el directorio de tablas de páginas
    t_list* directorio = list_create();
    t_list* listaProcesos = list_create();

    //Creo las conexiones con los otros modulos
    int conexion = crearConexiones(logger, valores, listaProcesos, memoria, directorio, framesMemoria);

    argsEsperarMensaje* argumentosCPU = malloc(sizeof(argsEsperarMensaje));
    
	//Cargo los argumentos para CPU
    argumentosCPU->rol = SERVIDOR;
    argumentosCPU->logger = logger;
    argumentosCPU->memoria = memoria;
    argumentosCPU->escucha = MEMORIA;
    argumentosCPU->cliente = conexion;
    argumentosCPU->directorio = directorio;
    argumentosCPU->path = valores.pathPruebas;
    argumentosCPU->framesMemoria = framesMemoria;
    argumentosCPU->listaProcesos = listaProcesos;
    argumentosCPU->tamanioPagina = valores.tamanoPagina;
    argumentosCPU->retardoRespuesta = valores.retardoRespuesta;

    atenderCliente(argumentosCPU);

    terminarPrograma(conexion, logger, config, memoria);

    return 0;
}