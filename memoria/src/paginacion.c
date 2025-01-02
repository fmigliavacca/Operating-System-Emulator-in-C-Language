#include <../include/paginacion.h>

//Iniciar la memoria (o sea el void* con todos los datos)
void* iniciarMemoria(t_valoresM valores)
{
    void* memoria = malloc(valores.tamanoMemoria);
    return memoria;
}
