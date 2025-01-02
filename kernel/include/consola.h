#ifndef CONSOLA_H_
#define CONSOLA_H_

//------------BIBLIOTECAS------------

#include <../../utils/include/utils.h>
#include <../../utils/include/comunicacion.h>
#include <../../kernel/include/estructuras.h>
#include <../../kernel/include/planificador.h>

//------------DECLARACION DE FUNCIONES------------

void consola(argsPlanificacion*);
void procesoEstado(argsPlanificacion*);
void mostrarProcesos(t_log*, t_lista*);
void aumentarGrado(argsPlanificacion*, int);
void disminuirGrado(argsPlanificacion*, int);
void hallarProcesoEnColas(argsPlanificacion*, int);
void iniciarProceso(argsPlanificacion*, char*, int*);
void leerScript(argsPlanificacion*, char*, char*, int*);
void modificarMultiprogramacion(argsPlanificacion*, int);
void finalizarProcesoEspecifico(argsPlanificacion*, char*);
void ejecutarInstrucciones(argsPlanificacion*, char*, int*);
void finalizarProcesoSegunEstado(argsPlanificacion*, t_lista*, t_PCB*, colasEstado);
bool recorrerLaCola(argsPlanificacion*, t_lista*, int, colasEstado);
t_PCB* inicializarPCB(int*, t_valores);
instrucciones procesarInstrucciones(char*);

#endif