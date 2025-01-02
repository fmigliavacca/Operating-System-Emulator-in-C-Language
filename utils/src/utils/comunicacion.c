#include <../include/comunicacion.h>

//Funcion para enviar un mensaje
void enviarMensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializarPaquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminarPaquete(paquete);
}

//Funcion para enviar texto de STDIN y direccion fisica
void enviarInterfazStdin(int socket_cliente, t_peticion_std dirFisica)
{
	void* stream = malloc(sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t) + dirFisica.tamanio);
	
	op_code cop = TEXTO_STDIN;

	memcpy(stream,&cop,sizeof(int));
	memcpy(stream + sizeof(int), &dirFisica.PID, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t), &dirFisica.direcFisica, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 2, &dirFisica.tamanio, sizeof(uint8_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t), dirFisica.cadena, dirFisica.tamanio);

	send(socket_cliente, stream, sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t) + dirFisica.tamanio, 0);

	free(stream);
}

//Funcion para recibir una operacion
int recibirOperacion(int socket)
{
	int operacion;

	if(recv(socket, &operacion, sizeof(int), MSG_WAITALL) > 0)
	{
		return operacion;
	}

	else
	{
		close(socket);
		return -1;
	}
}

//Funcion para recibir un buffer
void* recibirBuffer(int* size, int socket_cliente)
{
	void* buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

//Funcion para recibir un mensaje
void recibirMensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibirBuffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

void* esperarMensajesMemoria(void* argumentos)
{
	bool flag = true;

	while(flag)
	{
		int* retorno = esperarMensajes(argumentos);
		
		if(retorno != NULL)
		{
			flag = false;
		}
	}

	return NULL;
}

//Funcion para elegir que hacer con el stream segun el codOp
void* esperarMensajes(void* argumentos)
{
	argsEsperarMensaje* args = (argsEsperarMensaje*)argumentos;

	//Recibo el codigo de operacion
	args->codOp = recibirOperacion(args->cliente);

	//Segun el codigo de operacion veo como proceso lo recibido
	switch(args->codOp){
		
		//GENERICAS
		case MENSAJE:
			recibirMensaje(args->cliente, args->logger);
			break;
		case PCB:
			sem_wait(&(args->mutexCpu));
			recibirPCB(args->cliente, args);
			sem_post(&(args->semRecibirPCB));
			break;

		//INTERFACES
		case INTERFAZ:
			recibirInterfaz(args);
			break;

		//KERNEL
		case INTERRUPCION_QUANTUM:	
			manejarInterrupcionQuantum(args->cliente, args->pcb, args->interrupciones);
			break;
		case INTERRUPCION_FINALIZAR:
			log_info(args->logger, "FINALIZAR PROCESO");
			manejarInterrupcionProcesoFinalizado(args->cliente, args->pcb, args->interrupciones);	
			break;

		//MEMORIA
		case PATH_INSTRUCCIONES:
			usleep(((args->retardoRespuesta)*1000));
			recibirPath(args->logger, args->listaProcesos, args->cliente, args->path, args->directorio);
			break;
		case PETICION_INSTRUCCION:
			recibirPedidoInstruccion(args);
			break;
		case TEXTO_STDIN:
			usleep(((args->retardoRespuesta)*1000));
			recibirSTDIN(args);
			break;
		case CONTENIDO_STDOUT:
			usleep(((args->retardoRespuesta)*1000));
			recibirSTDOUT(args);
			break;
		case AVISO_FINALIZACION:
			usleep(((args->retardoRespuesta)*1000));
			finalizarProcesoEnMemoria(args);
			break;
		case PETICION_TAMANO_PAGINA:
			recibirPeticionTamanioPagina(args);
			break;
		case PETICION_MARCO:
			usleep(((args->retardoRespuesta)*1000));
			recibirPedidoMarco(args->cliente, args->directorio);
			break;
		case HACER_RESIZE:
			usleep(((args->retardoRespuesta)*1000));
			recibirResize(args->cliente, args->framesMemoria, args->directorio, args->logger, args->tamanioPagina);
			break;
		case PEDIDO_MOV_IN:
			usleep(((args->retardoRespuesta)*1000));
			accederAEspacioUsuario(0, args->cliente, args->directorio, args->logger, args->tamanioPagina, args->memoria);
			break;
		case PEDIDO_MOV_OUT:
			usleep(((args->retardoRespuesta)*1000));
			accederAEspacioUsuario(1, args->cliente, args->directorio, args->logger, args->tamanioPagina, args->memoria);
			break;
		case -1:
			log_info(args->logger, "Se perdio la conexion");
			free(args);
			return (int*)-1;
			break;

		//DEFAULT CASE
		default:
			log_info(args->logger, "Codigo de operacion invalido");
			return (int*)-1;
			break;
		}

		return NULL;
}

//Proceso a ejecutar cuando se recibe el pedido de enviar una instruccion a CPU
void recibirPedidoInstruccion(argsEsperarMensaje* args)
{
	int socket = args->cliente;
	t_list* listaProcesos = args->listaProcesos;
	int retardoRespuesta = args->retardoRespuesta;

	//Creo el stream donde voy a recibir el identificador del proceso
	void* stream = malloc(sizeof(uint32_t)*2);

	//Recibo el stream con el identificador del proceso
	if(recv(socket, stream, sizeof(sizeof(uint32_t)*2), 0) != sizeof(sizeof(uint32_t)*2)) 
	{
		free(stream);
		return;
	}

	//Deserializo la peticion para manejarla con facilidad
	t_peticionInstruccion peticion = deserializarPeticionInstruccion(stream);
	
	free(stream);

	//Espera la cantidad de tiempo asignada en el config (multiplico por 1000 ya que es ms)
	usleep(retardoRespuesta * 1000);

	//Realizo las acciones pertinentes para la busqueda y envio de la instruccion solicitada
	manejarPedidoInstruccion(peticion,listaProcesos,socket);
}

//Cuando kernel me dice que finalice un proceso
void finalizarProcesoEnMemoria(argsEsperarMensaje* args)
{
    int socket = args->cliente;
    t_list* directorio = args->directorio;
    t_list* listaProcesos = args->listaProcesos;
    t_bitarray* framesDisponibles = args->framesMemoria;

    //Recibo el PID que me pasa kernel del proceso a finalizar
    int PID, i = 0;

    recv(socket, &PID, sizeof(int), 0);

    //Para el directorio
    bool esIgualPID(void* tabla) 
	{
		bool aux = 0;
		if(PID == ((t_tabla_paginas*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	}

    //Para la lista de instrucciones
    bool esIgualPID2(void* tabla) 
	{
		bool aux = 0;
		if(PID == ((t_direccionProceso*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	}

    t_direccionProceso* auxProceso = list_find(listaProcesos, (void*)esIgualPID2);

    //Busco la tabla de paginas del proceso pedido
    t_tabla_paginas* auxTabla = list_find(directorio,(void*)esIgualPID);
    t_pagina* auxPagina;
    
    while(i<(list_size(auxTabla->paginas)))
    {
        auxPagina = list_get((auxTabla->paginas),i);
        bitarray_set_bit(framesDisponibles, (auxPagina->marco));
        i++;
    }

	log_info(args->logger, "PID: %d - Tamaño: %d", PID, list_size(auxTabla->paginas));

    //Libero recursos de la tabla de paginas
    list_destroy_and_destroy_elements(auxProceso->instrucciones, (void*)hacerFreeListaInstrucciones);
    list_remove_and_destroy_by_condition(listaProcesos, (void*)esIgualPID2, (void*)hacerFreeProceso);
    list_destroy_and_destroy_elements(auxTabla->paginas, (void*)hacerFreePaginas);
    list_remove_and_destroy_by_condition( directorio, (void*)esIgualPID, (void*)hacerFreeTablaPaginas);
}

void hacerFreeTablaPaginas(t_tabla_paginas* algo)
{
	free(algo);
};

void hacerFreePaginas(t_pagina* algo)
{
	free(algo);
}

void hacerFreeProceso(t_direccionProceso* dato)
{
	free(dato);
}

void hacerFreeListaInstrucciones(t_instruccion* dato)
{
	free(dato->instruccion);
	free(dato);
}

//Funcion para recibir PCBs
void recibirPCB(int socket, argsEsperarMensaje* args)
{
	t_PCB* pcb = args->pcb;

	//Creo el stream donde voy a recibir el PCB
	void* stream = malloc(sizeof(t_PCB));

	//Recibo la informacion serializada
	recv(socket, stream, sizeof(t_PCB), MSG_WAITALL);

	//Deserializo lo que recibi
	deserializarPCB(stream, pcb);

	*(args->interrupciones) = 1;

	//Libero el stream
	free(stream);
}

//Funcion para enviar PCBs
void enviarPCB(int socket, t_PCB pcb)
{
	//Creo un stream con el PCB serializado
	void* stream = serializarPCB(pcb);

	//Envio el stream ya cargado
	send(socket, stream, sizeof(op_code) + sizeof(t_PCB), 0);

	//Libero el stream
	free(stream);
}

//Enviar la instruccion a partir de una instruccion y un socket
void enviarInstruccion(t_instruccion instruction, int socket_cliente)
{	
	//Armo buffer
	t_buffer* buffer = malloc(sizeof(t_buffer));
	
	buffer->size = instruction.largoInstruccion;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream, instruction.instruccion, instruction.largoInstruccion);

	//Armo paquete
	t_paquete* paquete = malloc(sizeof(t_paquete));
	
	paquete->codigo_operacion = RECEPCION_INSTRUCCION;
	paquete->buffer = buffer;
	
	int bytes = buffer->size + (2*sizeof(int)); 

 	//Serializo paquete
	void* serializado = malloc(bytes);

	int desplazamiento = 0;

	memcpy(serializado, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(serializado + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(serializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	//Envio el paquete serializado
	send(socket_cliente, serializado, bytes, 0);
	
	//Libero los recursos
	eliminarPaquete(paquete);

	free(serializado);
}

// Envia una cadena de instrucciones a partir de una lista de instrucciones
void enviarCadenaDeInstrucciones(t_list* listaInstrucciones, int socket_cliente) 
{
	//Creo las variables necesarias
	int i = 0;
	t_instruccion* instruction;

	//Mientras la lista de instrucciones no este vacia
	while(listaInstrucciones != NULL)
	{
		//Leo las instrucciones desde la lista y las envio
		instruction = list_get(listaInstrucciones, i);
		enviarInstruccion(*instruction, socket_cliente);
		
		// Elimino el char* de cada nodo de memoria
		free(instruction->instruccion);
		
		// Elimino cada nodo de la lista
		free(instruction);
		i++;
	}

	// Elimino la lista
	list_destroy(listaInstrucciones);
}

//Funcion para eliminar un paquete
void eliminarPaquete(t_paquete* paquete)
{
	//Libero los recursos
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

//Funcion para eliminar un paquete stdin
void eliminarPaqueteStdin(t_paquete_stdin* paquete)
{
	//Libero los recursos
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// Le envia a Kernell la confirmación que el path fue encontrado y decodificado
void enviarConfirmacionPath(op_code codigo, int socket)
{
	void* stream= malloc(sizeof(uint32_t));
	memcpy(stream,&codigo,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),0);
	free(stream);
}

//Lee el archivo de pseudocódigo dado por el path y retorna las instrucciones en formato de lista
t_list* obtenerPseudoCodigo(t_log* logger, char* pathCodigo, char* pathCarpetaPseudo) 
{
	t_list* listaArchivoPseudo = list_create();

	char* pathFinal = malloc(strlen(pathCodigo) + strlen(pathCarpetaPseudo) + 1);

	strcpy(pathFinal, pathCarpetaPseudo);
	strcat(pathFinal, pathCodigo);

	//Abro el archivo solo para lectura
	FILE* archivoPseudo = fopen(pathFinal, "r");
    
	//Me fijo si el archivo existe
	if (archivoPseudo != NULL)
    {
        //Voy hacia el final para obtener la ultima posicion, que serian todos los caracteres
		fseek(archivoPseudo, 0, SEEK_END);
        int longitud = ftell(archivoPseudo);
        
		//Vuelvo al principio del archivo y hago un malloc para obtener todo el texto
		fseek(archivoPseudo, 0, SEEK_SET);
        char* textoArchivo = malloc(longitud + 1);

		//Uso fread para obtener todo el texto del archivo
        fread(textoArchivo, sizeof(char), longitud, archivoPseudo);
        
		//Le agrego un \0 al final, sino se bugea
		textoArchivo[longitud] = '\0';

		//Obtengo hasta el primer \n
        char* auxString = strtok(textoArchivo,"\n");

		//Obtengo todas las instrucciones hasta que sea NULL (llegue al final)
        while(auxString != NULL)
        {
			t_instruccion* auxInstruccion = malloc(sizeof(t_instruccion));

			auxInstruccion->instruccion = malloc(strlen(auxString) + 1);
			strcpy(auxInstruccion->instruccion, auxString);
			auxInstruccion->largoInstruccion = strlen(auxString) + 1;

			//Agrego la esctructura a la lista
			list_add(listaArchivoPseudo, auxInstruccion);

            auxString = strtok (NULL, "\n");
        }

        fclose(archivoPseudo);
        free(textoArchivo);
    }

	free(pathFinal);
	free(pathCodigo);
	
	return listaArchivoPseudo;
}	

//Luego de recibir opcode para enviar instruccion, ejecuta
void manejarPedidoInstruccion(t_peticionInstruccion peticion, t_list* listaProcesos, int socket)
{
	bool esIgualPID(void* proceso) 
	{
		bool aux = 0;
		if(peticion.PID == ((t_direccionProceso*)proceso)->PID)
		{
			aux = 1;
		}
		return aux;
	}

	t_direccionProceso* procesoBuscado = list_find(listaProcesos, (void*)esIgualPID);

	//Dentro de la lista de instrucciones accedo por index a la del program counter
	t_instruccion* instruccionBuscada = list_get(procesoBuscado->instrucciones, peticion.PC);

	enviarInstruccion(*instruccionBuscada, socket);
}

//Le envio el path y el PID a Memoria desde Kernel
void enviarPath(t_log* logger, char* path, int PID, int socket)
{
	op_code cop = PATH_INSTRUCCIONES;
	uint8_t largo = strlen(path) + 1;

	void* stream = malloc(sizeof(op_code) + sizeof(uint32_t) +  sizeof(uint8_t) + largo);
	
	memcpy(stream, &cop, sizeof(op_code));
	memcpy(stream + sizeof(op_code), &(PID), sizeof(uint32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t), &(largo), sizeof(uint8_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) + sizeof(uint8_t), path, largo);

	send(socket, stream, sizeof(op_code) + sizeof(uint32_t) + sizeof(uint8_t) + largo, 0);
	
	free(stream);
}

//Recibe el identificador del proceso, el path y acciona en base a ello 
void recibirPath(t_log* logger, t_list* listaProcesos, int socket, char* pathCarpetaPseudo, t_list* directorio)
{
	//Declaro la lista que luego llenaré con las instrucciones
	t_identificadorDePseudocodigo identificador;

	//Recibo primero el PID
	recv(socket, &(identificador.PID), sizeof(uint32_t), MSG_WAITALL);

	//Recibo el largo del path
	recv(socket, &(identificador.largoCadena), sizeof(uint8_t), MSG_WAITALL);

	//Le asigno la memoria necesaria segun su largo al path y lo recibo
	identificador.path = malloc(identificador.largoCadena);
	recv(socket, identificador.path, identificador.largoCadena, MSG_WAITALL);

	//Obtengo el pseudocodigo a partir del path extraido del identificador
	t_list* listaInstrucciones = obtenerPseudoCodigo(logger, identificador.path, pathCarpetaPseudo);

	if(!list_is_empty(listaInstrucciones))
	{
		log_info(logger, "PID: %d - Tamaño: 0", identificador.PID);

		//Creo la tabla de paginas del proceso
		t_tabla_paginas* aux = malloc(sizeof(t_tabla_paginas));

		aux->PID = identificador.PID;
		aux->paginas = list_create();

		list_add(directorio, aux);

		//Declaro la variable donde almacenaré el puntero al nuevo proceso
		t_direccionProceso* nuevoProceso = malloc(sizeof(t_direccionProceso));

		//Relleno la variable donde almacenaré el puntero al nuevo proceso
		nuevoProceso->PID = identificador.PID;
		nuevoProceso->instrucciones = listaInstrucciones;
	
		//Añado a la lista de procesos el nuevo proceso
		list_add(listaProcesos, nuevoProceso);

		//Le comunico a Kernel que decodifiqué el archivo de pseudocódigo con éxito y el proceso puede entrar a ready
		enviarConfirmacionPath(ENCONTRO_PATH, socket);
	}

	else
	{
		//Le comunico a Kernel que no existe el archivo
		enviarConfirmacionPath(NO_ENCONTRO_PATH, socket);
	}
}

//Funcion para manejar la interrupcion producida por fin de quantum
void manejarInterrupcionQuantum (int conexionKernelInterrupt, t_PCB* pcb, int* interrupciones)
{	
	//Cambia el valor de interrupciones al correspondiente para interpretar la interrupcion
	if(*interrupciones > 0)
	{
		*interrupciones = -2;
	}
}

//Funcion para manejar la interrupcion producida porque kernel dice que hay que finalizar el proceso
void manejarInterrupcionProcesoFinalizado (int conexionKernelInterrupt, t_PCB* pcb, int* interrupciones)
{	
	//Cambia el valor de interrupciones al correspondiente para interpretar la interrupcion
	if(*interrupciones != -1)
    {
        *interrupciones = -3;
    }
}

//----------INTERFACES-----------

//Funcion para buscar interfaces por nombre
t_elemento_interfaces* buscarInterfacesPorNombre(char* nombreInterfaz, t_lista* listaInterfaces)
{
	bool existeElNombre(void* interfaz) 
	{
		return (!strcmp(((t_elemento_interfaces*)interfaz)->nombre, nombreInterfaz));
	}

	pthread_mutex_lock(&(listaInterfaces->mutex));
	t_elemento_interfaces* interfazAux = list_find(listaInterfaces->lista, (void*)existeElNombre);
	pthread_mutex_unlock(&(listaInterfaces->mutex));

	return interfazAux;
}

//Funcion para enviar un sleep
bool enviarSleep(t_log* logger, t_lista* interfacesGenericas, char* nombreInterfaz, u_int32_t PID, u_int8_t unidades)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, interfacesGenericas);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarSleep(PID, unidades);

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(u_int8_t), 0);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	//Libero el stream
	free(stream);

	return true;
}

bool peticionSTDIN(t_log* logger, t_lista* interfacesSTDIN, char* nombreInterfaz, t_peticion_std dirFisica)
{	
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, interfacesSTDIN);

	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	void* stream = serializarSTDIN(dirFisica);
	
	send(interfaz->socket, stream, sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t), 0);

	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

bool peticionSTDOUT(t_log* logger, t_lista* interfacesSTDOUT, char* nombreInterfaz, t_peticion_std dirFisica)
{	
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, interfacesSTDOUT);

	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	void* stream = serializarSTDOUT(dirFisica);

	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(uint32_t) * 2 + sizeof(uint8_t), 0);

	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

void enviarDireccionFisicaYPID(int socket, t_peticion_std dirFisica)
{
	void* stream = malloc(sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t));

	op_code cop = CONTENIDO_STDOUT;

	memcpy(stream, &cop,sizeof(int));
	memcpy(stream + sizeof(int), &dirFisica.PID, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t), &dirFisica.direcFisica, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 2, &dirFisica.tamanio, sizeof(uint8_t));

	send(socket, stream, sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t), 0);

	free(stream);
}

//Funcion para recibir la direccion en entradasalida
t_peticion_std recibirDireccionFisicaYPID(int socket, t_log* logger)
{
	t_peticion_std aux;

	//Recibo la informacion serializada
	recv(socket, &aux.PID, sizeof(uint32_t), 0);
	recv(socket, &aux.direcFisica, sizeof(uint32_t), 0);
	recv(socket, &aux.tamanio, sizeof(uint8_t), 0);

	//Reservo el espacio recibido para la cadena
	aux.cadena = malloc(aux.tamanio);

	//retorno la estructura creada
	return aux;
}

void recibirSTDIN(argsEsperarMensaje* args)
{
	accederAEspacioUsuario(1, args->cliente, args->directorio, args->logger, args->tamanioPagina, args->memoria);
}

void recibirSTDOUT(argsEsperarMensaje* args)
{
	int socket = args->cliente;
	int tamanioPagina = args->tamanioPagina;
	void* memoria = args->memoria;
	t_log* logger = args->logger;
	t_list* directorio = args->directorio;

	accederAEspacioUsuario(0, socket, directorio, logger, tamanioPagina, memoria);
}

void recibirPeticionTamanioPagina(argsEsperarMensaje* args)
{
	void* aEnviar = malloc(sizeof(op_code) + sizeof(int));

	int socket = args->cliente;
	int tamanioPagina = args->tamanioPagina;
	
	op_code opc = ENVIO_TAMANO_PAGINA;

	memcpy(aEnviar, &opc, sizeof(op_code));
	memcpy(aEnviar + sizeof(op_code), &tamanioPagina, sizeof(int));

	send(socket, aEnviar, sizeof(op_code) + sizeof(int), 0);

	free(aEnviar);
}

t_peticion_fs recibirNombreYPID(int socket,t_log* logger)
{
	t_peticion_fs aux;

	recv(socket, &aux.PID, sizeof(uint32_t), 0);
	recv(socket, &aux.largoNombre, sizeof(uint32_t),0);
	
	aux.nombrArchivo = malloc(aux.largoNombre);

	recv(socket, aux.nombrArchivo, aux.largoNombre,0);

	return aux;
}

t_peticion_fs recibirTruncate(int socket, t_log* logger)
{
	t_peticion_fs aux;

	recv(socket,&aux.largoNombre,sizeof(uint32_t),0);
	recv(socket,aux.nombrArchivo,aux.largoNombre,0);
	recv(socket,&aux.nuevoTamanio,sizeof(uint32_t),0);

	return aux;
}

t_peticion_fs recibirContenidoDialfs(int socket,t_log* logger)
{
	t_peticion_fs aux;

	recv(socket, &aux.largoNombre, sizeof(uint32_t),0);
	recv(socket, aux.nombrArchivo, aux.largoNombre,0);
	recv(socket, &aux.nuevoTamanio, sizeof(uint32_t),0);
	recv(socket, &aux.direcLogica, sizeof(uint8_t),0);
	recv(socket, &aux.regPuntero, sizeof(uint32_t),0);
	
	return aux;
}

void recibirPID(int socket, u_int32_t* PID)
{
	recv(socket, PID, sizeof(u_int32_t), MSG_WAITALL);
}

//Funcion para recibir un sleep
void recibirSleep(int socket, u_int8_t* sleep, t_log* logger)
{
	//Creo el stream donde voy a recibir el sleep
	void* stream = malloc(sizeof(u_int8_t));

	//Recibo la informacion serializada
	recv(socket, stream, sizeof(u_int8_t), 0);

	//Deserializo lo que recibi
	deserializarSleep(stream, sleep);

	//Mostrar PCB - para probar Kernel
	log_info(logger, "Llego un sleep de %d unidades de tiempo", *sleep);

	//Libero el stream
	free(stream);
}

//Funcion para enviar interfaces
void enviarInterfaz(int fd, char* nombreInterfaz, tipo_interfaz tipoInterfaz)
{
	int longitud = strlen(nombreInterfaz) + 1;

	//Creo un stream con la interfaz serializada
	void* stream = serializarInterfaz(longitud, nombreInterfaz, tipoInterfaz);

	//Envio el stream ya cargado
	send(fd, stream, sizeof(op_code) + sizeof(int) + longitud + sizeof(tipo_interfaz), 0);

	//Libero el stream
	free(stream);

}

bool gestionarInterfaz(t_lista* interfacesConectadas, t_elemento_interfaces* elemento, t_log* logger)
{
	int respuesta = recibirOperacion(elemento->socket);

	switch(respuesta)
	{
		case 0:
			return(false);
			break;

		case 1:
			sem_post(&(elemento->continua));
			return(true);
			break;

		case -1:
			pthread_mutex_lock(&(interfacesConectadas->mutex));
			list_remove_element(interfacesConectadas->lista, elemento);
			pthread_mutex_unlock(&(interfacesConectadas->mutex));
			return(false);
			break;

		default:
			log_info(logger, "La confirmacion es invalida");
			return(false);
			break;
	}
}

//Funcion para recibir interfaces
void recibirInterfaz(argsEsperarMensaje* args) 
{   
	//Creo las variables necesarias para recibir la interfaz
    tipo_interfaz* tipoInterfaz = malloc(sizeof(tipo_interfaz));

	int longitud;

	//Creo el stream donde voy a recibir el PCB
	void* stream = malloc(sizeof(int));

	//Recibo primero la longitud del nombre
	recv(args->cliente, stream, sizeof(int), 0);
	memcpy(&longitud, stream, sizeof(int));

	free(stream);

	//Modifico el tamaño del stream y ahora si recibo el resto del contenido
	void* otroStream = malloc(longitud + sizeof(tipo_interfaz));
	recv(args->cliente, otroStream, longitud + sizeof(tipo_interfaz), 0);

	char* nombreInterfaz = malloc(longitud);

	//Deserializo lo que recibi
	deserealizarInterfaz(otroStream, longitud, nombreInterfaz, tipoInterfaz);

	free(otroStream);

	//Creo un elemento cargable en la lista y le cargo sus valores
	t_elemento_interfaces* elemento = malloc(sizeof(t_elemento_interfaces));
	pthread_mutex_init(&(elemento->mutex), NULL);
	sem_init(&(elemento->continua), 0, 0);
	elemento->nombre = malloc(longitud);
	strcpy(elemento->nombre, nombreInterfaz);
	elemento->socket = args->cliente;
	
	switch(*tipoInterfaz)
	{
		case GENERICA:
			log_info(args->logger, "%s se agrego como una interfaz generica", nombreInterfaz);
			pthread_mutex_lock(&(args->interfacesGenericas->mutex));
			list_add(args->interfacesGenericas->lista, elemento);
			pthread_mutex_unlock(&(args->interfacesGenericas->mutex));

			free(tipoInterfaz);

			while(gestionarInterfaz(args->interfacesGenericas, elemento, args->logger));

			break;

		case STDIN:
			log_info(args->logger, "%s se agrego como una interfaz STDIN", nombreInterfaz);
			pthread_mutex_lock(&(args->interfacesSTDIN->mutex));
			list_add(args->interfacesSTDIN->lista, elemento);
			pthread_mutex_unlock(&(args->interfacesSTDIN->mutex));

			free(tipoInterfaz);

			while(gestionarInterfaz(args->interfacesSTDIN, elemento, args->logger));

			break;

		case STDOUT:
			log_info(args->logger, "%s se agrego como una interfaz STDOUT", nombreInterfaz);
			pthread_mutex_lock(&(args->interfacesSTDOUT->mutex));
			list_add(args->interfacesSTDOUT->lista, elemento);
			pthread_mutex_unlock(&(args->interfacesSTDOUT->mutex));

			free(tipoInterfaz);

			while(gestionarInterfaz(args->interfacesSTDOUT, elemento, args->logger));

			break;

		case DIALFS:
			log_info(args->logger, "Se conecto la interfaz %s como DIALFS", nombreInterfaz);
			
			pthread_mutex_lock(&(args->interfacesDIALFS->mutex));
			list_add(args->interfacesDIALFS->lista, elemento);
			pthread_mutex_unlock(&(args->interfacesDIALFS->mutex));

			free(tipoInterfaz);
			
			while(gestionarInterfaz(args->interfacesDIALFS, elemento, args->logger));

			break;

		default:
			log_info(args->logger, "No existe el tipo de interfaz recibido");
			break;
	}

	free(elemento->nombre);
	free(elemento);
}

t_peticion_fs recibirPeticionEscribir(int socket, t_log* logger)
{
	t_peticion_fs aux;

	//Recibo la informacion serializada
	recv(socket, &aux.PID, sizeof(uint32_t), 0);
	recv(socket, &aux.direcLogica, sizeof(uint32_t), 0);
	recv(socket, &aux.largoNombre, sizeof(uint8_t), 0);
	recv(socket, &aux.regPuntero, sizeof(uint32_t), 0);
	
	//Reservo el espacio recibido para la cadena
	aux.nombrArchivo = malloc(aux.largoNombre);

	//retorno la estructura creada
	return aux;
}

bool enviarFS_CREATE(t_log* logger, t_lista* fileSystem, char* nombreInterfaz, char* nombreArchivo, uint32_t PID)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, fileSystem);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarFS_CREATE(nombreArchivo, PID);

	int longitudNombre = strlen(nombreArchivo) + 1;

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(uint32_t) + sizeof(int) + longitudNombre, 0);

	//Libero el stream
	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

bool enviarFS_DELETE(t_log* logger, t_lista* fileSystem, char* nombreInterfaz, char* nombreArchivo, u_int32_t PID)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, fileSystem);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarFS_DELETE(nombreArchivo, PID);

	int longitudNombre = strlen(nombreArchivo) + 1;

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(uint32_t) + sizeof(int) + longitudNombre, 0);

	//Libero el stream
	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

bool enviarFS_TRUNCATE(t_log* logger, t_lista* fileSystem, char* nombreInterfaz, char* nombreArchivo, uint32_t tamanioNuevo, u_int32_t PID)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, fileSystem);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarFS_TRUNCATE(nombreArchivo, tamanioNuevo, PID);

	int longitudNombre = strlen(nombreArchivo) + 1;

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(int) + longitudNombre + sizeof(uint32_t), 0);

	//Libero el stream
	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

bool enviarFS_WRITE(t_log* logger, t_lista* fileSystem, char* nombreInterfaz, char* nombreArchivo, uint32_t PID, uint32_t dirLogica, uint8_t tamanioAEscribir, uint32_t punteroArchivo)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, fileSystem);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarFS_WRITE(nombreArchivo, PID, dirLogica, tamanioAEscribir, punteroArchivo);

	int longitudNombre = strlen(nombreArchivo) + 1;

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 3 + sizeof(uint8_t), 0);

	//Libero el stream
	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

bool enviarFS_READ(t_log* logger, t_lista* fileSystem, char* nombreInterfaz, char* nombreArchivo, uint32_t PID, uint32_t dirLogica, uint8_t tamanioALeer, uint32_t punteroArchivo)
{
	t_elemento_interfaces* interfaz = buscarInterfacesPorNombre(nombreInterfaz, fileSystem);
	
	//Si la interfaz buscada no existe
	if(interfaz == NULL)
	{
		log_info(logger, "La interfaz %s no existe", nombreInterfaz);
		return false;
	}

	//Creo un stream con el PCB serializado
	void* stream = serializarFS_READ(nombreArchivo, PID, dirLogica, tamanioALeer, punteroArchivo);

	int longitudNombre = strlen(nombreArchivo) + 1;

	//Envio el stream ya cargado
	send(interfaz->socket, stream, sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 3 + sizeof(uint8_t), 0);

	//Libero el stream
	free(stream);

	//Espero a que responda la interfaz
	sem_wait(&(interfaz->continua));

	return true;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------MEMORIA Y NADA MAS QUE MEMORIA DE ACÁ PARA ABAJO-----------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/

//Devuelve una lista con la cantidad de paginas pedidas en orden desde la inicial
t_list* devolverPaginasDesde(t_list* paginasProceso, int paginaInicial, int cantidadPaginas)
{
    int i = 0;
    t_pagina* aux;

    t_list* paginas = list_create();

    while(i < cantidadPaginas)
    {
        aux = buscarPaginaPorNumero(paginasProceso, paginaInicial + i);
        list_add(paginas, aux);
		i++;
    }

    return paginas;
}

t_pagina* buscarPaginaPorFrame(t_list* listaPaginas, int marcoABuscar)
{ 
    //Funcion para buscar la pagina
    bool esIgualFrame(void* pagina) 
	{
		t_pagina* pagAux = (t_pagina*)pagina;

		bool aux = 0;
		if(marcoABuscar == pagAux->marco)
		{
			aux = 1;
		}

		return aux;
	};

    t_pagina* paginaBuscada = list_find(listaPaginas, (void*)esIgualFrame);
	
    return paginaBuscada;
}

t_pagina* buscarPaginaPorNumero(t_list* listaPaginas, int paginaABuscar)
{
    //Funcion para buscar la pagina
    bool esIgualPagina(void* pagina) 
	{
		bool aux = 0;
		if(paginaABuscar == ((t_pagina*)pagina)->numeroPagina)
		{
			aux = 1;
		}
		return aux;
	};

    t_pagina* paginaBuscada = list_find(listaPaginas, (void*)esIgualPagina);

    return paginaBuscada;
}

t_peticion_acceso_eu deserializarPeticionAccesoEu(void* stream)
{
    t_peticion_acceso_eu aux;
    memcpy(&aux.PID, stream, sizeof(int));
    memcpy(&aux.dirFisica, stream + sizeof(int), sizeof(uint32_t));
    memcpy(&aux.tamanio, stream + sizeof(int) + sizeof(uint32_t) , sizeof(int));
    return aux;
}

//Devuelve los frames del proceso
t_list* framesDelProceso(t_list* directorio, int PID)
{
    bool esIgualPID(void* tabla) 
	{
		bool aux = 0;
		if(PID == ((t_tabla_paginas*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	};
	
    t_tabla_paginas* aux = list_find(directorio, (void*)esIgualPID);

    return (aux->paginas);
}

//Guarda un dato en memoria
void guardarDato(int dirFisica, void* memoria, void* dato, int tamanio)
{
    memcpy((memoria+dirFisica), dato, tamanio);
}

//Cantidad de paginas asignadas a un proceso
int tamanioProceso(t_list* DirectoriotablaPaginas, int PID)
{
    int cantidadDePaginas;

    bool esIgualPID(void* tabla) 
	{
		bool aux = 0;
		if(PID == ((t_tabla_paginas*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	}

    //Busco la tabla de paginas del proceso pedido
    t_tabla_paginas* aux = list_find(DirectoriotablaPaginas,(void*)esIgualPID);

    if(list_is_empty(aux->paginas))
    {
        return 0;
    }
    
    //Obtengo la cantidad de paginas del proceso
    cantidadDePaginas = list_size(aux->paginas);

    return cantidadDePaginas;
}

void hacerFree(void* dato)
{
    free(dato);
}

//-------------------------------------------------Bit arrays-----------------------------------------------//

//Inicia el bitarray de frames disponibles
t_bitarray* iniciarBitArray(int tamanioMemoria, int tamanioPagina , char* cadena, t_log* logger)
{
    //Armo el bitarray de frames disponibles
	int largoCadenaBytes;
	int tamanioBitArray = tamanioMemoria/tamanioPagina;

	if(tamanioBitArray < 8)
	{
		largoCadenaBytes = 1; 
	}
	else
	{
		largoCadenaBytes = ceil((double)tamanioBitArray/8.0);
	}

	cadena = malloc(largoCadenaBytes);
	memset(cadena,0,largoCadenaBytes);

    t_bitarray* frames = bitarray_create_with_mode(cadena,(largoCadenaBytes),MSB_FIRST);

    //Seteo todos los frames como libres
	int i=0;
    while(i<(largoCadenaBytes*8))
    {
		if(i<tamanioBitArray)
		{
			bitarray_set_bit(frames, i);
		}
		else
		{
			bitarray_clean_bit(frames, i);
		}
		i++;
    }

    return frames;
}

//Devuelve la cantidad de frames libres en la memoria
int cantidadDeFramesLibres(t_bitarray* frames)
{
    int n = 0, i = 0;

    //Recorre el bitarray contando la cantidad de disponibles
    while((i < bitarray_get_max_bit(frames)))
    {
        if(bitarray_test_bit(frames,i))
        {
            n++;
        }
    }
    return n;
}

//Devuelve los frames libres en memoria
t_list* framesLibres(t_bitarray* frames)
{
    t_list* framesDisponibles = list_create();

	int i = 0;

    while((i < bitarray_get_max_bit(frames)))
    {

        if(bitarray_test_bit(frames,i))
        {	
			int* aux = malloc(sizeof(int));
            *aux = i;
            list_add(framesDisponibles, aux);
        }
		i++;
    }

    return framesDisponibles;
}

//Marca los frames como ocupados en el bit array
void ocuparFrames(t_bitarray* frames, t_list* framesAOcupar)
{
    int i = 0;
	int* n;

    while(i < (list_size(framesAOcupar)))
    {
    	n = list_get(framesAOcupar, i);
        bitarray_clean_bit(frames,*n);
        i++;
    }

	list_destroy_and_destroy_elements(framesAOcupar, (void*)hacerFreeListaInts);
}

//Marca los frames como desocupados en el bit array
void desOcuparFrames(t_bitarray* frames, t_list* framesADesOcupar)
{
    int i=0;
    int* n = malloc(sizeof(int));
	*n = 0;

    while(i < (list_size(framesADesOcupar)))
    {
        n = list_get(framesADesOcupar, i);
        bitarray_set_bit(frames, *n);
        i++;
    }

	list_destroy_and_destroy_elements(framesADesOcupar, (void*)hacerFreeListaInts);
}

void hacerFreeListaInts(int* algo)
{
	free(algo);
}

//------------------------------------------Funciones clave----------------------------------------//

//Devolver numero de marco
void recibirPedidoMarco(int socket, t_list* directorio)
{
    void* data = malloc(2*sizeof(int));
    int pid, pagina;

    //Deserializo la peticion del marco
    recv(socket, data, 2 * sizeof(int),0);
    memcpy(&pid, data, sizeof(int)); 
    memcpy(&pagina, data + sizeof(int), sizeof(int));

    bool esIgualPID(void* tabla) 
	{
		bool aux = 0;
		if(pid == ((t_tabla_paginas*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	};

    //Busco la tabla de paginas del proceso pedido
    t_tabla_paginas* auxTabla = list_find(directorio,(void*)esIgualPID);

    bool esIgualPagina(void* unaPagina) 
	{
		bool aux = 0;
		if(pagina == ((t_pagina*)unaPagina)->numeroPagina)
		{
			aux = 1;
		}
		return aux;
	};

    //Busco la página pedida
    t_pagina* auxPagina = list_find(auxTabla->paginas, (void*)esIgualPagina);

    //Serializo el marco a enviar
    void* aEnviar = malloc(sizeof(int) + sizeof(op_code));
    op_code codigo = ENVIO_MARCO;
    memcpy(aEnviar, &codigo, sizeof(op_code));
    memcpy(aEnviar + sizeof(op_code), &(auxPagina->marco), sizeof(int));

    //Envio el marco
    send(socket, aEnviar, sizeof(int) + sizeof(op_code), 0);

    free(data);
    free(aEnviar);
}

//Recibe la petición de resize y la ejecuta
void recibirResize(int socket, t_bitarray* frames, t_list* directorio, t_log* logger, int tamanioMarco)
{
    //Obtengo el PID del proceso
    int PID;
    recv(socket, &PID, sizeof(int), 0);

    //Obtengo el valor del resize
    int valorResize;
    recv(socket, &valorResize, sizeof(int), 0);

    //Obtener los frames actuales del proceso y los nuevos
    int framesActuales = tamanioProceso(directorio, PID);
    int nuevosFrames = (int) ceil((double)valorResize / (double)tamanioMarco);
    int tamanioAModificar;

    //Para el directorio
    bool esIgualPID(void* tabla) 
	{
		bool aux = 0;
		if(PID == ((t_tabla_paginas*)tabla)->PID)
		{
			aux = 1;
		}
		return aux;
	}

    //Obtener tabla de paginas necesaria
    t_tabla_paginas* tabla = list_find(directorio, (void*)esIgualPID);
    
    //Realizar operatoria
    int i = 0;
	int* n = NULL;

    op_code opc;
    void* algo = malloc(sizeof(int) * 2);

    if(framesActuales < nuevosFrames) //Debo agrandar el espacio del proceso
    {
        tamanioAModificar = nuevosFrames - framesActuales;

        t_list* libres = framesLibres(frames); 
        t_list* aOcupar = list_create();

        while(i < tamanioAModificar) //Ejecuta hasta que no haya más páginas por añadir
        {
            if(list_size(libres) > 0)
            {
                t_pagina* auxPagina = malloc(sizeof(t_pagina));

                n = list_remove(libres, 0); 
                
				//Añade el marco a la lista de marcos a ocupar
				list_add(aOcupar, n);
                auxPagina->numeroPagina = (framesActuales + i);
                auxPagina->marco = *n;

				//Añade la pagina a la tabla de paginas del proceso
                list_add(tabla->paginas, auxPagina);

                i++;
            }

            //Si no hubo frames disponibles
            else if(list_size(libres) == 0)
            {
                log_info(logger,"PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", PID, (framesActuales * tamanioMarco), ((i-1)*tamanioMarco));

                opc = OUT_OF_MEMORY;
				int a = 0;

                memcpy(algo, &opc, sizeof(int));
				memcpy(algo + sizeof(int), &a, sizeof(int));
                send(socket, algo, sizeof(int) * 2, 0);

                break;
            }
        }

		list_destroy_and_destroy_elements(libres, (void*)hacerFreeListaInts);

        //Si hubo frames disponibles
        if(i == tamanioAModificar)
        {
            log_info(logger,"PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", PID, (framesActuales*tamanioMarco), (tamanioAModificar*tamanioMarco));
            opc = RESIZE_OK;
			int cantPaginas = nuevosFrames;

            memcpy(algo, &opc, sizeof(int));
			memcpy(algo + sizeof(int), &cantPaginas, sizeof(int));
            send(socket, algo, sizeof(int) * 2, 0);
        }

		//Ocupa los frames en el bitarray
        ocuparFrames(frames, aOcupar);
    }

    else if(framesActuales > nuevosFrames) //Debo achicar el espacio del proceso
    {
        t_pagina* auxPagina;
        tamanioAModificar = framesActuales - nuevosFrames;

        log_info(logger,"PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d", PID, (framesActuales*tamanioMarco), (tamanioAModificar*tamanioMarco));
        
		//Mientras haya paginas por eliminar...
		while(i < tamanioAModificar)
        {
			auxPagina = list_get(tabla->paginas, list_size(tabla->paginas) - 1);

			//Lo marco como desocupado en el bitmap
			bitarray_set_bit(frames, auxPagina->marco);

			list_remove_and_destroy_element(tabla->paginas, list_size(tabla->paginas) - 1, (void*)hacerFreePaginas);

			i++;
        }

		opc = RESIZE_OK;
		int cantPaginas = nuevosFrames;
        memcpy(algo, &opc, sizeof(int));
		memcpy(algo + sizeof(int), &cantPaginas, sizeof(int));
        send(socket, algo, sizeof(int) * 2, 0);
    }

    else
    {
        log_info(logger,"PID: %d - El tamaño del proceso ya cuenta con esa cantidad de paginas", PID);
		opc = RESIZE_OK;
		int cantPaginas = nuevosFrames;
        memcpy(algo, &opc, sizeof(int));
		memcpy(algo + sizeof(int), &cantPaginas, sizeof(int));
        send(socket, algo, sizeof(int) * 2, 0);
    }

    //Libero memoria
    free(algo);
}

void accederAEspacioUsuario(int modo, int socket, t_list* directorio, t_log* logger, int tamanioMarco, void* memoria)	
{
    //Recibo el buffer
    t_peticion_acceso_eu peticion;
	recv(socket, &peticion.PID, sizeof(uint32_t), 0);
    recv(socket, &peticion.dirFisica, sizeof(uint32_t), 0);
    recv(socket, &peticion.tamanio, sizeof(uint8_t), 0);

    //Dato a escribir u obtener
	int confirmacion = 0;
    void* dato = malloc(peticion.tamanio);

    //Lista con las paginas de todo el proceso
    t_list* paginasProceso = framesDelProceso(directorio, peticion.PID);

    //Cantidad de paginas que ocupa el dato
    int paginasDato;
    int marcoInicial = (int)floor((double)peticion.dirFisica / (double)tamanioMarco);
    int desplazamiento = peticion.dirFisica - (marcoInicial * tamanioMarco);

	t_pagina* paginaAux = buscarPaginaPorFrame(paginasProceso, marcoInicial);

    int paginaInicial = paginaAux->numeroPagina;

    //Calculo en base a los desplazamientos
    if(desplazamiento == 0)
    {
        paginasDato = (int)ceil(((double)peticion.tamanio) / ((double)tamanioMarco));
    }

    else if(desplazamiento > 0)
    {
        paginasDato = (int)ceil(((double)(desplazamiento + peticion.tamanio)) / ((double)(tamanioMarco)));
    }

    //Obtener Paginas
    t_list* paginasParaAcceder = devolverPaginasDesde(paginasProceso , paginaInicial, paginasDato);

    //Acceso al espacio usuario
	//Lectura
	if(modo == 0)
    {
		log_info(logger, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño: %d", peticion.PID, peticion.dirFisica, peticion.tamanio);

        int i = 0;
		
        t_pagina* paginaALeer;

		//Si no hay desplazamiento en el primer marco
        if(desplazamiento == 0)
        {
            while(i < paginasDato)
            {
                paginaALeer = list_get(paginasParaAcceder, i);
				log_info(logger, "PID: %d - Pagina: %d - Marco: %d", peticion.PID, paginaALeer->numeroPagina, paginaALeer->marco);

				//Si tiene que copiar la ultima pagina
                if(i == (paginasDato - 1))
                {
					if(paginasDato == 1)
					{
						memcpy((dato + (i * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco)), peticion.tamanio);
					}

					else
					{
                    	int restante = peticion.tamanio - (i * tamanioMarco);
                    	memcpy((dato + (i * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco)), restante);
					}
                }

				//Si simplemente copia una pagina
                else
                {
                    memcpy((dato + (i * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco)), tamanioMarco);
                }

                i++;
            }
        }

        else if(desplazamiento > 0) //Si hay desplazamiento en el primer marco
        {
            while(i < paginasDato)
            {
                paginaALeer = list_get(paginasParaAcceder, i);
				log_info(logger, "PID: %d - Pagina: %d - Marco: %d", peticion.PID, paginaALeer->numeroPagina, paginaALeer->marco);

                if(i == 0) //Para la primer página, o sea, la que viene con desplazamiento
                {
					if(paginasDato == 1)
					{
						memcpy((dato + (i * tamanioMarco)),(memoria + ((paginaALeer->marco) * tamanioMarco) + desplazamiento), peticion.tamanio);
					}
					else
					{
                    	memcpy((dato + (i * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco) + desplazamiento), (tamanioMarco - desplazamiento));
					}
                }

                else if(i == (paginasDato - 1)) //Si tiene que copiar la ultima pagina
                {
                    int restante = peticion.tamanio - ((i - 1) * tamanioMarco) - (tamanioMarco - desplazamiento);
                    memcpy((dato + (tamanioMarco - desplazamiento) + ((i - 1) * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco)), restante);
                }

                else //Si simplemente copia una pagina
                {
                    memcpy((dato + (tamanioMarco - desplazamiento) +((i - 1) * tamanioMarco)), (memoria + ((paginaALeer->marco) * tamanioMarco)), tamanioMarco);
                }

                i++;
            }
        }

		confirmacion = 1;
		send(socket, &confirmacion, sizeof(int), 0);

        //Envio el dato
		send(socket, &peticion.tamanio, sizeof(uint8_t), 0);
		send(socket, dato, peticion.tamanio, 0);

        //Elimino los auxiliares
        free(dato);
    }

	//Escritura
    else if(modo == 1)
    {
		log_info(logger,"PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", peticion.PID, peticion.dirFisica, peticion.tamanio);

		recv(socket, dato, peticion.tamanio, 0);

        int i = 0;

        t_pagina* paginaAEscribir; 

		//Si no hay desplazamiento en el primer marco
        if(desplazamiento == 0)
        {
            while(i < paginasDato)
            {
                paginaAEscribir = list_get(paginasParaAcceder, i);
				log_info(logger,"PID: %d - Pagina: %d - Marco: %d", peticion.PID, paginaAEscribir->numeroPagina, paginaAEscribir->marco);

				//Si tiene que copiar la ultima pagina
                if(i == (paginasDato - 1))
                {
                    int restante = peticion.tamanio - (i*tamanioMarco);
                    memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco)) , (dato + (i * tamanioMarco)) , restante);
                }

                //Si simplemente copia una pagina
				else
                {
                    memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco)), (dato + (i * tamanioMarco)) , tamanioMarco);
                }
                i++;
            }
        }

		//Si hay desplazamiento en el primer marco
        else if(desplazamiento > 0)
        {
            while(i < paginasDato)
            {
                paginaAEscribir = list_get(paginasParaAcceder, i);
				log_info(logger,"PID: %d - Pagina: %d - Marco: %d", peticion.PID, paginaAEscribir->numeroPagina, paginaAEscribir->marco);

                //Para la primer página, o sea, la que viene con desplazamiento
				if(i == 0)
                {
					if(paginasDato == 1)
					{
						memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco) + desplazamiento) , (dato + (i * tamanioMarco)), peticion.tamanio);
					}
					else
					{
                    	memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco) + desplazamiento), (dato + (i * tamanioMarco)), (tamanioMarco - desplazamiento));
					}
                }

				//Si tiene que copiar la ultima pagina
                else if(i == (paginasDato - 1))
                {
                    int restante = peticion.tamanio - ((i - 1) * tamanioMarco) - (tamanioMarco - desplazamiento);
                    memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco)), (dato + (tamanioMarco - desplazamiento) + ((i - 1) * tamanioMarco)), restante);
                }

				//Si simplemente copia una pagina
                else
                {
                    memcpy((memoria + ((paginaAEscribir->marco) * tamanioMarco)), (dato + (tamanioMarco - desplazamiento) + ((i - 1) * tamanioMarco)), tamanioMarco);
                }

                i++;
            }
        }

        //Aviso que escribí en espacio usuario satisfactoriamente
		confirmacion = 1;

        send(socket, &confirmacion, sizeof(int), 0);
		
        free(dato);
    }

    //Destruyo listas
	list_destroy(paginasParaAcceder);
}