//------------BIBLIOTECAS------------

#include <../include/inicioYFin.h>

//------------DEFINICION DE FUNCIONES------------

//Inicializo el config
t_config* iniciarConfig(char* path)
{
    t_config* nuevo_config = config_create(path);

	if(nuevo_config == NULL){
		perror("Error al crear el config");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}

//Cargamos los valores de la configuracion
t_valoresM cargarConfig(t_config* config)
{
    t_valoresM cValores;

	//Cargo los valores del config en la estructura
	cValores.tamanoPagina = config_get_int_value(config, "TAM_PAGINA");
	cValores.tamanoMemoria = config_get_int_value(config, "TAM_MEMORIA");
	cValores.puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	cValores.pathPruebas = config_get_string_value(config, "PATH_INSTRUCCIONES");
	cValores.retardoRespuesta = config_get_int_value(config, "RETARDO_RESPUESTA"); 

    return cValores;
}

//Inicializo el logger
t_log* iniciarLogger()
{
	t_log* nuevo_logger = log_create("memoria.log", "memoria-info", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

//Funcion para crear las conexiones
int crearConexiones(t_log* logger, t_valoresM valores, t_list* listaProcesos, void* memoria, t_list* directorio, t_bitarray* framesMemoria)
{
	//Creo el server y espero conexiones
    int conexion = iniciarServidor(valores.puertoEscucha, logger);

	//Creo la estructura de argumentos para hacer los hilos
    argsEsperarMensaje* argumentosKernel = malloc(sizeof(argsEsperarMensaje));
    argsEsperarMensaje* argumentosInterfaz = malloc(sizeof(argsEsperarMensaje));
    
	//Cargo los argumentos para Kernel
    argumentosKernel->rol = SERVIDOR;
    argumentosKernel->logger = logger;
    argumentosKernel->memoria = memoria;
    argumentosKernel->escucha = MEMORIA;
    argumentosKernel->cliente = conexion;
    argumentosKernel->directorio = directorio;
    argumentosKernel->path = valores.pathPruebas;
    argumentosKernel->framesMemoria = framesMemoria;
    argumentosKernel->listaProcesos = listaProcesos;
    argumentosKernel->tamanioPagina = valores.tamanoPagina;
    argumentosKernel->retardoRespuesta = valores.retardoRespuesta;

    //Creo el hilo Kernel
    crearHiloEscucha(argumentosKernel);

    //Creo el hilo InterfazIO
    *argumentosInterfaz = *argumentosKernel;
    argumentosInterfaz->rol = INTERFACES;
    crearHiloEscucha(argumentosInterfaz); 

	return conexion;
}


//Funcion para finalizar el programa
void terminarPrograma( int conexionCliente, t_log* logger, t_config* config , void* memoria)
{
    free(memoria);
    log_destroy(logger);
    close(conexionCliente);
	config_destroy(config);
}