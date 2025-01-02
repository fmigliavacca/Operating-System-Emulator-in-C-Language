#ifndef SOCKETS_MAIN_H_
#define SOCKETS_MAIN_H_

//------------BIBLIOTECAS------------

#include "utils.h"

//------------FUNCIONES------------

//Creacion de conexiones
int iniciarServidor(char*, t_log*);
int crearConexion(char*, char*, t_log*);

//Esperar/Escuchar al cliente
int esperarCliente(int, t_log*);
void* atenderCliente(void*);

//HILOS
void crearHiloEscucha(argsEsperarMensaje*);
bool crearHiloInterfaces(argsEsperarMensaje*);

#endif