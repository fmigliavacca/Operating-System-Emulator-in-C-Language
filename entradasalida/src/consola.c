//------------- BIBLIOTECAS --------------

#include <../include/consola.h>
   
//------------CONSOLA INTERACTIVA------------

void consola(int conexionKernel,int conexionMemoria, char* nombreInterfaz, char* pathDialfs, char* resultado, t_config* config, t_log* logger)
{
    log_info(logger, ">> %s ", nombreInterfaz);
    
    //cargo el config
    t_valores valores = cargarConfig(config);

    while (strcmp(nombreInterfaz,"") !=0 )
    {
        if(strcmp(valores.tipoInterfaz, "GENERICA") == 0)
        {
            log_info(logger, "Interfaz Generica Conectada ");

            //Envio el aviso a kernel que tengo una interfaz de tipo GENERICA
            enviarInterfaz(conexionKernel, nombreInterfaz, GENERICA);
            log_info(logger,"Interfaz %s enviada a kernel con el codigo de operacion #%d", nombreInterfaz, STDIN);
        }
            
        if (strcmp(valores.tipoInterfaz, "STDIN") == 0)
        {
            log_info(logger, "Interfaz STDIN Conectada ");

            //Envio el aviso a kernel que tengo una interfaz de tipo STDIN
            enviarInterfaz(conexionKernel, nombreInterfaz, STDIN);
            log_info(logger,"Interfaz %s enviada a kernel con el codigo de operacion #%d", nombreInterfaz, STDIN);
        }

        if (strcmp(valores.tipoInterfaz, "STDOUT") == 0)
        {
            log_info(logger, "Interfaz STDOUT Conectada ");

            //Envio la interfaz a kernel
            enviarInterfaz(conexionKernel, nombreInterfaz, STDOUT);
            log_info(logger,"Interfaz %s enviada a memoria con el codigo de operacion #%d", nombreInterfaz, STDOUT);
        }

        //Si se crea una interfaz DIALFS
        if(strcmp(valores.tipoInterfaz, "DIALFS") == 0)
        {
            log_info(logger, "DialFS conectada!");
            iniciarArchivos(resultado, pathDialfs, logger);
            enviarInterfaz(conexionKernel, nombreInterfaz, DIALFS);
            log_info(logger, "Interfaz dialFS enviada a kernel");
        }

        break;
    }
}