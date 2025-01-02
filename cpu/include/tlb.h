#ifndef CPU_TLB_H_
#define CPU_TLB_H_

//------------BIBLIOTECAS------------

#include <../include/estructuras.h>
#include <../include/estructuras.h>
#include <../../utils/include/utils.h>
#include <../../utils/include/comunicacion.h>

//------------DECLARACION DE FUNCIONES------------

int pedirMarcoMemoria(int, int, int);
int obtenerEntradaMasVieja(void*,int);
int calcularNumMarco(int, void*, int, int, char*, int, t_log*);
void* crearTlb(int);
void* serailizarPIDYNumPag(int, int);
void sumarAging(void*, int);
void reiniciarAgingUsado(int, void*);
void enviarPIDYNumPag(int, int, int);
void algoritmoFIFO(void*, int, int, int, int);
void correrAlgoritmoTlb(void*, char*, int, int, int, int, int);

#endif