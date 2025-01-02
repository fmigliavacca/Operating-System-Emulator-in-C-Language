#ifndef ENTRADASALIDA_ESTRUCTURAS_H_
#define ENTRADASALIDA_ESTRUCTURAS_H_

#include <../../utils/include/utils.h>

//------------ESTRUCTURAS DE CONFIGURACIONES------------

//Estructura para leer el config
typedef struct
{
	int blockSize;
	int blockCount;
	int tiempoUnidadTrabajo;
	int retrasoCompactacion;
    char* ipKernel;
    char* ipMemoria;
	char* pathDialfs;
	char* tipoInterfaz;
	char* puertoKernel;
	char* puertoMemoria;
}t_valores;

typedef struct 
{
	int blockSize;
	int blockCount;
	int retrasoCompactacion;
}t_valores_bloques;

typedef struct 
{
	int tamanio;
	char* posicion;
	t_bitarray* bitarray;
}t_bitmap;

typedef struct 
{
	uint32_t bloqueInicial;
	uint32_t tamanioArchivo;
}t_metadata;

#endif
