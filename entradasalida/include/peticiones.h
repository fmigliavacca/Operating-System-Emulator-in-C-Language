#ifndef PETICIONES_H
#define PETICIONES_H_

//------------BIBLIOTECAS------------

#include <../include/inicioYFin.h>
#include <../include/filesystem.h>
#include <../../utils/include/utils.h>
#include <../../utils/include/comunicacion.h>

//------------DECLARACION DE FUNCIONES------------

int esperarConfirmacionMemoria(t_log*, int);
void confirmarInterfaz(int, int);
void ejecutarSleep(t_log*, int, int);
void ejecutarSTDIN(t_log*, int, int);
void ejecutarSTDOUT(t_log*, int, int);
void recibirContenidoSTDOUT(t_log*, int);
void ejecutarFS_CREATE(t_log*, int, t_valores, char*);
void ejecutarFS_DELETE(t_log*, int, t_valores, char*);
void ejecutarFS_TRUNCATE(t_log*, int, t_valores, char*);
void esperaInfinita(char*, int, int, t_config*, t_log*);

#endif