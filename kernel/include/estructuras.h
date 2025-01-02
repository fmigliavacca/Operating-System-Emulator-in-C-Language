#ifndef KERNEL_ESTRUCTURAS_H_
#define KERNEL_ESTRUCTURAS_H_

//Estructura para almacenar los valores del config
typedef struct
{
	int quantum;
	int gradoMultiProgramacion;
    char* ipCpu;
	char* valor;
	char* ipMemoria;
	char* pathCarpeta;
	char* puertoEscucha;
	char* puertoMemoria;
	char* puertoCpuDispatch;
	char* puertoCpuInterrupt;
	char* algoritmoPlanificacion;
	char** recursos;
	char** instanciasRecursos;
}t_valores;

//Estructura para almacenar los valores de las conexiones
typedef struct
{
	int conexionMemoria;
	int conexionDispatch;
	int conexionInterrupt;
	int conexionInterfazIO;
}t_conexiones;

//Enum de instrucciones consola kernel
typedef enum
{
	ERROR,
	PROCESO_ESTADO,
	INICIAR_PROCESO,
	EJECUTAR_SCRIPT,
	FINALIZAR_PROCESO,
	MULTIPROGRAMACION,
	INICIAR_PLANIFICACION,
	DETENER_PLANIFICACION
}instrucciones;

#endif