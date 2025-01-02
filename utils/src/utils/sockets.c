#include <../include/sockets.h>
#include <../include/comunicacion.h>

//Funcion para crear la conexion como cliente
int crearConexion(char *ip, char* puerto, t_log* logger)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
                    			server_info->ai_socktype,
                    			server_info->ai_protocol);
	
	//SOCKET REUTILIZABLE
	int optval = 1;
    setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

	int retornoConnect;

	do
	{
		log_info(logger, ".");
		retornoConnect = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
		sleep(5);
	} while (retornoConnect != 0);

	log_info(logger, "Conexion establecida !");

	freeaddrinfo(server_info);

	return socket_cliente;
}

//Funcion para crear un servidor
int iniciarServidor(char* puerto, t_log* logger)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
                         	servinfo->ai_socktype,
                         	servinfo->ai_protocol);

	//SOCKET REUTILIZABLE
	int optval = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

//Funcion para esperar un cliente
int esperarCliente(int socket_servidor, t_log* logger)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

//Funcion para atender clientes
void* atenderCliente(void* argumentos)
{
	//Bajo la informacion cargada en el void*
    argsEsperarMensaje* args = (argsEsperarMensaje*)argumentos;

	int* cliente = malloc(sizeof(int));		

	switch(args->rol)
	{
		//Si escucho como cliente
		case CLIENTE:

			//El valor del socket es el establecido en la conexion
			*cliente = args->cliente;
			
			//Recibo la respuesta
			esperarMensajes(args);

		break;

		//Si escucho como servidor
		case SERVIDOR:
			
			//Espero a que se conecte un cliente para asignarle un socket
			*cliente = esperarCliente(args->cliente, args->logger);
			args->cliente = *cliente;

			//Recibo los mensajes
			while(1)
			{
				esperarMensajes(args);
			}

			//Cierro el socket y libero los recursos
			close(*cliente);
			free(cliente);

		break;

		//Si escucho como interfaces
		case INTERFACES:

			//Creo un hilo por cada interfaz que llega
			while(crearHiloInterfaces(args));
			
		break;

		default: 

			log_info(args->logger, "El rol de comunicacion ingresado es invalido");

		break;
	}

	//Libero la estructura
	free(args);

	return NULL;
}

//Funcion para crear los hilos de escucha
void crearHiloEscucha(argsEsperarMensaje* args)
{
	//Defino un hilo
	pthread_t hilo;

	//Creo un hilo
	pthread_create(&hilo, NULL, atenderCliente, args);
    pthread_detach(hilo);
}

//Funcion para crear hilos on demand
bool crearHiloInterfaces(argsEsperarMensaje* argumentos)
{	
	int cliente = esperarCliente(argumentos->cliente, argumentos->logger);

	if(cliente != -1)
	{
		argsEsperarMensaje* args = malloc(sizeof(argsEsperarMensaje));

		args->cliente = cliente;
		args->fd = argumentos->fd;
		args->rol = argumentos->rol;
		args->pcb = argumentos->pcb;
		args->path = argumentos->path;
		args->codOp = argumentos->codOp;
		args->logger = argumentos->logger;
		args->memoria = argumentos->memoria;
		args->escucha = argumentos->escucha;
		args->interfaz = argumentos->interfaz;
		args->directorio = argumentos->directorio;
		args->framesMemoria = argumentos->framesMemoria;
		args->listaProcesos = argumentos->listaProcesos;
		args->tamanioPagina = argumentos->tamanioPagina;
		args->interfacesSTDIN = argumentos->interfacesSTDIN;
		args->interfacesSTDOUT = argumentos->interfacesSTDOUT;
		args->interfacesDIALFS = argumentos->interfacesDIALFS;
		args->retardoRespuesta = argumentos->retardoRespuesta;
		args->interfacesGenericas = argumentos->interfacesGenericas;

		pthread_t hilo;

		if(args->escucha == KERNEL)
		{
			pthread_create(&hilo, NULL, esperarMensajes, args);
    		pthread_detach(hilo);
		}

		else if(args->escucha == MEMORIA)
		{
			pthread_create(&hilo, NULL, esperarMensajesMemoria, args);
    		pthread_detach(hilo);
		}

		else
		{
			log_info(args->logger, "Error al crear el hilo");
			free(args);
		}

		return true;
	}

	return false;
}