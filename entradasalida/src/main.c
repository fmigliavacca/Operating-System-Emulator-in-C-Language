//------------BIBLIOTECAS------------

#include <../include/consola.h>
#include <../include/filesystem.h>
#include <../include/peticiones.h>
#include <../include/inicioYFin.h>
#include <../include/estructuras.h>
#include <../../utils/include/comunicacion.h>

//------------CODIGO MAIN INTERFAZ------------

int main(int argc, char** argv)
{  
    //Inicializo un logger
    t_log* logger = iniciarLogger();

    //Pido ingresar el nombre de la interfaz

    char* nombreInterfaz = readline(">> Ingrese nombre de la interfaz a iniciar: ");

    char* resultado = malloc(strlen(nombreInterfaz) + strlen(argv[1]) + strlen(".config") + 2);
    
    strcpy(resultado, argv[1]);
    strcat(resultado, "/");
    strcat(resultado, nombreInterfaz);
    strcat(resultado, ".config\0");

    //Inicializo el config y cargo sus valores       
    t_config* config = iniciarConfig(resultado);
    t_valores valores = cargarConfig(config);

    //Creo las conexiones
    int conexionKernel = crearConexion(valores.ipKernel, valores.puertoKernel, logger); 
    int conexionMemoria = crearConexion(valores.ipMemoria, valores.puertoMemoria, logger);

    //--------  PATH ---------

    char* pathDialfs = malloc(strlen(valores.pathDialfs) + strlen("/dialfs/") + 1);
    strcpy(pathDialfs, valores.pathDialfs);
    strcat(pathDialfs, "/dialfs/");

    //-------CONSOLA-------

    consola(conexionKernel, conexionMemoria, nombreInterfaz, pathDialfs, resultado, config, logger);

    //------------BUCLE INFINITO------------
    
    while(1)
    {
        esperaInfinita(pathDialfs, conexionKernel, conexionMemoria, config, logger);
    }

    //Libero recursos
    terminarPrograma(conexionKernel, conexionMemoria, logger, config);

    free(resultado);
    free(nombreInterfaz);

    return 0;
}