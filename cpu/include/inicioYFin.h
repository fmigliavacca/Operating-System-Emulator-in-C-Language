#ifndef CPU_INICIO_Y_FIN_H_
#define CPU_INICIO_Y_FIN_H_

//------------BIBLIOTECAS------------

#include <../../utils/include/utils.h>
#include <../include/estructuras.h>
#include <../../utils/include/comunicacion.h>
#include <../../utils/include/sockets.h>

//------------DECLARACION DE FUNCIONES------------

int pedirTamPag(int);
void terminarPrograma(int, int, int, t_log*, t_config*, t_dictionary*, void*);
t_log* iniciarLogger(void);
t_config* iniciarConfig(char*);
t_valores cargarConfig(t_config*);

#endif