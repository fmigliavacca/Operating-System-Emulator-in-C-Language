//------------- BIBLIOTECAS --------------

#include <../include/peticiones.h>

//-------------FUNCIONES-------------

//Funcion para confirmar que la interfaz completo su trabajo(1 = correctamente, 0 = incorrectamente)
void confirmarInterfaz(int socket, int confirmacion)
{
    send(socket, &confirmacion, sizeof(int), 0);
}

//Funcion para esperar a que memoria confirme que pudo realizar con exito la operacion
int esperarConfirmacionMemoria(t_log* logger, int conexionMemoria)
{
    int confirmacion;

    recv(conexionMemoria, &confirmacion, sizeof(int), MSG_WAITALL);

   return confirmacion;
}

//Funcion para recibir y mostrar en pantalla una STDOUT
void recibirContenidoSTDOUT(t_log* logger, int socket)
{
    t_peticion_std aux;

	recv(socket, &aux.tamanio, sizeof(uint8_t), MSG_WAITALL);

    //Le agrego un \0 al final de la cadena para que se escriba correctamente
    aux.cadena = malloc(aux.tamanio + 1);
    aux.cadena[aux.tamanio] = '\0';
    
    recv(socket, aux.cadena, aux.tamanio, MSG_WAITALL);

	log_info(logger, ">>%s", aux.cadena);
    
    free(aux.cadena);
}

//Funcion para recibir el nombre de un archivo
char* recibirNombreArchivo(int conexionKernel)
{
    //Recibo la longitud del nombre
    int longitud;
    
    if(recv(conexionKernel, &longitud, sizeof(int), MSG_WAITALL) <= 0){
        return NULL;
    }
    
    //Uso la longitud para reservar espacio en memoria y recibo el nombre
    char* nombre = malloc(longitud);
    
    if(recv(conexionKernel, nombre, longitud, MSG_WAITALL) <= 0 ){
        free(nombre);
        return NULL;
    }

    return nombre;
}

//Funcion para recibir contenido FS
char* recibirContenidoFS(t_log* logger, int socket)
{
    t_peticion_std aux;

	recv(socket, &aux.tamanio, sizeof(uint8_t), MSG_WAITALL);

    aux.cadena = malloc(aux.tamanio);

    recv(socket, aux.cadena, aux.tamanio, MSG_WAITALL);

	return aux.cadena;
}

//Funciones para ejecutar cada instruccion
void ejecutarSleep(t_log* logger, int conexionKernel, int tiempoUnidadTrabajo)
{
    uint32_t PID;
    recibirPID(conexionKernel, &PID);

    log_info(logger, "PID: %d - Operacion: SLEEP", PID);

    //Recibo la cantidad de unidades
    uint8_t unidades;
    recibirSleep(conexionKernel, &unidades, logger);

    //Calculo la cantidad total de tiempo a esperar
    int tiempoAEsperar = unidades * tiempoUnidadTrabajo;

    //Hago el sleep
    usleep(tiempoAEsperar);

    //Confirmo la operacion como exitosa
    confirmarInterfaz(conexionKernel, 1);
}

void ejecutarSTDIN(t_log* logger, int conexionKernel, int conexionMemoria)
{
    t_peticion_std peticion = recibirDireccionFisicaYPID(conexionKernel, logger);

    log_info(logger, "PID: %d - Operacion: STDIN", peticion.PID);

    //Una vez recibida la direccion fisica de memoria que nos manda kernel, ingreso el texto por teclado para enviarlo a memoria
    char* texto = readline(">> Ingrese el texto de la interfaz STDIN: ");
    memcpy(peticion.cadena, texto, peticion.tamanio);

    enviarInterfazStdin(conexionMemoria, peticion);

    confirmarInterfaz(conexionKernel, esperarConfirmacionMemoria(logger, conexionMemoria));

    free(texto);
    free(peticion.cadena);
}

void ejecutarSTDOUT(t_log* logger, int conexionKernel, int conexionMemoria)
{
    t_peticion_std peticion = recibirDireccionFisicaYPID(conexionKernel, logger);

    log_info(logger, "PID: %d - Operacion: STDOUT", peticion.PID);
    
    enviarDireccionFisicaYPID(conexionMemoria, peticion);

    int confirmacion = esperarConfirmacionMemoria(logger, conexionMemoria);

    recibirContenidoSTDOUT(logger, conexionMemoria);

    confirmarInterfaz(conexionKernel, confirmacion);

    free(peticion.cadena);
}

void ejecutarFS_CREATE(t_log* logger, int conexionKernel, t_valores valores, char* pathDialfs)
{
    //Recibo el PID del proceso que solicitante
    uint32_t PIDCrear;
    recibirPID(conexionKernel, &PIDCrear);

    //Recibo el nombre del archivo
    char* nombreArchivoACrear = recibirNombreArchivo(conexionKernel);

    log_info(logger, "PID: %d - Crear Archivo: %s", PIDCrear, nombreArchivoACrear);

    //Hago sleep necesario
    usleep(valores.tiempoUnidadTrabajo);

    //Creo el archivo en cuestion
    crearArchivo(pathDialfs, nombreArchivoACrear, logger);

    //Confirmo la accion
    confirmarInterfaz(conexionKernel, 1);
}

void ejecutarFS_DELETE(t_log* logger, int conexionKernel, t_valores valores, char* pathDialfs)
{
    //Recibo el PID del proceso que solicitante
    uint32_t PIDBorrar;
    recibirPID(conexionKernel, &PIDBorrar);

    //Recibo el nombre del archivo
    char* nombreArchivoABorrar = recibirNombreArchivo(conexionKernel);

    log_info(logger, "PID: %d - Eliminar Archivo: %s", PIDBorrar, nombreArchivoABorrar);

    //Hago usleep necesario
    usleep(valores.tiempoUnidadTrabajo);

    //Creo el archivo en cuestion
    borrarArchivo(pathDialfs, nombreArchivoABorrar, logger, valores.blockSize);

    //Confirmo la accion
    confirmarInterfaz(conexionKernel, 1);

    free(nombreArchivoABorrar);
}

void ejecutarFS_TRUNCATE(t_log* logger, int conexionKernel, t_valores valores, char* pathDialfs)
{
    //Recibo el PID del proceso que solicitante
    uint32_t PIDTruncar;
    recibirPID(conexionKernel, &PIDTruncar);

    //Recibo el nombre del archivo
    char* nombreArchivoATruncar = recibirNombreArchivo(conexionKernel);

    //Recibo el nuevo tamanio del archivo
    uint32_t tamanioNuevo;
    recv(conexionKernel, &tamanioNuevo, sizeof(uint32_t), MSG_WAITALL);
            
    log_info(logger, "PID: %d - Truncar Archivo: %s - Tamaño: %d", PIDTruncar, nombreArchivoATruncar, tamanioNuevo);

    //Hago usleep necesario
    usleep(valores.tiempoUnidadTrabajo);
    
    //Creo el archivo en cuestion
    truncarArchivo(pathDialfs, nombreArchivoATruncar, valores.blockSize, tamanioNuevo, valores.retrasoCompactacion, logger);

    //Confirmo la accion
    confirmarInterfaz(conexionKernel, 1);

    free(nombreArchivoATruncar);
}

void ejecutarFS_READ(t_log* logger, int conexionKernel, int conexionMemoria,char* pathDialfs, char* nombreArchivo, int blockSize)
{
    t_peticion_std peticion = recibirDireccionFisicaYPID(conexionKernel, logger);

    uint32_t regPuntero;
    recv(conexionKernel, &regPuntero, sizeof(uint32_t), MSG_WAITALL);
    
    log_info(logger, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d", peticion.PID, nombreArchivo, peticion.tamanio, regPuntero);

    //Una vez recibida la direccion fisica de memoria que nos manda kernel, ingreso el texto por teclado para enviarlo a memoria
    void* contenido = leerArchivo(pathDialfs, nombreArchivo, regPuntero, peticion.tamanio, blockSize, logger);
    memcpy(peticion.cadena, (char*)contenido, peticion.tamanio);

    enviarInterfazStdin(conexionMemoria, peticion);

    confirmarInterfaz(conexionKernel, esperarConfirmacionMemoria(logger, conexionMemoria));

    free(contenido);
    free(peticion.cadena);
}

void ejecutarFS_WRITE(t_log* logger, int conexionKernel, int conexionMemoria, char* pathDialfs, char* nombreArchivo, int blockSize)
{
    t_peticion_std peticion = recibirDireccionFisicaYPID(conexionKernel, logger);

    uint32_t regPuntero;
    recv(conexionKernel, &regPuntero, sizeof(uint32_t), MSG_WAITALL);

    log_info(logger, "PID: %d - Escribir Archivo: %s - Tamaño a Escribir: %d - Puntero Archivo: %d", peticion.PID, nombreArchivo, peticion.tamanio, regPuntero);

    enviarDireccionFisicaYPID(conexionMemoria, peticion);

    int confirmacion = esperarConfirmacionMemoria(logger, conexionMemoria);

    char* contenido = recibirContenidoFS(logger, conexionMemoria);

    //Escribo en el archivo en cuestion
    escribirArchivo(pathDialfs, nombreArchivo, regPuntero, peticion.tamanio, contenido, blockSize, logger);

    confirmarInterfaz(conexionKernel, confirmacion);

    free(contenido);
    free(peticion.cadena);
}

//Bucle infinito para recibir peticiones
void esperaInfinita(char* pathDialfs, int conexionKernel, int conexionMemoria, t_config* config, t_log* logger)
{
    t_valores valores = cargarConfig(config);

    //Espero a recibir un codigo de operacion
    tipoDePeticion codOp = recibirOperacion(conexionKernel);
    
    switch(codOp)
    {
        case PETICION_GENERICA:
            //Realizo el sleep
            ejecutarSleep(logger, conexionKernel, valores.tiempoUnidadTrabajo);
            break;

        case PETICION_STDIN:
            //Realizo la STDIN
            ejecutarSTDIN(logger, conexionKernel, conexionMemoria);
            break;

        case PETICION_STDOUT:
            //Realizo la STDOUT
            ejecutarSTDOUT(logger, conexionKernel, conexionMemoria);
            break;

        case CREAR_ARCHIVOFS:
            ejecutarFS_CREATE(logger, conexionKernel, valores, pathDialfs);
            break;

        case BORRAR_ARCHIVOFS:
            ejecutarFS_DELETE(logger, conexionKernel, valores, pathDialfs);
            break;

        case TRUNCAR_ARCHIVO:
            ejecutarFS_TRUNCATE(logger, conexionKernel, valores, pathDialfs);
            break;

        case ESCRIBIR_ARCHIVO:
            //Recibo el nombre del archivo
            char* nombreArchivoAEscribir = recibirNombreArchivo(conexionKernel);

            //Hago usleep necesario
            usleep(valores.tiempoUnidadTrabajo);

            ejecutarFS_WRITE(logger, conexionKernel, conexionMemoria, pathDialfs, nombreArchivoAEscribir, valores.blockSize);

            free(nombreArchivoAEscribir);
            break;

        case LEER_ARCHIVO:
            //Recibo el nombre del archivo
            char* nombreArchivoALeer = recibirNombreArchivo(conexionKernel);

            //Hago usleep necesario
            usleep(valores.tiempoUnidadTrabajo);
            
            //Escribo en el archivo en cuestion
            ejecutarFS_READ(logger, conexionKernel, conexionMemoria, pathDialfs, nombreArchivoALeer, valores.blockSize);

            free(nombreArchivoALeer);
            break;

        default:
        
            log_info(logger, "El tipo de peticion realizado es invalido");

            break;
    }
}