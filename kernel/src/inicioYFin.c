//------------BIBLIOTECAS------------

#include <../include/inicioYFin.h>

//------------DEFINICION DE FUNCIONES------------

//Funcion para cargar los valores del config
t_valores cargarConfig(t_config* config)
{
	//Creo la estructura para almacenar los valores
    t_valores valores;

	//Cargo los valores del config en la estructura
	valores.quantum = config_get_int_value(config,"QUANTUM");
    valores.ipCpu = config_get_string_value(config, "IP_CPU");
	valores.recursos = config_get_array_value(config, "RECURSOS");
	valores.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
	valores.pathCarpeta = config_get_string_value(config, "PATH_SCRIPT");
	valores.puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
	valores.puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	valores.instanciasRecursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
	valores.puertoCpuDispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	valores.puertoCpuInterrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	valores.gradoMultiProgramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	valores.algoritmoPlanificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

	//Retorno la estructura creada
    return valores;
}

//Funcion para inicializar el config
t_config* iniciarConfig(char* path)
{
	//Creo un nuevo config
	t_config* nuevo_config = config_create(path);

	//Me fijo que se haya creado correctamente
	if(nuevo_config == NULL)
	{
		perror("Error al crear el config");
		exit(EXIT_FAILURE);
	}

	//Retorno el config creado
	return nuevo_config;
}

//Funcion para inicializar el logger
t_log* iniciarLogger(void)
{
	//Creo el logger
	t_log* nuevo_logger = log_create("kernel.log", "kernel-info", true, LOG_LEVEL_INFO);

	//Retorno el logger
	return nuevo_logger;
}

//Funcion para liberar recursos
void terminarPrograma(int conexionMemoria, int conexionCpuInterrupt, int conexionCpuDispatch, int conexionInterfazIO, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
    close(conexionMemoria);
	close(conexionInterfazIO);
    close(conexionCpuDispatch);
    close(conexionCpuInterrupt);
}

//Funcion para crear las conexiones
t_conexiones crearConexiones(t_valores valores, t_log* logger, argsEsperarMensaje* args)
{
	//Creo uns estructura para almacenar los valores
	t_conexiones conexion;

	//Cargo los valores en la estructura
	log_info(logger, "Memoria");
    conexion.conexionMemoria = crearConexion(valores.ipMemoria, valores.puertoMemoria, logger);
    log_info(logger, "Interrupt");
    conexion.conexionInterrupt = crearConexion(valores.ipCpu, valores.puertoCpuInterrupt, logger);
    log_info(logger, "Dispatch");
    conexion.conexionDispatch = crearConexion(valores.ipCpu, valores.puertoCpuDispatch, logger);
	conexion.conexionInterfazIO = iniciarServidor(valores.puertoEscucha, logger);

	//Cargo los argumentos para InterfazIO
    args->logger = logger;
    args->rol = INTERFACES;
	args->escucha = KERNEL;
    args->cliente = conexion.conexionInterfazIO;
    args->interfacesSTDIN = inicializarListasYSemaforos();
    args->interfacesSTDOUT = inicializarListasYSemaforos();
    args->interfacesDIALFS = inicializarListasYSemaforos();
    args->interfacesGenericas = inicializarListasYSemaforos();

    //Creo el hilo InterfazIO
    crearHiloEscucha(args);

	//Retorno la estructura
	return conexion;
}

//Funcion para crear la lista de recurso con los valores del config
t_lista* crearListaRecursos(t_valores valores)
{
	//Inicializo las listas necesrias para manejar los recursos
	t_lista* listaRecursos = inicializarListasYSemaforos();

	//Recorro la lista de recursos mientras sea distinto de NULL
	while(*(valores.recursos) != NULL)
	{
		//Paso a int la cantidad de instancias de los recursos
		int instancias = atoi(*(valores.instanciasRecursos));

		//Creo un recurso y le cargo los valores del extraido del config
		t_recursos* recurso = malloc(sizeof(t_recursos));

		recurso->nombre = *(valores.recursos);
		recurso->PID = list_create();
		sem_init(&recurso->disponible, 0, instancias);

		//Le agrego tantos nodos de instancias a la lista como instancias del recurso haya(-1 disponible, != -1 ocupado)
		for(int i = 0; i < instancias; i++)
		{
			int* PIDrecurso = malloc(sizeof(int));
			*PIDrecurso = -1;

			list_add(recurso->PID, PIDrecurso);
		}
		
		//Agrego el recurso a la lista
		list_add(listaRecursos->lista, recurso);

		valores.recursos++;
		valores.instanciasRecursos++;
	}

	//Retorno la lista que acabamos de crear
	return listaRecursos;
}

//Creo el planificador pasandole los parametros
argsPlanificacion* crearHiloPlanificador(t_log* logger, t_valores valores, t_conexiones conexiones, argsEsperarMensaje* argsMensajes)
{
	//Creo las colas a utilizar
    t_lista* colaNew = inicializarListasYSemaforos();
    t_lista* colaReady = inicializarListasYSemaforos();
    t_lista* colaBlocked = inicializarListasYSemaforos();
    t_lista* colaPrioritaria = inicializarListasYSemaforos();

	//Creo la lista de recursos
	t_lista* listaRecursos = crearListaRecursos(valores);

    //Paso los parametros para el hilo
    argsPlanificacion* args = malloc(sizeof(argsPlanificacion));

    args->rol = PCB;
    args->logger = logger;
	args->colaNew = colaNew;
    args->valores = valores;
    *(args->PIDExecuting) = -1;
	args->conexiones = conexiones;
    args -> colaReady = colaReady;
	args->overFlowProgramacion = 0;
	args -> colaBlocked = colaBlocked;
	args->listaRecursos = listaRecursos;
	args->PIDExecuting = malloc(sizeof(int));
    args -> colaPrioritaria = colaPrioritaria;
    args->interfacesSTDIN = argsMensajes->interfacesSTDIN;
    args->interfacesSTDOUT = argsMensajes->interfacesSTDOUT;
    args->interfacesDIALFS = argsMensajes->interfacesDIALFS;
	args->interfacesGenericas = argsMensajes->interfacesGenericas;

    //Creo el hilo del planificador
    pthread_t* hilo = malloc(sizeof(pthread_t));
    pthread_create(hilo, NULL, hiloPlanificador, args);
    pthread_detach(*hilo);

	free(hilo);

	return args;
}