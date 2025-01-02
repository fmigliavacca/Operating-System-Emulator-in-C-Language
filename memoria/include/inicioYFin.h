#ifndef MEMORIA_INICIO_Y_FIN_H_
#define MEMORIA_INICIO_Y_FIN_H_

//------------BIBLIOTECAS------------

#include <../include/estructuras.h>
#include <../../utils/include/utils.h>
#include <../../utils/include/sockets.h>

//------------DECLARACION DE FUNCIONES------------

int crearConexiones(t_log*, t_valoresM, t_list*, void*, t_list*, t_bitarray*);
void terminarPrograma(int, t_log*, t_config*,void*);
t_log* iniciarLogger();
t_config* iniciarConfig(char*);
t_valoresM cargarConfig(t_config*);

#endif