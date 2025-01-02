//------------BIBLIOTECAS------------

#include <../include/consola.h>
#include <../include/planificador.h>

//------------DEFINICION DE FUNCIONES------------

//Funcion encargada de ejecutar la consola
void consola(argsPlanificacion* args)
{
    //Variables a utilizar por la consola interactiva
    char* consola;
    int contadorProcesos = 0;

    //Bucle infinito ejecutando la consola
    while(1)
    {
        //Aviso que la consola ya esta disponible
        log_info(args->logger, "CONSOLA DISPONIBLE");
            
        //Espero a que se ingrese una instruccion
        consola = readline(">>");

        ejecutarInstrucciones(args, consola, &contadorProcesos);

        free(consola);
    }
}

void ejecutarInstrucciones(argsPlanificacion* args, char* consola, int* contadorProcesos)
{
    char* comando = strtok(consola, " ");
    char* operando = strtok(NULL, "\n");

    //Interpreto la instruccion ingresada
    instrucciones instruccion = procesarInstrucciones(comando);

    //Dependendiendo la instruccion me fijo que hago
    switch(instruccion)
    {
        case EJECUTAR_SCRIPT:
            if(operando != NULL)
            {
                leerScript(args, comando, operando, contadorProcesos);
            }
            break;

        case INICIAR_PROCESO:
            if(operando != NULL)
            {
                iniciarProceso(args, operando, contadorProcesos);
            }
            break;

        case FINALIZAR_PROCESO:
            if(operando != NULL)
            {
                finalizarProcesoEspecifico(args, operando);
            }
            break;
        
        case DETENER_PLANIFICACION:
            log_info(args->logger, "DETENER_PLANIFICACION");
            sem_trywait(&semPlanificador);
            break;

        case INICIAR_PLANIFICACION:
            log_info(args->logger, "INICIAR_PLANIFICACION");
            sem_post(&semPlanificador);
            break;

        case MULTIPROGRAMACION:
            log_info(args->logger, "MULTIPROGRAMACION");
            if(operando != NULL)
            {
                int variacion = atoi(operando);
                if(variacion >= 0)
                {
                    modificarMultiprogramacion(args, variacion);
                }
            }
            break;

        case PROCESO_ESTADO:
            log_info(args->logger, "PROCESO_ESTADO");
            procesoEstado(args);
            break;

        case ERROR:
            break;
    }
}

//Funcion para procesar las instrucciones
instrucciones procesarInstrucciones(char* comando)
{
    //Dependiendo del ingreso por teclado retorno que tipo de insturccion se trata
    if(strcmp(comando, "EJECUTAR_SCRIPT") == 0)
    {
        return EJECUTAR_SCRIPT;
    }
    else if(strcmp(comando, "INICIAR_PROCESO") == 0)
    {
        return INICIAR_PROCESO;
    }
    else if(strcmp(comando, "FINALIZAR_PROCESO") == 0)
    {
        return FINALIZAR_PROCESO;
    }
    else if(strcmp(comando, "DETENER_PLANIFICACION") == 0)
    {
        return DETENER_PLANIFICACION;
    }
    else if(strcmp(comando, "INICIAR_PLANIFICACION") == 0)
    {
        return INICIAR_PLANIFICACION;
    }
    else if(strcmp(comando, "MULTIPROGRAMACION") == 0)
    {
        return MULTIPROGRAMACION;
    }
    else if(strcmp(comando, "PROCESO_ESTADO") == 0)
    {
        return PROCESO_ESTADO;
    }

    return ERROR;
}

t_PCB* inicializarPCB(int* contadorProcesos, t_valores valores)
{
    t_PCB* PCB = malloc(sizeof(t_PCB));

    PCB->PID = *contadorProcesos;
    PCB->PC = 0;
    PCB->quantum = valores.quantum;
    PCB->registrosDeCPU.PC = 0;
    PCB->registrosDeCPU.EAX = 0;
    PCB->registrosDeCPU.EBX = 0;
    PCB->registrosDeCPU.ECX = 0;
    PCB->registrosDeCPU.EDX = 0;
    PCB->registrosDeCPU.SI = 0;
    PCB->registrosDeCPU.DI = 0;
    PCB->registrosDeCPU.AX = 0;
    PCB->registrosDeCPU.BX = 0;
    PCB->registrosDeCPU.CX = 0;
    PCB->registrosDeCPU.DX = 0;

    return PCB;
}

void leerScript(argsPlanificacion* args, char* comando, char* path, int* contadorProcesos)
{
    char* pathFinal = malloc(strlen(path) + strlen(args->valores.pathCarpeta) + 1);
    
	strcpy(pathFinal, args->valores.pathCarpeta);
	strcat(pathFinal, path);

    //Abro el archivo solo para lectura y me fijo si el archivo existe
    FILE* script = fopen(pathFinal, "r");

    if (script != NULL) 
    {
        log_info(args->logger,"ARCHIVO ENCONTRADO");
        t_list* listaInstrucciones = list_create();

        //Voy hacia el final para obtener la ultima posicion y hago un malloc de todo el archivo
        fseek(script, 0, SEEK_END); 
        int longitud = ftell(script);
        fseek(script, 0, SEEK_SET);
        char* textoArchivo = malloc(longitud + 1);

        //Uso fread para obtener todo el texto del archivo, le agrego un \0 al final, sino se bugea
        fread(textoArchivo, sizeof(char), longitud, script);
        textoArchivo[longitud] = '\0';

        //Obtengo hasta el primer \n
        char* instruccion = strtok(textoArchivo, "\n");

        //Agrego a la lista todas las instrucciones (cuando strtok da NULL)
        while(instruccion != NULL)
        {
            list_add(listaInstrucciones, instruccion);
            instruccion = strtok(NULL, "\n");
        }

        //Obtengo la cantidad de instrucciones (esto no se puede hacer con strtok o da error, tuve que buscar otra forma)
        int cantInstrucciones = list_size(listaInstrucciones);

        //Ejecuto todas las instrucciones en el for
        for(int i = 0; i < cantInstrucciones; i++) 
        {
            ejecutarInstrucciones(args, list_get(listaInstrucciones, i), contadorProcesos);
        }

        //Libero los recursos
        list_destroy(listaInstrucciones);
        free(instruccion);
        free(pathFinal);
        fclose(script);
        free(textoArchivo);
    }

    else
    {
        //Si no encuentro el archivo no hago nada
        log_info(args->logger, "ARCHIVO NO ENCONTRADO");
    }
}

void iniciarProceso(argsPlanificacion* args, char* path, int* contadorProcesos)
{
    //Inicializo los valores para que no de error
    t_PCB* PCB = inicializarPCB(contadorProcesos, args->valores);

    //Envio el path a memoria
    enviarPath(args->logger, path, (PCB->PID), args->conexiones.conexionMemoria);

    //La operacion dira si es correcto o no
    if(recibirOperacion(args->conexiones.conexionMemoria) == ENCONTRO_PATH)
    {
        //Encolo el proceso en New e incremento el contador
        encolarNew(PCB, args->colaNew, args->logger);
        (*contadorProcesos)++;
    }

    else
    {
        free(PCB);
    }
}

void finalizarProcesoEspecifico(argsPlanificacion* args, char* PID)
{
    char *endptr;

    //Paso el PID a un int
    int PIDFinal = strtol(PID, &endptr, 10);

    if(PIDFinal == *(args->PIDExecuting))
    {
        //No se esta ejecutando ningun proceso, vamos a finalizarlo
	    *(args->PIDExecuting) = -1;

        //Interrupo el proceso en CPU
        enviarInterrupcionKernelCPU(INTERRUPCION_FINALIZAR, args->conexiones.conexionInterrupt);
    }

    else
    {
        //Busco en las colas el proceso con ese PID
        hallarProcesoEnColas(args, PIDFinal);
    }
}

void hallarProcesoEnColas(argsPlanificacion* args, int PID)
{
    //Recorro todas las listas, si lo encuentro pongo encontrado en 1
    bool encontrado = false;

    //Se fija en cada cola si se encuentra el proceso buscado

    //Uso el mutex para evitar condicion de carrera
    pthread_mutex_lock(&(args->colaNew->mutex));
    if(!list_is_empty(args->colaNew->lista))
    {
        encontrado = recorrerLaCola(args, args->colaNew, PID, NEW);
    }
    pthread_mutex_unlock(&(args->colaNew->mutex));

    pthread_mutex_lock(&(args->colaReady->mutex));
    if(!list_is_empty(args->colaReady->lista) && !encontrado)
    {
        encontrado = recorrerLaCola(args, args->colaReady, PID, READY);
    }
    pthread_mutex_unlock(&(args->colaReady->mutex));

    pthread_mutex_lock(&(args->colaBlocked->mutex));
    if(!list_is_empty(args->colaBlocked->lista) && !encontrado)
    {
        encontrado = recorrerLaCola(args, args->colaBlocked, PID, BLOCKED);
    }
    pthread_mutex_unlock(&(args->colaBlocked->mutex));

    pthread_mutex_lock(&(args->colaPrioritaria->mutex));
    if(!list_is_empty(args->colaPrioritaria->lista) && !encontrado)
    {
        encontrado = recorrerLaCola(args, args->colaPrioritaria, PID, PRIORITARIA);
    }
    pthread_mutex_unlock(&(args->colaPrioritaria->mutex));
}

void finalizarProcesoSegunEstado(argsPlanificacion* args, t_lista* colaGenerica, t_PCB* PCB, colasEstado flag)
{
    switch(flag)
    {
        case NEW:
            //Indico que se finalizo un proceso de New (NO PERMITE QUE OTRO PROCESO ENTRE A READY)
            sem_trywait(&semPCBNew);

            //Libero el PCB que se finalizo
	        free(PCB);
            break;

        case READY:
            finalizarProceso(args, PCB);

            //Indico que hay un proceso menos en Ready
            sem_trywait(&semPCBMultiProgramacion);
            break;

        case BLOCKED:
            finalizarProceso(args, PCB);
            break;

        case PRIORITARIA:
            finalizarProceso(args, PCB);
                    
            //Indico que hay un proceso menos en Ready (Prioritaria es Ready, solo que otra cola)
            sem_trywait(&semPCBReady);
            break;

        default:
            break;
    }
}

//Recorro la cola, si lo encuentro retorna 1, sino retorna 0
bool recorrerLaCola(argsPlanificacion* args, t_lista* colaGenerica, int PID, colasEstado flag)
{
    t_PCB* PCB = NULL;

    //Este int retorna si encontre el PCB o no
    bool encontrado = false;

    //Obtengo la longitud para el for
    int longitud = list_size(colaGenerica->lista);

    for(int i = 0; i < longitud; i++)
    {
        //Obtengo el PCB de la lista sin removerlo
        PCB = list_get(colaGenerica->lista, i);

        if(PCB->PID == PID)
        {
            //Si encontre el PCB lo remuevo de la lista y lo marco como encontrado
            PCB = list_remove(colaGenerica->lista, i);
            encontrado = true;
            break;
        }
    }

    if(encontrado == true)
    {
        log_info(args->logger, "FINALIZA EL PROCESO %d - Motivo: INTERRUPTED_BY_USER", PCB->PID);
        finalizarProcesoSegunEstado(args, colaGenerica, PCB, flag);
    }

    return encontrado;
}

//Funcion que cambia el grado de multiprogramacion
void modificarMultiprogramacion(argsPlanificacion* args, int operando)
{
    //Dependiendo de si el grado nuevo es mayor o menor, lo aumento o disminuyo, si es igual no hago nada. Luego, actualizo al nuevo grado de Multiprogramacion
    if(args->valores.gradoMultiProgramacion < operando)
    {
        aumentarGrado(args, operando);
        args->valores.gradoMultiProgramacion = operando;
    }

    else if(args->valores.gradoMultiProgramacion > operando)
    {
        disminuirGrado(args, operando);
        args->valores.gradoMultiProgramacion = operando;
    }
}

//Funcion que aumenta el grado de multiprogramacion si el valor ingresado es mayor al actual
void aumentarGrado(argsPlanificacion* args, int operando)
{
    //Obtengo cuantas veces tengo que aumentar el grado
    int iteraciones = operando - args->valores.gradoMultiProgramacion;
    log_info(args->logger, "Aumento en: %d", iteraciones);

    //Por aumento hago un sem_post
    for(int i=0; i < iteraciones; i++)
    {
        sem_post(&semPCBMultiProgramacion);
    }
}

//Funcion que aumenta el grado de multiprogramacion si el valor ingresado es mayor al actual
void disminuirGrado(argsPlanificacion* args, int operando)
{
    //Obtengo cuantas veces tengo que dismunuir el grado
    int iteraciones = args->valores.gradoMultiProgramacion- operando;
    log_info(args->logger, "Disminuyo en: %d", iteraciones);

    //Cuando finaliza el proceso, tengo que ver cuanto se disminuyo el grado, hasta que se acomode
    args->overFlowProgramacion+= iteraciones;
}

//Funcion PROCESO_ESTADO
void procesoEstado(argsPlanificacion* args)
{
    //Para cada cola me fijo si tiene elementos, si esta vacia no muestro nada
    if(!list_is_empty(args->colaNew->lista))
    {
        log_info(args->logger, "PROCESOS EN NEW:");
        mostrarProcesos(args->logger, args->colaNew);
    }

    if(!list_is_empty(args->colaReady->lista) || !list_is_empty(args->colaPrioritaria->lista))
    {
        log_info(args->logger, "PROCESOS EN READY:");

        if(!list_is_empty(args->colaReady->lista))
        {
            mostrarProcesos(args->logger, args->colaReady);
        }

        if(!list_is_empty(args->colaPrioritaria->lista))
        {
            mostrarProcesos(args->logger, args->colaPrioritaria);
        }
    }

    if(!list_is_empty(args->colaBlocked->lista))
    {
        log_info(args->logger, "PROCESOS EN BLOCKED:");
        mostrarProcesos(args->logger, args->colaBlocked);
    }

    if(*(args->PIDExecuting) != -1)
    {
        //En execute solo puede haber un proceso a la vez
        log_info(args->logger, "PROCESO EN EXECUTE:");
        log_info(args->logger, "<PROCESO %d>", *(args->PIDExecuting));
    }
}

//Funcion para mostrar procesos de cada cola
void mostrarProcesos(t_log* logger, t_lista* colaGenerica)
{
    //Uso un mutex para que no haya condicion de carrera
    pthread_mutex_lock(&(colaGenerica->mutex));

    int cantidadProcesos = list_size(colaGenerica->lista);

    t_PCB* PCB;

    //Luego de obtener cuantos procesos tengo, en un for muestro todos, hago list_get ya que toma los procesos pero no los saca permanentemente
    for(int i = 0; i < cantidadProcesos; i++)
    {
        PCB = list_get(colaGenerica->lista, i);
        log_info(logger, "PROCESO %d", PCB->PID);
    }

    pthread_mutex_unlock(&(colaGenerica->mutex));
}