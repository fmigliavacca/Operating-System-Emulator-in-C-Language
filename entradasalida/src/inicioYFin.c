//-------------BIBLIOTECAS--------------

#include <../include/inicioYFin.h>

//-------------FUNCIONES-------------

//Funcion para iniciar el logger
t_log* iniciarLogger(void)
{
	t_log* nuevo_logger;
	nuevo_logger = log_create("entradasalida.log","ENTRADASALIDA",1,LOG_LEVEL_INFO);

	return nuevo_logger;
}

//Funcion para iniciar el config del main
t_config* iniciarConfig(char* path)
{
	
	t_config* nuevo_config = config_create(path);

	if(nuevo_config == NULL)
	{
		perror("Error al crear el config");
		return NULL;
	}

	return nuevo_config;
}

//Funcion para cargar el config del main
t_valores cargarConfig(t_config* config)
{
    t_valores valores;

	valores.blockSize = config_get_int_value(config, "BLOCK_SIZE");
    valores.ipKernel = config_get_string_value(config, "IP_KERNEL");
	valores.blockCount = config_get_int_value(config, "BLOCK_COUNT");
    valores.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
	valores.pathDialfs = config_get_string_value(config,"PATH_BASE_DIALFS");
	valores.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
	valores.puertoKernel = config_get_string_value(config, "PUERTO_KERNEL");
	valores.puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
	valores.retrasoCompactacion = config_get_int_value(config,"RETRASO_COMPACTACION");
	valores.tiempoUnidadTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

	//El tiempo unidad de trabajo esta en mS por lo que lo multiplico por 1000 para transformarlo a uS
	valores.tiempoUnidadTrabajo *= 1000;

    return valores;
}

//Funcion para librar los recursos
void terminarPrograma(int conexionKernel, int conexionMemoria, t_log* logger, t_config* config)
{
	log_destroy(logger);
	close(conexionKernel);
    close(conexionMemoria);
	config_destroy(config);
}