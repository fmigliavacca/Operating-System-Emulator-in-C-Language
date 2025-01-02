//------------BIBLIOTECAS------------

#include <../include/inicioYFin.h>

//------------DEFINICION DE FUNCIONES------------

//Funcion para cargar los valores de config
t_valores cargarConfig(t_config* config) 
{
    t_valores valores;

    //Cargo los valores del config en la estructura
    valores.ipMemoria = config_get_string_value(config,"IP_MEMORIA");
    valores.algoritmoTlb = config_get_string_value(config,"ALGORITMO_TLB");
    valores.puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
    valores.cantidadEntradasTlb = config_get_int_value(config,"CANTIDAD_ENTRADAS_TLB");
    valores.puertoEscuchaDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    valores.puertoEscuchaInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

    //Retorno la estructura cargada
    return valores;
}

//Funcion para iniciar config
t_config* iniciarConfig(char* path)
{
    //Crea el config
    t_config* nuevo_config = config_create(path);

    //Evalua y devuelve el error de ser necesario
	if(nuevo_config == NULL)
    {
		perror("Error al crear el config");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}

//Funcion para iniciar logger
t_log* iniciarLogger(void)
{
    //Crea el logger
	t_log* nuevo_logger = log_create("cpu.log", "cpu-info", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

int pedirTamPag(int conexionMemoria)
{
    //Le digo su codigo de operacion
    op_code codOp = PETICION_TAMANO_PAGINA;

    //Envio el stream
    send(conexionMemoria, &codOp, sizeof(op_code), 0);

    //Verifico que el codigo de operacion sea el corricto
    if(recibirOperacion(conexionMemoria) != ENVIO_TAMANO_PAGINA)
    {
        perror("Codigo de operacion invalido");
        exit(EXIT_FAILURE);
    }

    //Creo una variable para poder recibir el tamano de la pagina
    int tamanoPagina;

    //Recibo el tamano de pagina
    recv(conexionMemoria, &tamanoPagina, sizeof(int), MSG_WAITALL);

    //Devuelvo el tamano de pagina
    return tamanoPagina;
}

//Funcion para liberar recursos
void terminarPrograma(int conexionKernelDispatch, int conexionKernelInterrupt, int conexionMemoria , t_log* logger, t_config* config, t_dictionary* diccionarioRegistros, void* tlb)
{
    free(tlb);
    log_destroy(logger);
    close(conexionMemoria);
	config_destroy(config);
    close(conexionKernelDispatch);
    close(conexionKernelInterrupt);
    dictionary_destroy(diccionarioRegistros);
}