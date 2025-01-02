#ifndef KERNEL_INICIO_Y_FIN_H_
#define KERNEL_INICIO_Y_FIN_H_

//------------BIBLIOTECAS------------

#include <../include/estructuras.h>
#include <../../utils/include/sockets.h>
#include <../../kernel/include/planificador.h>

//------------DECLARACION DE FUNCIONES------------

void terminarPrograma(int, int, int, int, t_log*, t_config*);
t_log* iniciarLogger(void);
t_config* iniciarConfig(char*);
t_valores cargarConfig(t_config*);
t_lista* crearListaRecursos(t_valores);
t_conexiones crearConexiones(t_valores, t_log*, argsEsperarMensaje*);
argsPlanificacion* crearHiloPlanificador(t_log*, t_valores, t_conexiones, argsEsperarMensaje*);

#endif