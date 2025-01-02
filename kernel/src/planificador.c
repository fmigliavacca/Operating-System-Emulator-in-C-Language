//------------BIBLIOTECAS------------

#include <../include/planificador.h>

//------------DEFINICION DE SEMAFOROS------------

//Defino los semaforos
sem_t semPCBNew;
sem_t semPCBReady;
sem_t semAtenderIO;
sem_t semPCBBlocked;
sem_t semPCBExecuting;
sem_t semPlanificador;
sem_t semPCBMultiProgramacion;

//------------FUNCION PRINCIPAL------------

//Funcion main del planificador
void* hiloPlanificador(void* argumentos)
{
	//Cargo los valores del void* a una estructura
	argsPlanificacion* args = (argsPlanificacion*)argumentos;

	//Creo el hilo para pasar a Ready
	crearHiloPasarAReady(args);

	//Me fijo cual es el algoritmo de planificacion que se esta utilizando
    if(strcmp(args->valores.algoritmoPlanificacion, "FIFO") == 0)
    {
		while(1)
		{
			//Verifico que no haya un proceso en ejecucion
			sem_wait(&semPCBExecuting);

			//Verifico que hay procesos en Ready
			sem_wait(&semPCBReady);

			//Llamo al planificador con FIFO
            planificadorFIFO(args);
        }
	}

    else if(strcmp(args->valores.algoritmoPlanificacion, "RR") == 0)
    {   
		while(1)
		{
			//Verifico que no haya un proceso en ejecucion
			sem_wait(&semPCBExecuting);

			//Verifico que hay procesos en Ready
			sem_wait(&semPCBReady);

			//Llamo al planificador con RR
            planificadorRR(args);
		}
    }

    else if(strcmp(args->valores.algoritmoPlanificacion, "VRR") == 0)
    {
		while(1)
		{
			//Verifico que no haya un proceso en ejecucion
			sem_wait(&semPCBExecuting);

			//Verifico que hay procesos en Ready
			sem_wait(&semPCBReady);

			//Llamo al planificador con VRR
            planificadorVRR(args);
		}
    }

	free(args);

	return NULL;
}

//------------FUNCIONES PARA ENCOLAR PROCESOS------------

//Funcion para encolar un proceso nuevo
void encolarNew(t_PCB* PCB, t_lista* colaNew, t_log* logger)
{
	//Pusheamos el nuevo PCB a New
	pthread_mutex_lock(&(colaNew->mutex));
    list_add(colaNew->lista, PCB);
    pthread_mutex_unlock(&(colaNew->mutex));
    
	log_info(logger,"Se crea el proceso %d en NEW", PCB->PID);
	
	//Avisamos que hay procesos esperando a pasar a Ready
	sem_post(&semPCBNew);
}

//Funcion para encolar un proceso que requiere de una interfaz I/O
void encolarBlocked(t_PCB* PCB, t_lista* colaBlocked, t_log* logger)
{
	//Cargo el PCB en la cola de Blocked para que sea atendido
	pthread_mutex_lock(&(colaBlocked->mutex));
    list_add(colaBlocked->lista, PCB);
    pthread_mutex_unlock(&(colaBlocked->mutex));

	//Notifico que hay un proceso en blocked esperando a ser atendido
	sem_post(&semPCBBlocked);
}

//Funcion pasar a ready si el grado de multiprogramacion lo permite
t_PCB* obtenerPCBAReady(t_lista* colaNew, t_log* logger)
{
	//Obtenemos el proceso de la cola de New
	pthread_mutex_lock(&(colaNew->mutex));
    t_PCB* PCBReady = list_remove(colaNew->lista, 0);
    pthread_mutex_unlock(&(colaNew->mutex));

	log_info(logger, "PID: %d - Estado Anterior: NEW - Estado actual: READY", PCBReady->PID);

	//Retornamos el PCB que ingresaremos en Ready
	return PCBReady;
}

//Funcion que una vez obtenido el PCB lo encola en Ready
void agregarPCBAReady(t_log* logger, t_lista* colaGenerica, t_PCB* PCB)
{
	//Pusheamos el PCB a la cola Ready
	pthread_mutex_lock(&(colaGenerica->mutex));
    list_add(colaGenerica->lista, PCB);
	mostrarPIDS(logger, colaGenerica->lista);
    pthread_mutex_unlock(&(colaGenerica->mutex));

	//Hay un proceso nuevo en Ready
	sem_post(&semPCBReady);
}

void mostrarPIDS(t_log* logger, t_list* cola)
{
	t_PCB* PCB;

	for(int i = 0; i < list_size(cola); i++)
	{
		PCB = list_get(cola, i);
		log_info(logger, "PROCESO %d", PCB->PID);
	}
}

//------------HILO DE READY------------

//Funcion para crear el hilo para pasar a ready
void crearHiloPasarAReady(argsPlanificacion* args)
{
	//Creo el hilo para pasar a Ready
	pthread_t* hilo = malloc(sizeof(pthread_t));

	//Creo los argumentos para crear el hilo para pasar a ready
	argsHiloReady* argumentos = malloc(sizeof(argsHiloReady));
	argumentos->logger = args->logger;
	argumentos->colaNew = args->colaNew;
	argumentos->colaReady = args->colaReady;

    pthread_create(hilo, NULL, hiloPasarAReady, argumentos);
    pthread_detach(*hilo);

	free(hilo);
}

//Funcion del Hilo para pasar a Ready
void* hiloPasarAReady(void* argumentos)
{
	//Cargo los valores del void* a una estructura
	argsHiloReady* args = (argsHiloReady*)argumentos;

	while(1)
	{
		//Verifica si hay algun proceso en la cola new y si pueden agregarse procesos a ready
		sem_wait(&semPCBNew);
		sem_wait(&semPCBMultiProgramacion);

		verificarPlanificacionActiva();

		t_PCB* PCB = obtenerPCBAReady(args->colaNew, args->logger);

		log_info(args->logger, "Cola Ready:");

    	agregarPCBAReady(args->logger, args->colaReady, PCB);
	}

	free(args);

	return NULL;
}

//------------PLANIFICADORES------------

//Planificador FIFO
int algoritmoFIFO(argsPlanificacion* args, t_lista* colaGenerica)
{
	verificarPlanificacionActiva();

	//Obtengo el primer PCB de Ready, uso mutex para la cola
	pthread_mutex_lock(&(colaGenerica->mutex));
    t_PCB* PCB = list_remove(colaGenerica->lista, 0);
    pthread_mutex_unlock(&(colaGenerica->mutex));

	//Envio el PCB a ejecutarse por Dispatch
	enviarPCB(args->conexiones.conexionDispatch, *PCB);

	log_info(args->logger, "PID: %d - Estado Anterior: READY - Estado actual: EXECUTE", PCB->PID);

	//Guardamos cual es el proceso en ejecuccion por si hay que finalizarlo, y para mostrar que se esta ejecutando
	*(args->PIDExecuting) = PCB->PID;

	int quantum = PCB->quantum;

	free(PCB);

	//Retorno el quantum, utilizado para RR y VRR
	return quantum;
}

//Planificador FIFO
void planificadorFIFO(argsPlanificacion* args)
{
	algoritmoFIFO(args, args->colaReady);

	//Espero a recibir el PCB por dispatch
	recibirContextoDelPCB(args);	
	sem_post(&semPCBExecuting);
}

//Planificador RR - Hay que hacer que reciba el PCB de CPU
void planificadorRR(argsPlanificacion* args)
{
	//Fijo el valor del quantum (paso de milisegundos a microsegundos para el usleep)
	int quantum = algoritmoFIFO(args, args->colaReady) * 1000;

	//Creo el hilo para que corra el tiempo del quantum, debe devolver el hilo para cancelarlo
	argsQuantum* argsHiloQuantum = crearHiloQuantum(quantum, args->conexiones.conexionInterrupt, args->logger);

	//Espero a recibir el PCB por dispatch, cancela el hilo si ya se interrumpio la ejecucion por otro motivo
	recibirContextoDelPCB(args);
	
	if(argsHiloQuantum->estado == 0)
	{
		pthread_cancel(*(argsHiloQuantum->hilo));
	}

	free(argsHiloQuantum->hilo);
	free(argsHiloQuantum);
	sem_post(&semPCBExecuting);
}

//Planificador VRR
void planificadorVRR(argsPlanificacion* args)
{
	int quantum;

	//Verifico si hay elementos en la cola con prioridad
	if(!list_is_empty(args->colaPrioritaria->lista))
	{
		//Fijo el valor del quantum (paso de milisegundos a microsegundos para el usleep)
		quantum = algoritmoFIFO(args, args->colaPrioritaria) * 1000;
	}

	else
	{
		//Fijo el valor del quantum (paso de milisegundos a microsegundos para el usleep)
		quantum = algoritmoFIFO(args, args->colaReady) * 1000;
	}

	//Creo el hilo para que corra el tiempo del quantum, debe devolver el hilo para cancelarlo
	argsQuantum* argsHiloQuantum = crearHiloQuantum(quantum, args->conexiones.conexionInterrupt, args->logger);

	//Espero a recibir el PCB por dispatch, cancela el hilo si ya se interrumpio la ejecucion por otro motivo
	recibirContextoDelPCB(args);
	
	if(argsHiloQuantum->estado == 0)
	{
		pthread_cancel(*(argsHiloQuantum->hilo));
	}

	free(argsHiloQuantum->hilo);
	free(argsHiloQuantum);
	sem_post(&semPCBExecuting);
}

//------------FUNCIONES HILO DEL QUANTUM------------

//Creo el hilo quantum y lo devuelvo para poder cancelarlo
argsQuantum* crearHiloQuantum(int quantum, int conexionInterrupt, t_log* logger)
{
	pthread_t* hilo = malloc(sizeof(pthread_t));

	argsQuantum* args = malloc(sizeof(argsQuantum));
	args->estado = 0;
	args->hilo = hilo;
	args->logger = logger;
	args->quantum = quantum;
	args->conexion = conexionInterrupt;

	pthread_create(hilo, NULL, hiloQuantum, args);
    pthread_detach(*hilo);

	return args;
}

//Funcion del hilo del quantum, hace un sleep para contar el tiempo y envia la interrupcion
void* hiloQuantum(void* argumentos)
{
	//Cargo los valores del void* a una estructura
	argsQuantum* args = (argsQuantum*)argumentos;

	//Corre el tiempo en microsegundos
	usleep(args->quantum);

	//Envio a CPU por interrupt que finalizo el Quantum
	enviarInterrupcionKernelCPU(INTERRUPCION_QUANTUM, args->conexion);	

	args->estado = 1;

	return NULL;
}

//Cuando es necesario envia una interrupcion desde Kernel hacia CPU (Es generica)
void enviarInterrupcionKernelCPU(op_code codigoInterrupcion, int conexionInterrupt)
{
	send(conexionInterrupt, &codigoInterrupcion, sizeof(op_code), 0);
}

//------------FUNCIONES PARA RECIBIR CONTEXTO DE EJECUCION------------

//Cuando envio el PCB a CPU me quedo esperando a que lo devuelva (no puede ejecutar dos procesos a la vez)
void recibirContextoDelPCB(argsPlanificacion* args)
{
	//Creo el temporal para contar el tiempo
	t_temporal* tiempoEjecucion = temporal_create();

	void* stream = malloc(sizeof(t_PCB));
	t_PCB* PCB = malloc(sizeof(t_PCB));
	op_code motivoDesalojo;

	//Recibo el motivo del desalojo y el PCB
	recv(args->conexiones.conexionDispatch, &motivoDesalojo, sizeof(op_code), MSG_WAITALL);

	temporal_stop(tiempoEjecucion);

	recv(args->conexiones.conexionDispatch, stream, sizeof(t_PCB), MSG_WAITALL);
	deserializarPCB(stream, PCB);

	verificarPlanificacionActiva();

	motivoDesalojo = checkFinalizacion(args, motivoDesalojo);

	//Me fijo que hacer segun el motivo de desalojo
	switch(motivoDesalojo)
	{
		case FIN_DE_QUANTUM:
			//Si finalizo el Quantum lo vuelvo a ingresar en Ready
			PCB->quantum = args->valores.quantum;
			log_info(args->logger, "PID: %d - Desalojado por fin de Quantum", PCB->PID);
			log_info(args->logger, "PID: %d - Estado Anterior: EXECUTE - Estado actual: READY", PCB->PID);

			log_info(args->logger, "Cola Ready:");
			agregarPCBAReady(args->logger, args->colaReady, PCB);
			break;

		case PROCESO_FINALIZADO:
			//Si finalizo el proceso, puedo cargar un nuevo proceso en Ready
			log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: SUCCESS", PCB->PID);
			finalizarProceso(args, PCB);
			break;

		case SOLICITUD_INTERFAZ:
			PCB->quantum = args->valores.quantum;
			recepcionInterfaz(args, PCB, tiempoEjecucion);
			break;

		case RECURSOS:
			recepcionRecursos(args, PCB, tiempoEjecucion);
			break;

		case PROCESO_FINALIZADO_MANUAL:
			//Si un proceso fue finalizado mientras se estaba ejecutando, debo aclarar que fue desalojado por Kernel
			log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: INTERRUPTED_BY_USER", PCB->PID);
			finalizarProceso(args, PCB);
			break;

		case OUT_OF_MEMORY:
			//Si nos quedamos sin memoria
			log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: OUT_OF_MEMORY", PCB->PID);
			finalizarProceso(args, PCB);
			break;

		default:
			break;
	}

	//No se esta ejecutando ningun proceso
	*(args->PIDExecuting) = -1;

	//Libero los recursos
	temporal_destroy(tiempoEjecucion);
	free(stream);
}

op_code checkFinalizacion(argsPlanificacion* args, op_code motivoDesalojo)
{
	if(*(args->PIDExecuting) == -1)
	{
		return PROCESO_FINALIZADO_MANUAL;
	}

	return motivoDesalojo;
}

//Finalizo el proceso que me llega
void finalizarProceso(argsPlanificacion* args, t_PCB* PCB)
{
	//Libero todos los recursos asociados a un proceso
	liberarTodososRecursos(PCB, args->listaRecursos);

	//Verifico que al liberar un proceso, este no interfiera con el grado de multiprogramacion (si este disminuyo en algun momento)
	if(args->overFlowProgramacion == 0)
	{
		//Al finalizar el proceso, si no se disminuyo el grado de multiprogramacion se permite que un proceso mas ingrese a la cola Ready
		sem_post(&semPCBMultiProgramacion);
	}

	else
	{
		//Si se disminuyo el grado de multiprogramacion, resto 1 a la diferencia de grado
		args->overFlowProgramacion -= 1;
	}

	//Le aviso a memoria para que desaloje los recursos
	avisoFinalizacion(args->conexiones.conexionMemoria, PCB->PID);

	//Libero el PCB que se finalizo
	free(PCB);
}

void avisoFinalizacion(int socket, uint32_t PID)
{
	void* stream = malloc(sizeof(op_code) + sizeof(uint32_t));

	op_code cop = AVISO_FINALIZACION;

	memcpy(stream, &cop, sizeof(op_code));
	memcpy(stream + sizeof(op_code), &PID, sizeof(uint32_t));

	send(socket, stream, sizeof(op_code) + sizeof(uint32_t), 0);

	free(stream);
}

//Me fijo si detuve la planificacion o no, si esta detenida se queda esperando
void verificarPlanificacionActiva()
{
	sem_wait(&semPlanificador);
	sem_post(&semPlanificador);
}

//------------FUNCIONES DE INTERFACES ENTRADA/SALIDA------------

//Me llega un proceso desde CPU que requiere de una interfaz IO
void recepcionInterfaz(argsPlanificacion* args, t_PCB* PCB, t_temporal* tiempoEjecucion)
{
	//Obtengo el quantum restante
	int quantumVariable = PCB->quantum - temporal_gettime(tiempoEjecucion);

	//Si el quantum variable no es 0 o menor (ya se paso del tiempo) y uso VRR, le asigno Q'
	if(quantumVariable > 0 && !strcmp(args->valores.algoritmoPlanificacion, "VRR"))
	{
		PCB->quantum = quantumVariable;
	}

	//Tengo que recibir el nombre y tipo para ver si es valida
	recepcionTipoYNombre(args, PCB);
}

//Recibo el tipo de interfaz y el nombre de la interfaz a utilizar
void recepcionTipoYNombre(argsPlanificacion* args, t_PCB* PCB)
{
	tipo_interfaz interfazRecibida;
	int longitud;

	//Recibo primero el tipo de interfaz y despues la longitud del nombre
	recv(args->conexiones.conexionDispatch, &interfazRecibida, sizeof(tipo_interfaz), MSG_WAITALL);
	recv(args->conexiones.conexionDispatch, &longitud, sizeof(int), MSG_WAITALL);

	//Ahora con la longitud obtengo el nombre
	char* nombreInterfaz = malloc(longitud);
	recv(args->conexiones.conexionDispatch, nombreInterfaz, longitud, MSG_WAITALL);

	//Verifico que se encuentre conectada la interfaz
	tipo_interfaz interfazRequerida = buscarInterfaz(args, nombreInterfaz);

	if(interfazRequerida == interfazRecibida)
	{
		log_info(args->logger, "PID: %d - Bloqueado por: <%s>", PCB->PID, nombreInterfaz);

		//Si un proceso necesita un dispostivo I/O lo cargo en blocked
		encolarBlocked(PCB, args->colaBlocked, args->logger);

		//Recibo los operandos
		t_operandosKernel* operandos = obtenerInstruccionInterfaz(args, interfazRequerida, nombreInterfaz);

		//Una vez que tengo los operandos creo el hilo para atender a la interfaz
		crearHiloInterfaz(args, operandos, interfazRequerida, PCB);
	}

	else
	{
		//Finalizamos el proceso ya que no se encuentra conectado o no es una operacion valida
		log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: INVALID_INTERFACE", PCB->PID);
		finalizarProceso(args, PCB);
	}
}

//Funcion para crear el hilo cuando es necesario hacer una funcion de entrada/salida
void crearHiloInterfaz(argsPlanificacion* args, t_operandosKernel* operandos, tipo_interfaz interfaz, t_PCB* PCB)
{
	argsAtencionIO* argsIO = malloc(sizeof(argsAtencionIO));
	argsIO->logger = args->logger;
	argsIO->colaBlocked = args->colaBlocked;
	argsIO->colaReady = args->colaReady;
	argsIO->colaPrioritaria = args->colaPrioritaria;
	argsIO->algoritmo = args->valores.algoritmoPlanificacion;
	argsIO->PID = PCB->PID;
	argsIO->operandos = operandos;
	argsIO->quantum = args->valores.quantum;
	argsIO->listaInterfaz = devolverColaInterfaz(args, interfaz);

	pthread_t* hilo = malloc(sizeof(pthread_t));

	pthread_create(hilo, NULL, hiloInterfaz, argsIO);
    pthread_detach(*hilo);

	free(hilo);
}

//En este hilo se hace la instruccion solicitada desde CPU
void* hiloInterfaz(void* argumentos)
{
	argsAtencionIO* args = (argsAtencionIO*)argumentos;

	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(args->operandos->operando1, args->listaInterfaz);

	//Si un proceso esta usando esa interfaz no esta disponible para otro
	pthread_mutex_lock(&(interfaz->mutex));

	if(verificarProcesoFinalizado(args->colaBlocked, args->PID))
	{
		//Realizo la instruccion necesaria
		realizarInstruccionIO(args, interfaz);

		//Verifico si la planificacion esta activa, de estarlo verifico otra vez que no se haya eliminado el proceso
		verificarPlanificacionActiva();

		if(verificarProcesoFinalizado(args->colaBlocked, args->PID))
		{
			//Si el proceso no fue finalizado, lo quito de Blocked y lo devuelvo a Ready
			t_PCB* PCB = obtenerPCBPorPID(args->colaBlocked, args->PID);

			log_info(args->logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", PCB->PID);
			if(strcmp(args->algoritmo, "VRR") == 0 && PCB->quantum != args->quantum)
			{
				log_info(args->logger, "Ready Prioridad:");
				agregarPCBAReady(args->logger, args->colaPrioritaria, PCB);
			}

			else
			{
				log_info(args->logger, "Cola Ready:");
				agregarPCBAReady(args->logger, args->colaReady, PCB);
			}
		}
	}

	pthread_mutex_unlock(&(interfaz->mutex));
	free(args->operandos);
	free(args);

	return NULL;
}

bool verificarProcesoFinalizado(t_lista* colaBlocked, int PID)
{
	bool existeElPID(void* elemento)
	{
		t_PCB* PCB = (t_PCB*)elemento;
		return (PCB->PID == PID);
	};

	//Usamos un mutex para ver si algun elemento de la lista tiene ese PCB y devolvemos true si lo es
	pthread_mutex_lock(&(colaBlocked->mutex));
	bool encontrado = list_any_satisfy(colaBlocked->lista, (void*)existeElPID);
	pthread_mutex_unlock(&(colaBlocked->mutex));

	return encontrado;
}

t_PCB* obtenerPCBPorPID(t_lista* listaGenerica, int PID)
{
	bool existeElPID(void* elemento)
	{
		t_PCB* PCB = (t_PCB*)elemento;
		return (PCB->PID == PID);
	};

	//Usamos un mutex para ver si algun elemento de la lista tiene ese PCB y devolvemos true si lo es
	pthread_mutex_lock(&(listaGenerica->mutex));
	t_PCB* PCB = list_remove_by_condition(listaGenerica->lista, (void*)existeElPID);
	pthread_mutex_unlock(&(listaGenerica->mutex));

	return PCB;
}

void realizarInstruccionIO(argsAtencionIO* args, t_elemento_interfaces* interfaz)
{
	//Creo una variable peticion para poder llamar a las funciones
	t_peticion_std peticion;
	
	//Me fijo que tipo de instruccion tengo que manejar
	switch(args->operandos->instruccion)
	{
		case SLEEP:
			int unidades = atoi(args->operandos->operando2Nombre);
			enviarSleep(args->logger, args->listaInterfaz, interfaz->nombre, args->PID, unidades);
			break;

		case READ:
			peticion.PID = args->PID;
			peticion.tamanio = args->operandos->operando3;
			peticion.direcFisica = args->operandos->operando2;

			peticionSTDIN(args->logger, args->listaInterfaz, interfaz->nombre, peticion);
			break;

		case WRITE:
			peticion.PID = args->PID;
			peticion.tamanio = args->operandos->operando3;
			peticion.direcFisica = args->operandos->operando2;

			peticionSTDOUT(args->logger, args->listaInterfaz, interfaz->nombre, peticion);
			break;

		case FS_CREATE:
			enviarFS_CREATE(args->logger, args->listaInterfaz, interfaz->nombre, args->operandos->operando2Nombre, args->PID);
			break;

		case FS_DELETE:
			enviarFS_DELETE(args->logger, args->listaInterfaz, interfaz->nombre, args->operandos->operando2Nombre, args->PID);
			break;

		case FS_TRUNCATE:
			enviarFS_TRUNCATE(args->logger, args->listaInterfaz, interfaz->nombre, args->operandos->operando2Nombre, args->operandos->operando3, args->PID);
			break;

		case FS_WRITE:
			enviarFS_WRITE(args->logger, args->listaInterfaz, interfaz->nombre, args->operandos->operando2Nombre, args->PID, args->operandos->operando3, args->operandos->operando4, args->operandos->operando5);
			break;

		case FS_READ:
			enviarFS_READ(args->logger, args->listaInterfaz, interfaz->nombre, args->operandos->operando2Nombre, args->PID, args->operandos->operando3, args->operandos->operando4, args->operandos->operando5);
			break;

		default:
			log_info(args->logger, "La instruccion solicitada es invalida");
			break;
	}
}

//Dada una interfaz, devuelve la lista de dicha interfaz
t_lista* devolverColaInterfaz(argsPlanificacion* args, tipo_interfaz interfaz)
{
	switch(interfaz)
	{
		case GENERICA:
			return args->interfacesGenericas;
			break;

		case STDIN:
			return args->interfacesSTDIN;
			break;

		case STDOUT:
			return args->interfacesSTDOUT;
			break;

		case DIALFS:
			return args->interfacesDIALFS;
			break;

		default:
			return NULL;	
			break;
	}
}

//Verifico si la Interfaz de ese nombre se encuentra conectada y devuelvo el tipo de esta
tipo_interfaz buscarInterfaz(argsPlanificacion* args, char* nombreInterfaz)
{     
	//Voy recorriendo cada lista y retorno el tipo dependiendo de la lista
	if(existeEnLista (args->interfacesGenericas, nombreInterfaz))
	{
		return GENERICA;
	}

	else if(existeEnLista (args->interfacesSTDIN, nombreInterfaz))
	{
		return STDIN;
	}

	else if(existeEnLista (args->interfacesSTDOUT, nombreInterfaz))
	{
		return STDOUT;
	}
	
	else if(existeEnLista (args->interfacesDIALFS, nombreInterfaz))
	{
		return DIALFS;
	}

	log_info(args->logger, "La interfaz solicitada no se encuentra conectada");

	//Si no se encuentra en ninguna lo informamos
	return ERROR;
}

//Funcion para ver si una interfaz existe en una lista
bool existeEnLista(t_lista* listaGenerica, char* nombreInterfaz)
{
	bool existeElNombre(void* interfaz)      
	{         
		return (!strcmp(((t_elemento_interfaces*)interfaz)->nombre, nombreInterfaz));
	}

	//Usamos un mutex para ver si algun elemento de la lista tiene ese nombre y devolvemos true si lo es
	pthread_mutex_lock(&(listaGenerica->mutex));
	bool encontrado = list_any_satisfy(listaGenerica->lista, (void*)existeElNombre);
	pthread_mutex_unlock(&(listaGenerica->mutex));

	return encontrado;
}

//Funcion que a partir de una interfaz devuelve la instruccion que va a ejecutar
t_operandosKernel* obtenerInstruccionInterfaz(argsPlanificacion* args, tipo_interfaz interfaz, char* nombreInterfaz)
{
	t_operandosKernel* operandos = malloc(sizeof(t_operandosKernel));
	
	//El primer operando siempre es el nombre la interfaz
	operandos->operando1 = nombreInterfaz;

	switch(interfaz)
	{
		case GENERICA:
			operandos->instruccion = SLEEP;
			break;

		case STDIN:
			operandos->instruccion = READ;
			break;

		case STDOUT:
			operandos->instruccion = WRITE;
			break;

		case DIALFS:

			//A diferencia de las otras interfaces, DIALFS tiene mas de una instruccion posible, debemos obtenerla tambien
			operandos->instruccion = recibirOperacion(args->conexiones.conexionDispatch);

			break;

		default:
			break;
	}

	//Una vez que ya sabemos la instruccion, ya sabemos cuantos operandos require cada una
	operandos = recibirOperandosRestantes(args, operandos);

	return operandos;
}

//Una vez que tenemos la instruccion, dependiendo de cual sea devolvemos los operandos que le correspondan
t_operandosKernel* recibirOperandosRestantes(argsPlanificacion* args, t_operandosKernel* operandos)
{
	int longitud;

	//Recibo una cantidad determinada de argumentos segun la instruccion
	if(operandos->instruccion == SLEEP || operandos->instruccion == FS_CREATE || operandos->instruccion == FS_DELETE)
	{
		//Recibo un solo operando mas
		recv(args->conexiones.conexionDispatch, &longitud, sizeof(int), MSG_WAITALL);
		operandos->operando2Nombre = malloc(longitud);
		recv(args->conexiones.conexionDispatch, operandos->operando2Nombre, longitud, MSG_WAITALL);
	}

	if(operandos->instruccion == READ || operandos->instruccion == WRITE)
	{
		//Recibo dos operandos mas
		recv(args->conexiones.conexionDispatch, &(operandos->operando2), sizeof(u_int32_t), MSG_WAITALL);
		recv(args->conexiones.conexionDispatch, &(operandos->operando3), sizeof(u_int32_t), MSG_WAITALL);
	}
	
	if(operandos->instruccion == FS_TRUNCATE)
	{
		//Recibo dos operandos mas
		recv(args->conexiones.conexionDispatch, &longitud, sizeof(int), MSG_WAITALL);
		operandos->operando2Nombre = malloc(longitud);
		recv(args->conexiones.conexionDispatch, operandos->operando2Nombre, longitud, MSG_WAITALL);

		recv(args->conexiones.conexionDispatch, &(operandos->operando3), sizeof(u_int32_t), MSG_WAITALL);
	}

	if(operandos->instruccion == FS_WRITE || operandos->instruccion == FS_READ)
	{
		//Recibo la longitud del nombre del archivo
		recv(args->conexiones.conexionDispatch, &longitud, sizeof(int), MSG_WAITALL);

		//Recibo el nombre del archivo
		operandos->operando2Nombre = malloc(longitud);
		recv(args->conexiones.conexionDispatch, operandos->operando2Nombre, longitud, MSG_WAITALL);

		//Recibo direccion logica, tamanio a leer y puntero del archivo
		recv(args->conexiones.conexionDispatch, &(operandos->operando3), sizeof(u_int32_t), MSG_WAITALL);
		recv(args->conexiones.conexionDispatch, &(operandos->operando4), sizeof(u_int32_t), MSG_WAITALL);
		recv(args->conexiones.conexionDispatch, &(operandos->operando5), sizeof(u_int32_t), MSG_WAITALL);
	}

	return operandos;
}

//------------FUNCIONES PARA MANEJO DE RECURSOS------------

//Funcion para buscar instancias de un recurso mediante su PID
int* buscarInstanciaPID(int PIDBuscado, t_list* listaPID)
{
	bool existeElPID(void* instancia) 
	{
		return (*(int*)instancia == PIDBuscado);
	}
	
	int* instancia = list_find(listaPID, (void*)existeElPID);

	return instancia;
}

//Funcion para recibir la peticion/liberacion de un recurso
void recepcionRecursos(argsPlanificacion* args, t_PCB* PCB, t_temporal* tiempoEjecucion)
{
	void* stream = malloc(sizeof(tipoInstruccionesRecursos) + sizeof(int));
	tipoInstruccionesRecursos instruccion;
	int longitud;

	//Recibo primero el tipo de instruccion y despues la longitud del nombre del recurso
	recv(args->conexiones.conexionDispatch, stream, sizeof(tipoInstruccionesRecursos) + sizeof(int), MSG_WAITALL);

	memcpy(&instruccion, stream, sizeof(tipoInstruccionesRecursos));
	memcpy(&longitud, stream + sizeof(tipoInstruccionesRecursos), sizeof(int));

	char* nombreRecurso = malloc(longitud);

	recv(args->conexiones.conexionDispatch, nombreRecurso, longitud, MSG_WAITALL);

	t_recursos* recurso = buscarRecurso(nombreRecurso, args->listaRecursos);

	if(recurso == NULL)
	{
		log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: INVALID_RESOURCE", PCB->PID);
		finalizarProceso(args, PCB);
	}

	else
	{
		argsManejoRecursos* argsRecursos = malloc(sizeof(argsManejoRecursos));
		argsRecursos->proceso = PCB;
		argsRecursos->recurso = recurso;
		argsRecursos->logger = args->logger;
		argsRecursos->blocked = args->colaBlocked;
		argsRecursos->listaPlanificador = args->colaReady;
		argsRecursos->listaRecursos = args->listaRecursos;

		if(instruccion == INTERRUPT_WAIT)
		{
			pedirRecurso(argsRecursos, tiempoEjecucion);
		}

		else if(instruccion == INTERRUPT_SIGNAL)
		{
			liberarRecurso(argsRecursos, false, tiempoEjecucion);
		}
	}	
}

void agregarPrimeroEnReady(argsManejoRecursos* args)
{
	//Pusheamos el PCB a la cola Ready
	pthread_mutex_lock(&(args->listaPlanificador->mutex));
	list_add_in_index(args->listaPlanificador->lista, 0, args->proceso);
	pthread_mutex_unlock(&(args->listaPlanificador->mutex));

	//Hay un proceso nuevo en Ready
	sem_post(&semPCBReady);
}

//Funcion para pedir recursos
void pedirRecurso(argsManejoRecursos* args, t_temporal* tiempoEjecucion)
{
	//Me fijo si hay instancias disponibles del recurso en cuestion
	if(!sem_trywait(&(args->recurso->disponible)))
	{
		//Luego de pasar el wait (hay al menos una instancia disponible) busca esta instancia disponible y le pone su PID para indicar que el la tiene ahora
		pthread_mutex_lock(&(args->listaRecursos->mutex));
		int* instancia = buscarInstanciaPID(-1, args->recurso->PID);
		*instancia = args->proceso->PID;
		pthread_mutex_unlock(&(args->listaRecursos->mutex));

		//Agrego el PCB primero en la lista de ready
		args->proceso->quantum -= temporal_gettime(tiempoEjecucion);
		agregarPrimeroEnReady(args);
	}

	else
	{
		pthread_t* hilo = malloc(sizeof(pthread_t));
		pthread_create(hilo, NULL, pedirRecursoHilo, args);
		pthread_detach(*hilo);
	}
}

//Funcion para pedir recursos
void liberarRecurso(argsManejoRecursos* args, bool finalizoProceso, t_temporal* tiempoEjecucion)
{
	//printf("EL PROCESO \"%d\" VA A LIBERAR EL RECURSO \"%s\"", args->proceso, args->recurso);

	pthread_mutex_lock(&(args->listaRecursos->mutex));

	//Busco instancias del recurso en cuestion asociadas al proceso
	int* instancia = buscarInstanciaPID(args->proceso->PID, args->recurso->PID);

	//Si se encuentra instancia del recurso en cuestion asociada al proceso, la libero
	if(instancia != NULL)
	{
		//Pongo en -1 la instancia que libere
		*instancia = -1;

		//Libero efectivamente la instancia
		sem_post(&(args->recurso->disponible));
	}
	
	else
	{
		log_info(args->logger, "El proceso no posee instancias de este recurso");
	}

	if(!finalizoProceso)
	{
		//Agrego el PCB primero en la lista de Ready
		args->proceso->quantum -= temporal_gettime(tiempoEjecucion);
		agregarPrimeroEnReady(args);
	}

	pthread_mutex_unlock(&(args->listaRecursos->mutex));
}

//Funcion para liberar todos los recursos cuando un proceso va a exit
void liberarTodososRecursos(t_PCB* proceso, t_lista* listaRecursos)
{
	t_recursos* recurso;

	do
	{
		recurso = tieneAlgunRecurso(proceso, listaRecursos);

		if(recurso != NULL)
		{
			argsManejoRecursos* args = malloc(sizeof(argsManejoRecursos));
			args->recurso = recurso;
			args->proceso = proceso;
			args->listaRecursos = listaRecursos;

			liberarRecurso(args, true, NULL);

			free(args);
		}

	}while(recurso != NULL);
}

//Funcion para pedir recursos
void* pedirRecursoHilo(void* argumentos)
{
	argsManejoRecursos* args = (argsManejoRecursos*)argumentos;

	log_info(args->logger, "PID: %d - Bloqueado por %s", args->proceso->PID, args->recurso->nombre);
	log_info(args->logger, "PID: %d - Estado Anterior: EXECUTE - Estado actual: BLOCKED", args->proceso->PID);
	encolarBlocked(args->proceso, args->blocked, args->logger);

	//Si encuentra la instancia hace un wait del recurso
	sem_wait(&(args->recurso->disponible));

	if(verificarProcesoFinalizado(args->blocked, args->proceso->PID))
	{
		//Luego de pasar el wait (hay al menos una instancia disponible) busca esta instancia disponible y le pone su PID para indicar que el la tiene ahora
		pthread_mutex_lock(&(args->listaRecursos->mutex));
		
		int* instancia = buscarInstanciaPID(-1, args->recurso->PID);
		*instancia = args->proceso->PID;

		pthread_mutex_unlock(&(args->listaRecursos->mutex));

		//Si el proceso no fue finalizado, lo quito de Blocked y lo devuelvo a Ready
		if(obtenerPCBPorPID(args->blocked, args->proceso->PID) != NULL)
		{
			log_info(args->logger, "PID: %d - Estado Anterior: BLOCKED - Estado actual: READY", args->proceso->PID);
			agregarPCBAReady(args->logger, args->listaPlanificador, args->proceso);
		}
	}

	else
	{
		log_info(args->logger, "El proceso fue finalizado anteriormente");
		sem_post(&(args->recurso->disponible));
	}

	return NULL;
}

//Funcion para buscar un recurso
t_recursos* buscarRecurso(char* nombre, t_lista* listaRecursos)
{
	t_recursos* recurso = buscarRecursoPorNombre(nombre, listaRecursos);

	return recurso;
}

//Funcion para verificar sin un proceso tiene algun recurso asignado
t_recursos* tieneAlgunRecurso(t_PCB* proceso, t_lista* listaRecursos)
{
	bool tieneInstancias(void* recurso) 
	{
		//Si el proceso tiene alguna instancia con su nombre devuelvo true
		if(buscarInstanciaPID(proceso->PID, ((t_recursos*)recurso)->PID) != NULL)
		{
			return true;
		}

		//Si el proceso no posee instancias asignadas devuelvo false
		return false;
	}

	pthread_mutex_lock(&(listaRecursos->mutex));
	t_recursos* recurso = list_find(listaRecursos->lista, (void*)tieneInstancias);
	pthread_mutex_unlock(&(listaRecursos->mutex));

	return recurso;
}

//Funcion para buscar un recurso por nombre
t_recursos* buscarRecursoPorNombre(char* nombreRecurso, t_lista* listaRecursos)
{
	bool existeElNombre(void* interfaz) 
	{
		return (!strcmp(((t_recursos*)interfaz)->nombre, nombreRecurso));
	}

	pthread_mutex_lock(&(listaRecursos->mutex));
	t_recursos* recurso = list_find(listaRecursos->lista, (void*)existeElNombre);
	pthread_mutex_unlock(&(listaRecursos->mutex));

	return recurso;
}