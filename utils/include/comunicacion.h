#ifndef COMUNICACION_MAIN_H_
#define COMUNICACION_MAIN_H_

//------------BIBLIOTECAS------------

#include "utils.h"
#include "serializacion.h"
#include <../include/serializacion.h>

//------------FUNCIONES------------

int recibirOperacion(int);
bool esIgualPID(void*);
void enviarPCB(int, t_PCB);
void enviarMensaje(char*, int);
void recibirMensaje(int, t_log*);
void eliminarPaquete(t_paquete*);
void enviarArchivoPseudo(int, char*);
void enviarPath(t_log*, char*, int, int);
void recibirPCB(int, argsEsperarMensaje*);
void recibirInterfaz(argsEsperarMensaje*);
void enviarConfirmacionPath(op_code, int);
void enviarInstruccion(t_instruccion, int);
void eliminarPaqueteStdin(t_paquete_stdin*);
void manejarInterrupcionQuantum (int, t_PCB*, int*);
void enviarInterfaz(int, char*, tipo_interfaz);
void enviarCadenaDeInstrucciones(t_list*, int);
void recibirPedidoInstruccion(argsEsperarMensaje*);
void finalizarProcesoEnMemoria(argsEsperarMensaje*);
void recibirPath(t_log*, t_list*, int, char*, t_list*);
void recibirPeticionTamanioPagina(argsEsperarMensaje*);
void manejarInterrupcionProcesoFinalizado (int, t_PCB*, int*);
void manejarPedidoInstruccion(t_peticionInstruccion, t_list*, int);
void* esperarMensajes(void*);
void* recibirBuffer(int*, int);
void* esperarMensajesMemoria(void*);
op_code recibirConfirmacionPath(int);
t_list* obtenerPseudoCodigo(t_log*, char*, char*); 

void recibirPID(int, u_int32_t*);

//Interfaces
int esperarInterfaz(t_log*, int);
bool enviarSleep(t_log*, t_lista*, char*, u_int32_t, u_int8_t);
bool enviarFS_CREATE(t_log*, t_lista*, char*, char*, uint32_t);
bool enviarFS_DELETE(t_log*, t_lista*, char*, char*, u_int32_t);
bool peticionSTDIN(t_log*, t_lista*, char*, t_peticion_std);
bool peticionSTDOUT(t_log*, t_lista*, char*, t_peticion_std);
bool enviarFS_TRUNCATE(t_log*, t_lista*, char*, char*, uint32_t, u_int32_t);
bool enviarFS_READ(t_log*, t_lista*, char*, char*, uint32_t, uint32_t, uint8_t, uint32_t);
bool enviarFS_WRITE(t_log*, t_lista* , char*, char*, uint32_t, uint32_t, uint8_t, uint32_t);
void recibirSleep(int, u_int8_t*, t_log*);
void enviarDireccionFisicaYPID(int , t_peticion_std);
void enviarInterfazStdin(int, t_peticion_std);
void recibirSTDIN(argsEsperarMensaje*);
void recibirSTDOUT(argsEsperarMensaje*);
t_peticion_fs recibirNombreYPID(int, t_log*);
t_peticion_fs recibirTruncate(int, t_log*);
t_peticion_fs recibirContenidoDialfs(int,t_log*);
t_peticion_std recibirContenido(int, t_log*);
t_peticion_std recibirDireccionFisicaYPID(int, t_log*);
t_elemento_interfaces* buscarInterfacesPorNombre(char*, t_lista*);

//Memoria
int tamanioProceso(t_list*, int);
int cantidadDeFramesLibres(t_bitarray*);
void hacerFree(void*);
void hacerFreeListaInts(int*);
void hacerFreePaginas(t_pagina*);
void recibirPedidoMarco(int , t_list*);
void ocuparFrames(t_bitarray*, t_list*);
void guardarDato(int, void*, void*, int);
void desOcuparFrames(t_bitarray*, t_list*);
void hacerFreeProceso(t_direccionProceso*);
void hacerFreeTablaPaginas(t_tabla_paginas*);
void hacerFreeListaInstrucciones(t_instruccion*);
void recibirResize(int, t_bitarray*, t_list*, t_log*, int);
void accederAEspacioUsuario(int, int, t_list*, t_log*, int, void*);
t_list* framesLibres(t_bitarray*);
t_list* framesDelProceso(t_list*, int);
t_list* devolverPaginasDesde(t_list*, int, int);
t_pagina* buscarPaginaPorFrame(t_list*, int);
t_pagina* buscarPaginaPorNumero(t_list*, int);
t_bitarray* iniciarBitArray(int, int, char*, t_log*);
t_peticion_acceso_eu deserializarPeticionAccesoEu(void*);

#endif