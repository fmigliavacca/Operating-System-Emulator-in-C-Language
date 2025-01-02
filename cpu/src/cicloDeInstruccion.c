//------------BIBLIOTECAS------------

#include <../include/cicloDeInstruccion.h>

//--------------SERIALIZACION---------------

//Funcion para serializar las instrucciones IO_FS_WRITE o IO_FS_READ
void* serializarIO_FS_WRITEoIO_FS_READ(t_operandos operandos, uint32_t direccionLogica, uint32_t tamanoALeer, uint32_t punteroArchivo, tipoInstruccionInterfaz tipoInstruccion)
{   

    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo + sizeof(uint32_t) * 3);

    //Creo una variable para almacenar el tipo de insterfaz y otra para almacenar el tipo de instruccion, y asi pasarselas en el memcpy
    tipo_interfaz interfaz = DIALFS;

    //Copio en el stream los valores de las unidades de trabajo y el enum de la interfaz
    memcpy(stream, &interfaz, sizeof(tipo_interfaz));
    memcpy(stream + sizeof(tipo_interfaz), &longitudPrimero, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int), operandos.operando1, longitudPrimero);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero, &tipoInstruccion, sizeof(tipoInstruccionInterfaz));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(tipoInstruccionInterfaz), &longitudSegundo, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(tipoInstruccionInterfaz) + sizeof(int), operandos.operando2, longitudSegundo);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudSegundo, &direccionLogica, sizeof(uint32_t));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudSegundo + sizeof(uint32_t) * 1, &tamanoALeer, sizeof(uint32_t));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudSegundo + sizeof(uint32_t) * 2, &punteroArchivo, sizeof(uint32_t));

    //Retrono el stream
    return stream;
}

//Funcion para enviar la instruccion IO_FS_WRITE o IO_FS_READ a kernel
void enviarIO_FS_WRITEoIO_FS_READ(t_operandos operandos, int conexionDispatch, uint32_t direccionLogica, uint32_t tamanoALeer, uint32_t punteroArchivo, tipoInstruccionInterfaz instruccionInterfaz)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    int longitudTotal = sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo + sizeof(uint32_t) * 3;

    //Serailiza la interfaz y sus unidades de trabajo
    void* streamEnviar = serializarIO_FS_WRITEoIO_FS_READ(operandos, direccionLogica, tamanoALeer, punteroArchivo, instruccionInterfaz);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar las instrucciones IO_FS_TRUNCATE
void* serializarIO_FS_TRUNCATE(t_operandos operandos, uint32_t tamano)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo + sizeof(uint32_t));   

    //Creo una variable para almacenar el tipo de insterfaz y otra para almacenar el tipo de instruccion, y asi pasarselas en el memcpy
    tipo_interfaz interfaz = DIALFS;
    tipoInstruccionInterfaz tipoInstruccion = FS_TRUNCATE;

    //Copio en el stream los valores de las unidades de trabajo y el enum de la interfaz
    memcpy(stream, &interfaz, sizeof(tipo_interfaz));
    memcpy(stream + sizeof(tipo_interfaz), &longitudPrimero, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int), operandos.operando1, longitudPrimero);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero, &tipoInstruccion, sizeof(tipoInstruccionInterfaz));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero, &longitudSegundo, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int), operandos.operando2, longitudSegundo);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo, &tamano, sizeof(uint32_t));

    //Retrono el stream
    return stream;
}

//Funcion para enviar la instruccion IO_FS_TRUNCATE a kernel
void enviarIO_FS_TRUNCATE(t_operandos operandos, int conexionDispatch, uint32_t tamano)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    int longitudTotal = sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo + sizeof(uint32_t);

    //Serailiza la interfaz y sus unidades de trabajo
    void* streamEnviar = serializarIO_FS_TRUNCATE(operandos, tamano);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar las instrucciones IO_FS_CREATE o IO_FS_DELETE
void* serializarIO_FS_CREATEoIO_FS_DELETE(t_operandos operandos, tipoInstruccionInterfaz tipoInstruccion)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo);   

    //Creo una variable para almacenar el tipo de insterfaz y pasarla en el memcpy
    tipo_interfaz interfaz = DIALFS;

    //Copio en el stream los valores de las unidades de trabajo y el enum de la interfaz
    memcpy(stream, &interfaz, sizeof(tipo_interfaz));
    memcpy(stream + sizeof(tipo_interfaz), &longitudPrimero, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int), operandos.operando1, longitudPrimero);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero, &tipoInstruccion, sizeof(tipoInstruccionInterfaz));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero, &longitudSegundo, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int), operandos.operando2, longitudSegundo);

    //Retrono el stream
    return stream;

}

//Funcion para enviar la instruccion IO_FS_CREATE o IO_FS_DELETE a kernel
void enviarIO_FS_CREATEoIO_FS_DELETE(t_operandos operandos, int conexionDispatch, tipoInstruccionInterfaz tipoInstruccion)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    int longitudTotal = sizeof(tipo_interfaz) + sizeof(tipoInstruccionInterfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo;

    //Serailiza la interfaz y sus unidades de trabajo
    void* streamEnviar = serializarIO_FS_CREATEoIO_FS_DELETE(operandos, tipoInstruccion);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para enviar la instruccion COPY_STRING a memoria
void enviarCOPY_STRING(int tamanoString, uint32_t direcFisicaSI, uint32_t direcFisicaDI, uint32_t pid, int conexionMemoria, uint8_t tamanoDato, t_log* logger)
{
    //Calculo la longitud total del stream
    int longitudTotalMOV_IN = sizeof(op_code) + sizeof(uint32_t) * 2 + sizeof(uint8_t);

    //Creo el stream a enviar
    void* streamEnviarMOV_IN = malloc(longitudTotalMOV_IN);

    //Le asigno un codigo de operacion
    op_code codOp = PEDIDO_MOV_IN;

    //Copio en el stream los valores
    memcpy(streamEnviarMOV_IN, &codOp, sizeof(op_code));
    memcpy(streamEnviarMOV_IN + sizeof(op_code), &pid, sizeof(uint32_t)); 
    memcpy(streamEnviarMOV_IN + sizeof(op_code) + sizeof(uint32_t), &direcFisicaSI, sizeof(uint32_t));
    memcpy(streamEnviarMOV_IN + sizeof(op_code) + sizeof(uint32_t) * 2, &tamanoDato, sizeof(uint8_t));

    //Envia los datos mediante la conexionDispatch
    send(conexionMemoria, streamEnviarMOV_IN, longitudTotalMOV_IN, 0);

    if(recibirOperacion(conexionMemoria) != 1)
    {
        perror("Error en instruccion COPY_STRING");
        exit(EXIT_FAILURE);
    }

    recv(conexionMemoria, &tamanoDato, sizeof(uint8_t), MSG_WAITALL);

    //Le agrego un \0 al final de la cadena para que se escriba correctamente
    char* dato = malloc(tamanoDato + 1);
    dato[tamanoDato] = '\0';
    
    recv(conexionMemoria, dato, tamanoDato, MSG_WAITALL);

    //Log obligatorio
    log_info(logger, "PID: %d - Acción: LECTURA - Dirección Física: %d - Valor: %s", pid, direcFisicaSI, dato);

    //Calculo la direccion del stream de mov out a enviar
    int longitudTotalMOV_OUT = sizeof(op_code) + sizeof(uint32_t) * 2 + sizeof(uint8_t) + tamanoDato;

    //Reservo espacio necesario para el stream de mov out
    void* streamEnviarMOV_OUT = malloc(longitudTotalMOV_OUT);

    //Le asigno un codigo de operacion
    codOp = PEDIDO_MOV_OUT;

    //Copio en el stream los valores
    memcpy(streamEnviarMOV_OUT, &codOp, sizeof(op_code));
    memcpy(streamEnviarMOV_OUT +  sizeof(op_code), &pid, sizeof(uint32_t)); 
    memcpy(streamEnviarMOV_OUT + sizeof(op_code) + sizeof(uint32_t), &direcFisicaDI, sizeof(uint32_t));
    memcpy(streamEnviarMOV_OUT + sizeof(op_code) + sizeof(uint32_t) * 2, &tamanoDato, sizeof(uint8_t));
    memcpy(streamEnviarMOV_OUT + sizeof(op_code) + sizeof(uint32_t) * 2 + sizeof(uint8_t), dato, tamanoDato);

    //Envia los datos mediante la conexionDispatch
    send(conexionMemoria, streamEnviarMOV_OUT, longitudTotalMOV_OUT, 0);

    //Contemplo el error
    if(recibirOperacion(conexionMemoria) != 1)
    {
        perror("Error en instruccion COPY_STRING");
        exit(EXIT_FAILURE);
    }
    //Log obligatorio
    log_info(logger, "PID: %d - Acción: ESCRITURA - Dirección Física: %d - Valor: %s", pid, direcFisicaDI, dato);

    //Libero recursos
    free(streamEnviarMOV_IN);
    free(streamEnviarMOV_OUT);
    free(dato);
}

//Funcion para serializar las instrucciones WAIT o SIGNAL
void* serializarWAITOSIGNAL(char* recurso, tipoInstruccionesRecursos WAIToSIGNAL, int tamanioRecurso)
{   
    //Reservo espacio de memoria para mandar los datos
    void* stream = malloc(sizeof(int) + tamanioRecurso + sizeof(tipoInstruccionesRecursos));

    //Copio en el stream los valores
    memcpy(stream, &WAIToSIGNAL, sizeof(tipoInstruccionesRecursos));
    memcpy(stream + sizeof(tipoInstruccionesRecursos), &tamanioRecurso, sizeof(int));
    memcpy(stream + sizeof(tipoInstruccionesRecursos) + sizeof(int), recurso, tamanioRecurso);

    //retorno el stream
    return stream;
}

//Funcion para enviar la instruccion SIGNAL o WAIT a kernel
void enviarWAITOSIGNAL(char* recurso, int conexionDispatch, tipoInstruccionesRecursos SIGNALoWAIT)
{
    //Calculo el tamanio del recurso
    int tamanioRecurso = strlen(recurso) + 1;

    //Calculo el tamanio del recurso
    int longitudTotal = sizeof(tipoInstruccionesRecursos) + sizeof(int) + tamanioRecurso;

    //Creo el stream a enviar
    void* streamEnviar = serializarWAITOSIGNAL(recurso, SIGNALoWAIT, tamanioRecurso);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar la instruccion MOV_OUT que debe enviarse a memoria
void* serializarMOV_OUT(uint8_t tamanioDato, int direcFisica, int dato, uint32_t pid)
{
   //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(uint8_t) + sizeof(int) + sizeof(op_code) + sizeof(uint32_t) + tamanioDato);

    //Le asigno un codigo de operacion
    op_code codOp = PEDIDO_MOV_OUT;

    //Copio en el stream los valores
    memcpy(stream, &codOp, sizeof(op_code));
    memcpy(stream +  sizeof(op_code), &pid, sizeof(uint32_t)); 
    memcpy(stream + sizeof(uint32_t) + sizeof(op_code), &direcFisica, sizeof(int));
    memcpy(stream + sizeof(uint32_t) + sizeof(op_code) + sizeof(int), &tamanioDato, sizeof(uint8_t));
    memcpy(stream + sizeof(uint32_t) + sizeof(op_code) + sizeof(int) + sizeof(uint8_t), &dato, tamanioDato);

    //retorno el stream
    return stream; 
}

//Funcion para enviar la instruccion MOV_OUT a memoria
void enviarMOV_OUT(uint8_t tamanioDato, int direcFisica, int dato, int conexionMemoria, uint32_t pid)
{
    //Calculo la longitud total del stream
    int longitudTotal = sizeof(int) + sizeof(uint8_t) + tamanioDato + sizeof(op_code) + sizeof(uint32_t);

    //Creo el stream a enviar
    void* streamEnviar = serializarMOV_OUT(tamanioDato, direcFisica, dato, pid);

    //Envia los datos mediante la conexionDispatch
    send(conexionMemoria, streamEnviar, longitudTotal, 0);

    //Contemplo el error
    if(recibirOperacion(conexionMemoria) != 1)
    {
        perror("Error en instruccion MOV_OUT");
        exit(EXIT_FAILURE);
    }

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar la instruccion MOV_IN que debe enviarse a memoria
void* serializarMOV_IN(uint8_t tamanioDato, int direcFisica, uint32_t pid)
{
   //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(uint8_t) + sizeof(int) + sizeof(op_code) + sizeof(uint32_t));

    //Le asigno un codigo de operacion
    op_code codOp = PEDIDO_MOV_IN;

    //Copio en el stream los valores
    memcpy(stream, &codOp, sizeof(op_code));
    memcpy(stream +  sizeof(op_code), &pid, sizeof(uint32_t)); 
    memcpy(stream + sizeof(op_code) + sizeof(uint32_t), &direcFisica, sizeof(int));
    memcpy(stream + sizeof(op_code) + sizeof(uint32_t) + sizeof(int), &tamanioDato, sizeof(uint8_t));

    //retorno el stream
    return stream; 
}

//Funcion para enviar la instruccion MOV_IN a memoria
int enviarMOV_IN(uint8_t tamanioDirecFisica, int direcFisica, int conexionMemoria, uint32_t pid)
{
    //Calculo la longitud total del stream
    int longitudTotal = sizeof(int) + sizeof(uint8_t) + sizeof(op_code) + sizeof(uint32_t);

    //Creo el stream a enviar
    void* streamEnviar = serializarMOV_IN(tamanioDirecFisica, direcFisica, pid);

    //Envia los datos mediante la conexionDispatch
    send(conexionMemoria, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);

    //Recibo el dato
    if(recibirOperacion(conexionMemoria) == 1)
    {   
        int dato;

        uint8_t tamanoDato;

        recv(conexionMemoria, &tamanoDato, sizeof(uint8_t), MSG_WAITALL);
        recv(conexionMemoria, &dato, tamanoDato, MSG_WAITALL);

        return dato;
    }

    else
    {
        perror("Error al recibir el dato de memoria en la instruccion MOV_IN");
        exit(EXIT_FAILURE);    
    }

}

//Funcion para serializar la instruccion RESIZE que debe enviarse a memoria
void* serializarRESIZE(uint32_t pid, int tamanioProceso)
{
    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(uint32_t) + sizeof(int) + sizeof(op_code));

    //Le asigno un codigo de operacion
    op_code codOp = HACER_RESIZE;

    //Copio en el stream los valores
    memcpy(stream, &codOp, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &pid, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(uint32_t), &tamanioProceso, sizeof(int));

    //retorno el stream
    return stream;
}

//Funcion para enviar la instruccion RESIZE a memoria
void enviarRESIZE(uint32_t pid, int tamanioProceso, int conexionMemoria)
{
    //Calculo la longitud total del stream
    int longitudTotal = sizeof(int) + sizeof(uint32_t) + sizeof(op_code);

    //Creo el stream a enviar
    void* streamEnviar = serializarRESIZE(pid, tamanioProceso);

    //Envia los datos mediante la conexionDispatch
    send(conexionMemoria, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar las instrucciones IO_STDIN_READ y IO_STDOUT_WRITE que debe enviarse a kernel
void* serializarSTDINOSTDOUT(tipo_interfaz interfaz, char* nombreInterfaz, uint32_t direccionFisica , uint32_t tamanoRegistro) 
{
    //Obtengo la longitud del nombre de la interfaz
    int longitudNombreInterfaz = strlen(nombreInterfaz) +1;

    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(tipo_interfaz) + sizeof(int) + longitudNombreInterfaz + sizeof(uint32_t) * 2);

    //Copio en el stream los valores
    memcpy(stream, &interfaz, sizeof(tipo_interfaz));
    memcpy(stream + sizeof(tipo_interfaz), &longitudNombreInterfaz, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int), nombreInterfaz, longitudNombreInterfaz);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudNombreInterfaz, &direccionFisica, sizeof(uint32_t));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudNombreInterfaz + sizeof(uint32_t), &tamanoRegistro, sizeof(uint32_t));

    //retorno el stream
    return stream;
}

//Funcion para enviar a kernel las instrucciones IO_STDIN_READ y IO_STDOUT_WRITE
void enviarSTDINOSTDOUT(tipo_interfaz interfaz, char* nombreInterfaz, uint32_t direccionFisica , uint32_t tamanoRegistro, int conexionDispatch)
{
    //Obtengo la longitud del nombre de la interfaz
    int longitudNombreInterfaz = strlen(nombreInterfaz) +1;

    int longitudTotal = sizeof(tipo_interfaz) + sizeof(int) + longitudNombreInterfaz + sizeof(uint32_t) * 2;

    //Creo el stream a enviar
    void* streamEnviar = serializarSTDINOSTDOUT(interfaz, nombreInterfaz, direccionFisica , tamanoRegistro);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar la instruccion IO_GEN_SLEEP que debe enviarse a kernel
void* serializarIO_GEN_SLEEP(t_operandos operandos, tipo_interfaz interfaz)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) +1;
    int longitudSegundo = strlen(operandos.operando2) +1;

    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo);   

    //Copio en el stream los valores de las unidades de trabajo y el enum de la interfaz
    memcpy(stream, &interfaz, sizeof(tipo_interfaz));
    memcpy(stream + sizeof(tipo_interfaz), &longitudPrimero, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int), operandos.operando1, longitudPrimero);
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero, &longitudSegundo, sizeof(int));
    memcpy(stream + sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(int), operandos.operando2, longitudSegundo);

    //Retrono el stream
    return stream;
}

//Funcion para enviar a kernel el tipo de intefaz y las unidades de trabajo que debe hacer sleep entradaysalida
void  enviarIO_GEN_SLEEP(t_operandos operandos, tipo_interfaz interfaz, int conexionDispatch)
{
    //Obtengo la lontigud de los operandos
    int longitudPrimero = strlen(operandos.operando1) + 1;
    int longitudSegundo = strlen(operandos.operando2) + 1;

    int longitudTotal = sizeof(tipo_interfaz) + sizeof(int) + longitudPrimero + sizeof(int) + longitudSegundo;

    //Serailiza la interfaz y sus unidades de trabajo
    void* streamEnviar = serializarIO_GEN_SLEEP(operandos, interfaz);

    //Envia los datos mediante la conexionDispatch
    send(conexionDispatch, streamEnviar, longitudTotal, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para serializar PC y PID
void* serializarPCyPID(uint32_t PC, uint32_t PID)
{
    //Reservo espacio necesario para el stream
    void* stream = malloc(sizeof(op_code) + sizeof(uint32_t) * 2);

    //Le digo su codigo de operacion
    op_code cop = PETICION_INSTRUCCION;

    //Copio en el stream todos los valores del PC y PID serializados
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &PC, sizeof(uint32_t));
    memcpy(stream + sizeof(op_code) + sizeof(uint32_t), &PID, sizeof(uint32_t));

    //Retorno el stream
    return stream;
}

//Funcion para enviar el PC y PID
void enviarPCyPID(t_PCB pcb, int conexionMemoria, t_log* logger)
{
    //Serializa el PC y el PID
    void* streamEnviar = serializarPCyPID(pcb.registrosDeCPU.PC, pcb.PID);

    log_info(logger,"PID: %i - FETCH - Program Counter: %i", pcb.PID, pcb.registrosDeCPU.PC);

    //Envia los datos a memroia
    send(conexionMemoria, streamEnviar, sizeof(op_code) + sizeof(uint32_t) * 2, 0);

    //Libero el stream
    free(streamEnviar);
}

//Funcion para tranformar el stream de la instruccion en un struct del tipo t_instruccionEntera
t_instruccionEntera* separarInstruccion(char* instruccionCompleta)
{
    //Creo la estructura de la instruccion con los operandos
    t_instruccionEntera* instruccionEnteraFinal = malloc(sizeof(t_instruccionEntera));

    //Copio el char instruccion completa en otro que hace un malloc
    char* line = strdup(instruccionCompleta);

    //Cargo la estructura con la instruccion y los operandos
    instruccionEnteraFinal->instruccion = strtok(line," ");
    instruccionEnteraFinal->operandos = strtok(NULL,"");

    //Libero el stream que recibi de memoria
    free(instruccionCompleta);

    //Retorno la estructura cargada
    return instruccionEnteraFinal;
}

char* recibirStreamInstruccion(int conexionMemoria, t_log* logger)
{    
    //Verifico que el codigo de operacion sea correcto y devuelve error en caso de no serlo
    if(recibirOperacion(conexionMemoria) != RECEPCION_INSTRUCCION)
    {
        perror("Error en el fetch de la instruccion");
        exit(EXIT_FAILURE);
    }

    //Creo un entero para guardar el tamaño de la intruccion
    int tamano;

    //Recibe el tamano de la instruccion
    recv(conexionMemoria, &tamano, sizeof(int), MSG_WAITALL);
    
    //Reserva espacio 
    char* instruccionCompleta = malloc(tamano);

    //Recibe la instruccion
    recv(conexionMemoria, instruccionCompleta, tamano, MSG_WAITALL);

    //Devuelvo el string de la instruccion
    return instruccionCompleta;
}

void* serializarPCBConCodOp(t_PCB pcb, op_code codigoOperacion)
{
    //Reservo el espacio necesario para el stream
	void* stream = malloc(sizeof(op_code) + sizeof(t_PCB));

	//Copio en el stream todos los valores del PCB serializados
	memcpy(stream, &codigoOperacion, sizeof(op_code));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 0 + sizeof(uint8_t) * 0, &(pcb.PC), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 1 + sizeof(uint8_t) * 0, &(pcb.PID), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 2 + sizeof(uint8_t) * 0, &(pcb.quantum), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 3 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.PC), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 4 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.EAX), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 5 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.EBX), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 6 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.ECX), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.EDX), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 8 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.SI), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 9 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.DI), sizeof(u_int32_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 0, &(pcb.registrosDeCPU.AX), sizeof(u_int8_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 1, &(pcb.registrosDeCPU.BX), sizeof(u_int8_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 2, &(pcb.registrosDeCPU.CX), sizeof(u_int8_t));
	memcpy(stream + sizeof(op_code) + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 3, &(pcb.registrosDeCPU.DX), sizeof(u_int8_t));

	//Retorno el stream creado
	return stream;
}

//Funcion que envia el pcb a kernel con un codigo de operacion que se indica en el parametro
void enviarPCBConCodOp(t_PCB pcb, op_code codigoOperacion, int conexionKernelDispatch)
{
    //Actualizo el pc que esta por fuera de los registros
    pcb.PC = pcb.registrosDeCPU.PC;

    //Creo un stream con el PCB serializado y su respectivo codigo de operacion
	void* stream = serializarPCBConCodOp(pcb, codigoOperacion);

	//Envio el stream ya cargado
	send(conexionKernelDispatch, stream, sizeof(op_code) + sizeof(t_PCB), 0); 

	//Libero el stream
	free(stream);
}

//--------------FUNCIONES AUXILIARES GENERICAS---------------

//Funcion para calcular el tamano de un operando
int calcularTamanoOPerando(char* operando)
{   
    //Me fijo si es un registro de 4 bytes
    if (strlen(operando) == 3 || !strcmp(operando, "SI") || !strcmp(operando, "DI")) 
    {   
            
        return sizeof(int32_t);
    }

    //Me fijo si es un registro de 1 byte
    else if (strlen(operando) == 2)
    {   
            
        return sizeof(int8_t);
    }

    //Si no entro en ninguno de los anteriores devuelve error avisando que tipo de error de produjo
    else{
        perror("Registro invalido");
        exit(EXIT_FAILURE);
    }    
}

//Funcion para recibir la respuesta de la instruccion resize
int recibirRespuestaRESIZE(int conexionMemoria, t_PCB* pcb, int* interrupciones)
{   
    int respuesta = recibirOperacion(conexionMemoria);
    int cantPaginas;
    recv(conexionMemoria, &cantPaginas, sizeof(int), MSG_WAITALL);

    if(respuesta == OUT_OF_MEMORY)
    {
        *interrupciones = -5;
    }

    return cantPaginas;
}

//Funcion para separar los operandos de una instruccion
t_operandos separarOperandos(char* operandos)
{   
    //Creo la estructura de los operandos separados
    t_operandos operandosSeparados;

    //Separo los operandos y los meto en la estructura
    operandosSeparados.operando1 = strtok(operandos," ");
    operandosSeparados.operando2 = strtok(NULL , " ");
    operandosSeparados.operando3 = strtok(NULL , " ");
    operandosSeparados.operando4 = strtok(NULL , " ");
    operandosSeparados.operando5 = strtok(NULL , " ");

    //Si en strtok lee un 0, por algun motivo devuelve null y da segmentation fault despues, asi que lo pongo en "0",(se repite para cada operando)
    if (operandosSeparados.operando1 == NULL) {
        operandosSeparados.operando1 = "0";
    }

    if (operandosSeparados.operando2 == NULL) {
        operandosSeparados.operando2 = "0";
    }

    if (operandosSeparados.operando3 == NULL) {
        operandosSeparados.operando3 = "0";
    }

    if (operandosSeparados.operando4 == NULL) {
        operandosSeparados.operando4 = "0";
    }

    if (operandosSeparados.operando5 == NULL) {
        operandosSeparados.operando5 = "0";
    }

    //Devuelvo la estructura cargada con los operandos
    return operandosSeparados;
}

//Diccionario que realiza el string de un registro con su posicion de memoria
void cargarDiccionarioRegistros(t_registrosDeCPU* registrosCpu, t_dictionary* diccionarioRegistros)
{
    //Cargo las posiciones de memoria del registroCpu relacionadas a un string de cada registro
    dictionary_put(diccionarioRegistros, "PC", &(registrosCpu->PC));
    dictionary_put(diccionarioRegistros, "AX", &(registrosCpu->AX));
    dictionary_put(diccionarioRegistros, "BX", &(registrosCpu->BX));
    dictionary_put(diccionarioRegistros, "CX", &(registrosCpu->CX));
    dictionary_put(diccionarioRegistros, "DX", &(registrosCpu->DX));
    dictionary_put(diccionarioRegistros, "SI", &(registrosCpu->SI));
    dictionary_put(diccionarioRegistros, "DI", &(registrosCpu->DI));
    dictionary_put(diccionarioRegistros, "EAX", &(registrosCpu->EAX));
    dictionary_put(diccionarioRegistros, "EBX", &(registrosCpu->EBX));
    dictionary_put(diccionarioRegistros, "ECX", &(registrosCpu->ECX));
    dictionary_put(diccionarioRegistros, "EDX", &(registrosCpu->EDX));
}

//Funcion para pasar el string de la instruccion a un enum
t_enumInstrucciones asignarCodigoInstrucciones(char* instruccion)
{   
    //Creo una instancia del enum y la inicializo en -1 para manejar errores
    t_enumInstrucciones codigoInstruccion  = -1;

    //Evaluo la coparacion de cada caso con el string respectivo y modifico el valro de codigoInstruccion
    if(strcmp(instruccion, "SET") == 0){
        codigoInstruccion = SET;
    }
    else if(strcmp(instruccion, "MOV_IN") == 0){
        codigoInstruccion = MOV_IN;
    }
    else if(strcmp(instruccion, "MOV_OUT") == 0){
        codigoInstruccion = MOV_OUT;
    }
    else if(strcmp(instruccion, "SUM") == 0){
        codigoInstruccion = SUM;
    }
    else if(strcmp(instruccion, "SUB") == 0){
        codigoInstruccion = SUB;
    }
    else if(strcmp(instruccion, "JNZ") == 0){
        codigoInstruccion = JNZ;
    }
    else if(strcmp(instruccion, "RESIZE") == 0){
        codigoInstruccion = RESIZE;
    }
    else if(strcmp(instruccion, "COPY_STRING") == 0){
        codigoInstruccion = COPY_STRING;
    }
    else if(strcmp(instruccion, "WAIT") == 0){
        codigoInstruccion = WAIT;
    }
    else if(strcmp(instruccion, "SIGNAL") == 0){
        codigoInstruccion = SIGNAL;
    }
    else if(strcmp(instruccion, "IO_GEN_SLEEP") == 0){
        codigoInstruccion = IO_GEN_SLEEP;
    }
    else if(strcmp(instruccion, "IO_STDIN_READ") == 0){
        codigoInstruccion = IO_STDIN_READ;
    }
    else if(strcmp(instruccion, "IO_STDOUT_WRITE") == 0){
        codigoInstruccion = IO_STDOUT_WRITE;
    }
    else if(strcmp(instruccion, "IO_FS_CREATE") == 0){
        codigoInstruccion = IO_FS_CREATE;
    }
    else if(strcmp(instruccion, "IO_FS_DELETE") == 0){
        codigoInstruccion = IO_FS_DELETE;
    }
    else if(strcmp(instruccion, "IO_FS_TRUNCATE") == 0){
        codigoInstruccion = IO_FS_TRUNCATE;
    }
    else if(strcmp(instruccion, "IO_FS_WRITE") == 0){
        codigoInstruccion = IO_FS_WRITE;
    }
    else if(strcmp(instruccion, "IO_FS_READ") == 0){
        codigoInstruccion = IO_FS_READ;
    }
    else if(strcmp(instruccion, "EXIT") == 0){
        codigoInstruccion = EXIT;
    }

    //Evaluo el caso en que la instruccion no esta dentro de las prevista
    else if(codigoInstruccion == -1){
        perror("Instruccion invalida");
		exit(EXIT_FAILURE);
    }    
    
    //Retorno el enum con el codigo de que le corresponde
    return codigoInstruccion;
}

//--------------REALIZAR INSTRUCCIONES---------------

//Funcion que realiza todas las posibles instrucciones
void realizarInstruccion(t_instruccionEntera* instruccionEntera, t_PCB* pcb, t_dictionary* diccionario, int conexionDispatch, void* tlb, int tamPagina, int pid, int numEntradasTlb, char* algoritmoTlb, int conexionMemoria, t_log* logger, int* interrupciones)
{   
    //Transformo el string de la instruccion en un enum para poder usarlo en el switch
    t_enumInstrucciones codigoInstruccion = asignarCodigoInstrucciones(instruccionEntera->instruccion);

    //Realizo la funcion dependiendo de que codigo de instruccion tengo
    switch (codigoInstruccion)
    {
        case SET:
            realizarInstruccionSET(instruccionEntera->operandos, diccionario);
            break;
        case MOV_IN:
            realizarInstruccionMOV_IN(instruccionEntera->operandos, diccionario, tlb, numEntradasTlb, tamPagina, algoritmoTlb, conexionMemoria, pid, logger);
            break;
        case MOV_OUT:
            realizarInstruccionMOV_OUT(instruccionEntera->operandos, diccionario, tlb, numEntradasTlb, tamPagina, algoritmoTlb, conexionMemoria, pid, logger);
            break;
        case SUM:
            realizarInstruccionSUM(instruccionEntera->operandos, diccionario);
            break;
        case SUB:
            realizarInstruccionSUB(instruccionEntera->operandos, diccionario);
            break;
        case JNZ:
            realizarInstruccionJNZ(instruccionEntera->operandos, diccionario);
            break;
        case RESIZE:
            realizarInstruccionRESIZE(instruccionEntera->operandos, diccionario, pcb, conexionMemoria, interrupciones, tlb, numEntradasTlb);
            break;
        case COPY_STRING:
            realizarInstruccionCOPY_STRING(instruccionEntera->operandos, diccionario, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
            break;
        case WAIT:
            realizarInstruccionWAIT(instruccionEntera->operandos, conexionDispatch, pcb, interrupciones);
            break;
        case SIGNAL:
            realizarInstruccionSIGNAL(instruccionEntera->operandos, conexionDispatch, pcb, interrupciones);
            break;
        case IO_GEN_SLEEP:
            realizarInstruccionIO_GEN_SLEEP(pcb, instruccionEntera->operandos, conexionDispatch, interrupciones);
            break;
        case IO_STDIN_READ:
            realizarInstruccionIO_STDIN_READoIO_STDOUT_WRITE(pcb, instruccionEntera->operandos, diccionario, tlb, tamPagina, numEntradasTlb, algoritmoTlb, conexionMemoria, conexionDispatch, logger, STDIN, interrupciones);
            break;
        case IO_STDOUT_WRITE:
            realizarInstruccionIO_STDIN_READoIO_STDOUT_WRITE(pcb, instruccionEntera->operandos, diccionario, tlb, tamPagina, numEntradasTlb, algoritmoTlb, conexionMemoria, conexionDispatch, logger, STDOUT, interrupciones);
            break;
        case IO_FS_CREATE:
            realizarInstruccionIO_FS_CREATEoIO_FS_DELETE(pcb, instruccionEntera->operandos, conexionDispatch, FS_CREATE, interrupciones);
            break;
        case IO_FS_DELETE:
            realizarInstruccionIO_FS_CREATEoIO_FS_DELETE(pcb, instruccionEntera->operandos, conexionDispatch, FS_DELETE, interrupciones);
            break;
        case IO_FS_TRUNCATE:
            realizarInstruccionIO_FS_TRUNCATE(pcb, instruccionEntera->operandos, conexionDispatch, diccionario, interrupciones);
            break;
        case IO_FS_WRITE:
            realizarInstruccionIO_FS_WRITEoIO_FS_READ(pcb, diccionario, instruccionEntera->operandos, conexionDispatch, FS_WRITE, interrupciones);
            break;
        case IO_FS_READ:
            realizarInstruccionIO_FS_WRITEoIO_FS_READ(pcb, diccionario, instruccionEntera->operandos, conexionDispatch, FS_READ, interrupciones);
            break;
        case EXIT:
            realizarInstruccionEXIT(pcb, interrupciones);
            break;
    }

    //Libero recursos
    free(instruccionEntera->instruccion);
    free(instruccionEntera);
    
}

//Funcion que realiza la instruccion SET
void realizarInstruccionSET(char* operandos, t_dictionary* diccionario)
{
    //Creo una instancia de t_operandos para separar el string de operandos en diferentes strings
    t_operandos operandosSeparados = separarOperandos(operandos);   

    //Se transforma el numero en formato string a formato int
    int valorFinal = atoi(operandosSeparados.operando2);

    //Calculo el tamano del operando
    int tamanoOPerando = calcularTamanoOPerando(operandosSeparados.operando1);
    
    //Le asigno un el puntero correspondiente dependiendo del tamano del registro
	if (tamanoOPerando == sizeof(uint32_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroAfectado (el cual apunta a la posicion de memoria del registro)
		uint32_t *registroAfectado = dictionary_get(diccionario, operandosSeparados.operando1);
        
        //Seteo el registro afectado en el valor que dice la instruccion
        *registroAfectado = valorFinal;     
	}

	else if (tamanoOPerando == sizeof(uint8_t))
	{   
        //Le asigno el tipo de dato uint_8 al registroAfectado (el cual apunta a la posicion de memoria del registro)
		uint8_t *registroAfectado = dictionary_get(diccionario, operandosSeparados.operando1);

        //Seteo el registro afectado en el valor que dice la instruccion
        *registroAfectado = valorFinal;
	}
	
}

//Funcion que realiza la instruccion MOV_IN
void realizarInstruccionMOV_IN(char* operandos, t_dictionary* diccionario, void* tlb, int numEntradasTlb, int tamPagina, char* algoritmoTlb, int conexionMemoria, int pid, t_log* logger)
{   
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Creo variable para guardar la logntiud del registro
    uint8_t tamanoRegistro;

    //Calculo el tamano del operando
    int tamanoOPerando1 = calcularTamanoOPerando(operandosSeparados.operando1);

    tamanoRegistro = tamanoOPerando1;

    //Calculo el tamano del operando
    int tamanoOPerando2 = calcularTamanoOPerando(operandosSeparados.operando2);

    //Creo una variable para la direccion fisica
    int direcFisica;

    if(tamanoOPerando2 == 1)
    {
        //Obtengo el regsitro direccion
    uint8_t* ptrRegistroDireccion = dictionary_get(diccionario, operandosSeparados.operando2);

    //Lo guardo en una variable para manejarlo mas facil
    uint8_t registroDireccion = *ptrRegistroDireccion;

    //Traduzco direccion logica a fisica
    direcFisica = traducirDirecLogica(registroDireccion, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    else if(tamanoOPerando2 == 4)
    {   
        //Obtengo el regsitro direccion
    uint32_t* ptrRegistroDireccion = dictionary_get(diccionario, operandosSeparados.operando2);

    //Lo guardo en una variable para manejarlo mas facil
    uint32_t registroDireccion = *ptrRegistroDireccion; 

    //Traduzco direccion logica a fisica
    direcFisica = traducirDirecLogica(registroDireccion, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    
    //Envio la instruccion a memoria y recibo el dato
    int dato = enviarMOV_IN(tamanoRegistro, direcFisica, conexionMemoria, pid);

    //Log obligatorio
    log_info(logger, "PID: %d - Acción: LECTURA - Dirección Física: %d - Valor: %d", pid, direcFisica, dato);

    if(tamanoRegistro == 1)
    {
        uint8_t* ptrRegistroDatos = dictionary_get(diccionario, operandosSeparados.operando1);
        *ptrRegistroDatos = dato;
    }

    else if(tamanoRegistro == 4)
    {   
        uint32_t* ptrRegistroDatos = dictionary_get(diccionario, operandosSeparados.operando1);
        *ptrRegistroDatos = dato;
    }
}

//Funcion que realiza la instruccion MOV_OUT
void realizarInstruccionMOV_OUT(char* operandos, t_dictionary* diccionario, void* tlb, int numEntradasTlb, int tamPagina, char* algoritmoTlb, int conexionMemoria, int pid, t_log* logger)
{
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Creo variable para guardar la logntiud del registro
    uint8_t tamanoRegistro;

    //Calculo el tamano del operando
    int tamanoOPerando1 = calcularTamanoOPerando(operandosSeparados.operando1);

    //Calculo el tamano del operando
    int tamanoOPerando2 = calcularTamanoOPerando(operandosSeparados.operando2);

    tamanoRegistro = tamanoOPerando2;

    //creo una variable para la direcccion fisica
    int direcFisica;

    if(tamanoOPerando1 == 1)
    {
        //Obtengo el regsitro direccion
        uint8_t* ptrRegistroDireccion = dictionary_get(diccionario, operandosSeparados.operando1);

        //Lo guardo en una variable para manejarlo mas facil
        uint8_t registroDireccion = *ptrRegistroDireccion;

        //Traduzco direccion logica a fisica
        direcFisica = traducirDirecLogica(registroDireccion, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }
    
    else if(tamanoOPerando1 == 4)
    {   
        //Obtengo el regsitro direccion
        uint32_t* ptrRegistroDireccion = dictionary_get(diccionario, operandosSeparados.operando1);

        //Lo guardo en una variable para manejarlo mas facil
        uint32_t registroDireccion = *ptrRegistroDireccion;

        //Traduzco direccion logica a fisica
        direcFisica = traducirDirecLogica(registroDireccion, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    if(tamanoRegistro == 1)
    {
        uint8_t* ptrRegistroDatos = dictionary_get(diccionario, operandosSeparados.operando2);
        int dato = *ptrRegistroDatos;

        //Log obligatorio
        log_info(logger, "PID: %d - Acción: ESCRITURA - Dirección Física: %d - Valor: %d", pid, direcFisica, dato);

        enviarMOV_OUT(tamanoRegistro, direcFisica, dato, conexionMemoria, pid);
    }

    else if(tamanoRegistro == 4)
    {   
        uint32_t* ptrRegistroDatos = dictionary_get(diccionario, operandosSeparados.operando2);
        int dato = *ptrRegistroDatos;

        //Log obligatorio
        log_info(logger, "PID: %d - Acción: ESCRITURA - Dirección Física: %d - Valor: %d", pid, direcFisica, dato);

        enviarMOV_OUT(tamanoRegistro, direcFisica, dato, conexionMemoria, pid);
    }
}

//Funcion que realiza la instruccion SUM
void realizarInstruccionSUM(char* operandos, t_dictionary* diccionario)
{
    //Separo los operandos en diferentes strings
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del primer operando
    int tamanoOPerando1 = calcularTamanoOPerando(operandosSeparados.operando1);

    //Calculo el tamano del segundo operando
    int tamanoOPerando2 = calcularTamanoOPerando(operandosSeparados.operando2);
    
    //Comparo el registroDestino para descubir si es un uint_32 
    if (tamanoOPerando1 == sizeof(uint32_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro a modificar)
		uint32_t *registroDestino = dictionary_get(diccionario, operandosSeparados.operando1);
        
        //Comparo el registroDestino para descubir si es un uint_32 
        if (tamanoOPerando2 == sizeof(uint32_t)) 
	    {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint32_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);            
        
            //Sumo al registroDestino el regsitroOrigen
            *registroDestino += *registroOrigen;
	    }

        //Comparo el registro para descubir si es un uint_8
        else if (tamanoOPerando2 == sizeof(uint8_t))
        {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint8_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);

            //Sumo al registroDestino el regsitroOrigen
            *registroDestino += *registroOrigen;
        }
	}

    //Comparo el registro para descubir si es un uint_8
	else if (tamanoOPerando1 == sizeof(uint8_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro a modificar)
		uint32_t *registroDestino = dictionary_get(diccionario, operandosSeparados.operando1);
        
        //Comparo el registroDestino para descubir si es un uint_32 
        if (tamanoOPerando2 == sizeof(uint32_t)) 
	    {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint32_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);            
        
            //Sumo al registroDestino el regsitroOrigen
            *registroDestino += *registroOrigen;
	    }

        //Comparo el registro para descubir si es un uint_8
        else if (tamanoOPerando2 == sizeof(uint8_t))
        {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint8_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);

            //Sumo al registroDestino el regsitroOrigen
            *registroDestino += *registroOrigen;
        }
	}
}

//Funcion que realiza la instruccion SUB
void realizarInstruccionSUB(char* operandos, t_dictionary* diccionario)
{
    //Separo los operandos en diferentes strings
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del primer operando
    int tamanoOPerando1 = calcularTamanoOPerando(operandosSeparados.operando1);

    //Calculo el tamano del segundo operando
    int tamanoOPerando2 = calcularTamanoOPerando(operandosSeparados.operando1);
    
    //Comparo el registroDestino para descubir si es un uint_32 
    if (tamanoOPerando1 == sizeof(uint32_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro a modificar)
		uint32_t *registroDestino = dictionary_get(diccionario, operandosSeparados.operando1);
        
        //Comparo el registroDestino para descubir si es un uint_32 
        if (tamanoOPerando2 == sizeof(uint32_t)) 
	    {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint32_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);            
        
            //Resto al registroDestino el regsitroOrigen
            *registroDestino -= *registroOrigen;
	    }

        //Comparo el registro para descubir si es un uint_8
        else if (tamanoOPerando2 == sizeof(uint8_t))
        {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint8_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);

            //Resto al registroDestino el regsitroOrigen
            *registroDestino -= *registroOrigen;
        }
	}

    //Comparo el registro para descubir si es un uint_8
	else if (tamanoOPerando1 == sizeof(uint8_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro a modificar)
		uint32_t *registroDestino = dictionary_get(diccionario, operandosSeparados.operando1);
        
        //Comparo el registroDestino para descubir si es un uint_32 
        if (tamanoOPerando2 == sizeof(uint32_t)) 
	    {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint32_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);            
        
            //Resto al registroDestino el regsitroOrigen
            *registroDestino -= *registroOrigen;
	    }

        //Comparo el registro para descubir si es un uint_8
        else if (tamanoOPerando2 == sizeof(uint8_t))
        {   
            //Le asigno el tipo de dato uint_32 al registroDestino (el cual apunta a la posicion de memoria del registro que se debe obtener el valor)
            uint8_t *registroOrigen = dictionary_get(diccionario, operandosSeparados.operando2);

            //Resto al registroDestino el regsitroOrigen
            *registroDestino -= *registroOrigen;
        }
	}
}

//Funcion que realiza la instruccion RESIZE
void realizarInstruccionRESIZE(char* operandos, t_dictionary* diccionario, t_PCB* pcb, int conexionMemoria, int* interrupciones, void* tlb, int cantEntradas)
{   
    //Cambio el operando de formato string a int
    int tamanioProceso = atoi(operandos);
  
    //Envio la instruccion RESIZE a memoria
    enviarRESIZE(pcb->PID, tamanioProceso, conexionMemoria);

    //Recibo la respuesta de memoria
    int cantPaginas = recibirRespuestaRESIZE(conexionMemoria, pcb, interrupciones);

    actualizarTLB(tlb, cantEntradas, cantPaginas, pcb->PID);
}

void actualizarTLB(void* tlb, int cantEntradas, int cantPaginas, int PID)
{
    int tamanioFila = 4;
    int* ptrTLB = tlb;

    for(int i = 0; i < cantEntradas; i++)
    {
        if(PID == *ptrTLB && cantPaginas <= *(ptrTLB + 1))
        {
            *(ptrTLB) = -1;
            *(ptrTLB + 1) = -1; 
            *(ptrTLB + 2) = -1; 
            *(ptrTLB + 3) = -1;  
        }
        ptrTLB += tamanioFila;
    }
}

//Funcion que realiza la instruccion COPY_STRING
void realizarInstruccionCOPY_STRING(char* operandos, t_dictionary* diccionario, int tamPagina, void* tlb, int pid, int numEntradasTlb, char* algoritmoTlb, int conexionMemoria, t_log* logger)
{       
    //Paso el operando de char a entero
    int tamanoString = atoi(operandos);
    uint8_t tamanoDato = tamanoString;

    //Creo punteros a SI y DI
    uint32_t* ptrSI = dictionary_get(diccionario, "SI");
    uint32_t* ptrDI = dictionary_get(diccionario, "DI");

    //traduzco las direcciones
    uint32_t direccionFisicaSI = traducirDirecLogica(*ptrSI, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    uint32_t direccionFisicaDI = traducirDirecLogica(*ptrDI, tamPagina, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);

    //Creo una variable para el tamano de la direccion fisica
    enviarCOPY_STRING(tamanoString, direccionFisicaSI, direccionFisicaDI, pid, conexionMemoria, tamanoDato, logger);
}

//Funcion que realiza la instruccion WAIT
void realizarInstruccionWAIT(char* operandos, int conexionDispatch, t_PCB* pcb, int* interrupciones)
{   
    //Envio PCB con su respectivo codigo de operacion
    enviarPCBConCodOp(*(pcb), RECURSOS, conexionDispatch);

    //Envio isntruccion WAIT a kernel
    enviarWAITOSIGNAL(operandos, conexionDispatch, INTERRUPT_WAIT);

    //Aviso que hubo una peticion de recursos
    if(*interrupciones != -3)
    {
        *interrupciones = -6;
    }
}

//Funcion que realiza la instruccion SIGNAL
void realizarInstruccionSIGNAL(char* operandos, int conexionDispatch, t_PCB* pcb, int* interrupciones)
{   
    //Envio PCB con su respectivo codigo de operacion
    enviarPCBConCodOp(*(pcb), RECURSOS, conexionDispatch);

    //Envio instruccion SIGNAL a kernel
    enviarWAITOSIGNAL(operandos, conexionDispatch, INTERRUPT_SIGNAL);

    //Aviso que hubo una peticion de recursos
    if(*interrupciones != -3)
    {
        *interrupciones = -6;
    }
}

//Funcion que realiza la instruccion JNZ
void realizarInstruccionJNZ(char* operandos, t_dictionary* diccionario)
{
    //Separo los operandos en diferentes strings
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del primer operando
    int tamanoOPerando = calcularTamanoOPerando(operandosSeparados.operando1);

    if (tamanoOPerando == sizeof(uint32_t)) 
	{   
        //Le asigno el tipo de dato uint_32 al registroAfectado (el cual apunta a la posicion de memoria del registro)
		uint32_t *registro = dictionary_get(diccionario, operandosSeparados.operando1);

        //Pregunto si el registro es diferente de 0 para saber si debo cambiar el PC
        if(*registro != 0)
        {
            //Se transforma el numero en formato string a formato int
            int valorNuevoPC = atoi(operandosSeparados.operando2);

            //Obtengo la posicion de memoria de PC
            uint32_t* punteroPC = dictionary_get(diccionario, "PC");

            //Le asigo el nuevo valor a PC(el que recibi en la instruccion)
            *punteroPC = valorNuevoPC;
        }
	}

	else if (tamanoOPerando == sizeof(uint8_t))
	{   
        //Le asigno el tipo de dato uint_8 al registroAfectado (el cual apunta a la posicion de memoria del registro)
		uint8_t *registro = dictionary_get(diccionario, operandosSeparados.operando1);

        //Pregunto si el registro es diferente de 0 para saber si debo cambiar el PC
        if(*registro != 0)
        {
            //Se transforma el numero en formato string a formato int
            int valorNuevoPC = atoi(operandosSeparados.operando2);

            //Obtengo la posicion de memoria de PC
            uint32_t* punteroPC = dictionary_get(diccionario, "PC");

            //Le asigo el nuevo valor a PC(el que recibi en la instruccion)
            *punteroPC = valorNuevoPC;
        }
	}    
}

//Funcion que realiza la instruccion IO_GEN_SLEEP
void realizarInstruccionIO_GEN_SLEEP(t_PCB* pcb, char* operandos, int conexionDispatch, int* interrupciones)
{
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Creo un enum del tipo interfaz para luergo compararlo con el string recibido y asignarle un valor dependiendo de su valor
    tipo_interfaz tipoDeInterfaz = GENERICA;

    //Envio el PCB con el motivo de interrupcion
    enviarPCBConCodOp(*(pcb), SOLICITUD_INTERFAZ, conexionDispatch);

    //Le digo a kernel mediante conexionDispatch para que mande a dormir a la interfaz durante la cantidad de unidades de trabajo
    enviarIO_GEN_SLEEP(operandosSeparados, tipoDeInterfaz, conexionDispatch);

    //Aviso que hubo una operacion IO
    if(*interrupciones != -3)
    {
        *interrupciones = -4;
    }
}

//Funcion que realiza la instruccion IO_STDIN_READ
void realizarInstruccionIO_STDIN_READoIO_STDOUT_WRITE(t_PCB* pcb, char* operandos, t_dictionary* diccionario, void* tlb, int tamPagina, int numEntradasTlb, char* algoritmoTlb, int conexionMemoria, int conexionDispatch, t_log* logger, tipo_interfaz tipoInterfaz, int* interrupciones)
{
    //Separo los operandos en diferentes strings
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del segundo operando
    int tamanoOperando2 = calcularTamanoOPerando(operandosSeparados.operando2);

    //Calculo el tamano del tercer operando
    int tamanoOperando3 = calcularTamanoOPerando(operandosSeparados.operando3);

    uint32_t direcFisica;

    if(tamanoOperando2 == 1)
    {
        //Creo un puntero apuntando al registro que contiene la direccion logica
        uint8_t* ptrDireccionLogica = dictionary_get(diccionario, operandosSeparados.operando2);

        //Calculo la direccion fisica
        direcFisica = traducirDirecLogica(*ptrDireccionLogica, tamPagina, tlb, pcb->PID, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    else if(tamanoOperando2 == 4)
    {
        //Creo un puntero apuntando al registro que contiene la direccion logica
        uint32_t* ptrDireccionLogica = dictionary_get(diccionario, operandosSeparados.operando2);

        //Calculo la direccion fisica
        direcFisica = traducirDirecLogica(*ptrDireccionLogica, tamPagina, tlb, pcb->PID, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    //Creo la variable del valor que esta en el operando 3
    uint32_t tamanoRegistro;

    if(tamanoOperando3 == 1)
    {
        //Creo un puntero apuntando al registro que contiene el tamano del registro
        uint8_t* ptrTamanoRegistro = dictionary_get(diccionario, operandosSeparados.operando3);

        //Creo una variable y le guardo el valor de puntero del tamano del registro
        tamanoRegistro = *ptrTamanoRegistro;
    }

    else if(tamanoOperando3 == 4)
    {
        //Creo un puntero apuntando al registro que contiene el tamano del registro
        uint32_t* ptrTamanoRegistro = dictionary_get(diccionario, operandosSeparados.operando3);

        //Creo una variable y le guardo el valor de puntero del tamano del registro
        tamanoRegistro = *ptrTamanoRegistro;
    }

    //Envio pcb a kernel avisandole que le envio la instruccion
    enviarPCBConCodOp(*(pcb), SOLICITUD_INTERFAZ, conexionDispatch);

    //Envio a kernel los datos de la instruccion con su codigo de operacion mediante la conexion dispatch
    enviarSTDINOSTDOUT(tipoInterfaz, operandosSeparados.operando1, direcFisica , tamanoRegistro, conexionDispatch);

    if(*interrupciones != -3)
    {
        *interrupciones = -4;
    }
}

//Funcion que realiza la instruccion IO_FS_CREATE o IO_FS_DELETE
void realizarInstruccionIO_FS_CREATEoIO_FS_DELETE(t_PCB* pcb, char* operandos, int conexionDispatch, tipoInstruccionInterfaz instruccionInterfaz, int* interrupciones)
{
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Envio el PCB con el motivo de interrupcion
    enviarPCBConCodOp(*(pcb), SOLICITUD_INTERFAZ, conexionDispatch);

    //Le digo a kernel mediante conexionDispatch para que realice la instrccion IO_FS_CREATE o IO_FS_DELETE segun corresponda
    enviarIO_FS_CREATEoIO_FS_DELETE(operandosSeparados, conexionDispatch, instruccionInterfaz);

    //Aviso que hubo una operacion 
    if(*interrupciones != -3)
    {
        *interrupciones = -4;
    }
}

//Funcion que realiza la instruccion IO_FS_TRUNCATE
void realizarInstruccionIO_FS_TRUNCATE(t_PCB* pcb, char* operandos, int conexionDispatch, t_dictionary* diccionario, int* interrupciones)
{   
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del ultimo operandos
    int tamanoOperando = calcularTamanoOPerando(operandosSeparados.operando3);

    //Creo una variable para almacenar el valor del registro
    uint32_t tamanoTruncate;

    if(tamanoOperando == 1){
        //Creo un puntero apuntando al registro que contiene el tamano del registro
        uint8_t* ptrTamanoTruncate = dictionary_get(diccionario, operandosSeparados.operando3);

        //Creo una variable y le guardo el valor de puntero del tamano del registro
        tamanoTruncate = *ptrTamanoTruncate;
    }
    
    else if(tamanoOperando == 4)
    {
        //Creo un puntero apuntando al registro que contiene el tamano del registro
        uint32_t* ptrTamanoTruncate = dictionary_get(diccionario, operandosSeparados.operando3);

        //Creo una variable y le guardo el valor de puntero del tamano del registro
        tamanoTruncate = *ptrTamanoTruncate;
    }

    //Envio el PCB con el motivo de interrupcion
    enviarPCBConCodOp(*(pcb), SOLICITUD_INTERFAZ, conexionDispatch);

    //Le digo a kernel mediante conexionDispatch para que realice la instrccion IO_FS_TRUNCATE
    enviarIO_FS_TRUNCATE(operandosSeparados, conexionDispatch, tamanoTruncate);

    //Aviso que hubo una operacion 
    if(*interrupciones != -3)
    {
        *interrupciones = -4;
    }
}

//Funcion que realiza la instruccion IO_FS_WRITE o IO_FS_READ segun corresponda
void realizarInstruccionIO_FS_WRITEoIO_FS_READ(t_PCB* pcb, t_dictionary* diccionario, char* operandos, int conexionDispatch, tipoInstruccionInterfaz instruccionInterfaz, int* interrupciones)
{
    //Separo los operandos
    t_operandos operandosSeparados = separarOperandos(operandos);

    //Calculo el tamano del tercer operando
    int tamanoOperando3 = calcularTamanoOPerando(operandosSeparados.operando3);

    //Calculo el tamano del cuarto operando
    int tamanoOperando4 = calcularTamanoOPerando(operandosSeparados.operando4);

    //Calculo el tamano del ultimo operando
    int tamanoOperando5 = calcularTamanoOPerando(operandosSeparados.operando5);

    //Creo las variables para almacenar los valores de los registros
    uint32_t direccionLogica;
    uint32_t tamanoALeer;
    uint32_t punteroArchivo;

    //Creo un puntero del tamano del registro segun corresponda para el tercer operando
    if(tamanoOperando3 == 1)
    {
        //Creo un puntero apuntando al registro que contiene la direccion logica
        uint8_t* ptrDireccionLogica = dictionary_get(diccionario, operandosSeparados.operando3);

        //Guardo el valor en la varaible creada anteriormente
        direccionLogica = *ptrDireccionLogica;
    }

    else if(tamanoOperando3 == 4)
    {
        //Creo un puntero apuntando al registro que contiene la direccion logica
        uint32_t* ptrDireccionLogica = dictionary_get(diccionario, operandosSeparados.operando3);

        //Guardo el valor en la varaible creada anteriormente
        direccionLogica = *ptrDireccionLogica;
    }

    //Creo un puntero del tamano del registro segun corresponda para el cuarto operando
    if(tamanoOperando4 == 1)
    {
        //Creo un puntero apuntando al registro que contiene el tamano a leer
        uint8_t* ptrTamanoALeer = dictionary_get(diccionario, operandosSeparados.operando4);

        //Guardo el valor en la varaible creada anteriormente
        tamanoALeer = *ptrTamanoALeer;
    }

    else if(tamanoOperando4 == 4)
    {
        //Creo un puntero apuntando al registro que contiene el tamano a leer
        uint32_t* ptrTamanoALeer = dictionary_get(diccionario, operandosSeparados.operando4);

        //Guardo el valor en la varaible creada anteriormente
        tamanoALeer = *ptrTamanoALeer;
    }

    //Creo un puntero del tamano del registro segun corresponda para el quinto operando
    if(tamanoOperando5 == 1)
    {
        //Creo un puntero apuntando al registro que contiene el Puntero del archivo     
        uint8_t* ptrPunteroArchivo = dictionary_get(diccionario, operandosSeparados.operando5);

        //Guardo el valor en la varaible creada anteriormente
        punteroArchivo = *ptrPunteroArchivo;
    }

    else if(tamanoOperando5 == 4)
    {
        //Creo un puntero apuntando al registro que contiene el tamano a leer
        uint32_t* ptrPunteroArchivo = dictionary_get(diccionario, operandosSeparados.operando5);

        //Guardo el valor en la varaible creada anteriormente
        punteroArchivo = *ptrPunteroArchivo;
    }

    //Envio el PCB con el motivo de interrupcion
    enviarPCBConCodOp(*(pcb), SOLICITUD_INTERFAZ, conexionDispatch);

    //Le digo a kernel mediante conexionDispatch para que realice la instrccion IO_FS_TRUNCATE
    enviarIO_FS_WRITEoIO_FS_READ(operandosSeparados, conexionDispatch, direccionLogica, tamanoALeer, punteroArchivo, instruccionInterfaz);

    //Aviso que hubo una operacion 
    if(*interrupciones != -3)
    {
        *interrupciones = -4;
    }
}

//Funcion que realiza la instruccion EXIT
void realizarInstruccionEXIT(t_PCB* pcb, int* interrupciones)
{
    *interrupciones = -1;
}

//--------------CICLO DE INSTRUCCION---------------

//Funcion que realiza la etapa fetch del ciclo de instruccion
char* etapaFetch(t_PCB* pcb, int conexionMemoria, t_log* logger)
{   
    //Envia el PC y el PID a memoria
    enviarPCyPID(*pcb, conexionMemoria, logger);

    //Recibe la instruccion que nos envia memoria
    char* instruccionCompleta = recibirStreamInstruccion(conexionMemoria, logger);
    
    //Incrementa el PC 
    pcb->registrosDeCPU.PC ++;
    
    //Retorno el stream de instruccion
    return instruccionCompleta;
}

//Funcion que realiza la etapa decode del ciclo de instruccion
t_instruccionEntera* etapaDecode(char* instruccionCompleta)
{   
    //Creo y cargo un puntero a una estrcutura que tiene la instruccion y los operandos separados
    t_instruccionEntera* instruccionEntera = separarInstruccion(instruccionCompleta);

    //Devuelvo la estructura caraga
    return instruccionEntera;
}

//Funcion que realiza la etapa execute del ciclo de instruccion
void etapaExecute(t_instruccionEntera* instruccionEntera, t_PCB* pcb, t_dictionary* diccionarioRegistro, int conexionDispatch, void* tlb, int tamPagina, int pid, int numEntradasTlb, char* AlgoritmoTlb, int conexionMemoria, t_log* logger, int* interrupciones)
{    
    //Realizo la instruccion
    realizarInstruccion(instruccionEntera, pcb, diccionarioRegistro, conexionDispatch, tlb, tamPagina, pid, numEntradasTlb, AlgoritmoTlb, conexionMemoria, logger, interrupciones);
}

//Funcion que realiza el ciclo de instruccion de la cpu
void realizarCicloInstruccion(t_PCB* pcb, t_dictionary* diccionarioRegistros, int conexionMemoria, int conexionDispatch, t_log* logger, void* tlb, int tamPagina, int pid, int numEntradasTlb, char* AlgoritmoTlb, int* interrupciones)
{   
    //Le pide la instruccion a memoria y la recibe en instruccionCompleta
    char* instruccionCompleta = etapaFetch(pcb, conexionMemoria, logger);

    //Inicializa el struct para separar la instrccion de los operandos
    t_instruccionEntera* instruccionEntera = etapaDecode(instruccionCompleta);

    //Log obligatorio
    log_info(logger, "PID: %i - Ejecutando: %s - %s", pcb->PID, instruccionEntera->instruccion, instruccionEntera->operandos);

    //Realiza la instruccion y actualiza los registros
    etapaExecute(instruccionEntera, pcb, diccionarioRegistros, conexionDispatch, tlb, tamPagina, pid, numEntradasTlb, AlgoritmoTlb, conexionMemoria, logger, interrupciones);
}

//FUncion que maneja las interrupcion y devuelve el pcb
void manejarInterrupciones(t_PCB pcb, int conexionKernelDispatch, int* interrupciones, t_log* logger)
{   
    //COntemplo los casos en lo que debo desalojar el PCB
    switch (*interrupciones)
    {
    case -1:
        //Caso en el que el proceso termino por la instruccion EXIT
        log_info(logger, "EL PCB SE DEVOLVIO POR INSTRUCCION EXIT");
        enviarPCBConCodOp(pcb, PROCESO_FINALIZADO, conexionKernelDispatch); 
        break;
    case -2:
        //Caso en el que el proceso termino por orden del kernell debido a fin del quantum
        log_info(logger, "EL PCB SE DEVOLVIO POR FIN DE QUANTUM");
        enviarPCBConCodOp(pcb, FIN_DE_QUANTUM, conexionKernelDispatch);
        break;
    case -3:
    //Caso en el que el proceso termino por orden del kernell debido a que finalizo el proceso
        log_info(logger, "EL PCB SE DEVOLVIO POR ORDEN DE KERNELL QUE FINALIZO EL PROCESO");
        enviarPCBConCodOp(pcb, PROCESO_FINALIZADO_MANUAL, conexionKernelDispatch);
        break;   
    case -4:
        //Caso en el que el proceso se bloquea por una IO (ya se envio antes el PCB)
        log_info(logger, "EL PCB SE DEVOLVIO POR INSTRUCCION DE IO");
        break;
    case -5:
        //Caso en el que el proceso se devuelve debido a que la instruccon RESIZE devolvio OUT OF MEMORY
        log_info(logger, "EL PCB SE DEVOLVIO POR OUT OF MEMORY");
        enviarPCBConCodOp(pcb, OUT_OF_MEMORY, conexionKernelDispatch);
        break;
    case -6:
        //Caso en el que el proceso se devuelve debido a que se realizo la instruccion SIGNAL o WAIT(ya se envio antes el PCB)
        log_info(logger, "EL PCB SE DEVOLVIO POR PETICION DE RECURSOS");
        break;
    //Devuelve error si sucedio algo inesperado
    default:
        perror("Error en la etapa de check interrupt");
        exit(EXIT_FAILURE);
        break;
    }
    *interrupciones = 0;
}