//------------BIBLIOTECAS------------

#include <../include/tlb.h>

int entradaAReemplazar = 1;

//------------DEFINICION DE FUNCIONES------------

//Funcion para reiniciar el aging de una entrada
void reiniciarAgingUsado(int numMarco, void* tlb)
{   
    //Variables para mover el puntero
    int posicionDeMarco = 2;
    int tamanoFila = 4;

    //Creo un puntero que apunte al principio de la tlb
    int* ptr = tlb;

    //Lo muevo a la columna de los numeros de marco
    ptr += posicionDeMarco;

    //Itero a lo largo de la tlb hasta encontrar el marco en el que hice hit
    while(*ptr != numMarco)
    {
        ptr += tamanoFila;
    }

    //Muevo el puntero una posicion para apuntar a la age de esa fila
    ptr ++;

    //La reinicio a 0
    *ptr = 0;
}

//Funcionar para sumar 1 en la ultima columna de toda la tlb
void sumarAging(void* tlb, int numEntradasTlb)
{   
    //Variables para mover el puntero
    int posicionDeAge = 3;
    int tamanoFila = 4;

    //Creo un puntero que apunte al principio de la tlb
    int* ptr = tlb;

    //Muevo el puntero a las filas de la age
    ptr += posicionDeAge;

    //Creo un iterador para iterar a lo largo de la tlb
    int i = 0;

    //Itero a lo largo de la tlb y le sumo en 1 la age de cada fila
    while(*ptr != -1 && i < numEntradasTlb)
    {   
        //Sumo uno a la edad de la fila
        *ptr = *ptr + 1;

        //Avanzo a la proxima fila y aumento el iterador
        ptr += tamanoFila;
        i++;
    }
}

//Funcion para calcular la entrada mas vieja para aplicar LRU
int obtenerEntradaMasVieja(void* tlb,int  numEntradasTlb)
{   
    //Inicializo la entrada a reemplazar en 1, asi si no entra al if la 1 seria la de mayor age
    int entradaAReemplazarLRU = 1;

    //Variables para mover el puntero
    int posicionDeAge = 3;
    int tamanoFila = 4;

    //Creo un puntero que apunte al principio de la tlb
    int* ptr = tlb;

    //Muevo el puntero a la segunda fila en la columna de la age
    ptr += posicionDeAge + tamanoFila;

    //Creo en una variable para guardar la age de la primera fila
    int ageMax = *(ptr - 4);

    //Itero a lo largo de la tlb y comparo si la age de la fila anterior es mayor al de la siguiente
    for(int i = 0; i < numEntradasTlb - 1; i++)
    {   
        //Guardo en una variable la age de la fila actual
        int age2 = *(ptr + 4 * i);

        //Lo comparo con la fila anterior
        if(ageMax < age2)
        {   
            //Si es igual actualizo la variable ageMax y actualizo la entrada a reemplazar
            ageMax = age2;
            entradaAReemplazarLRU = i + 2;
        }
    }
    //Devuelvo la entrada a reemplazar
    return entradaAReemplazarLRU;
}

//Funcion para serializar el PID y el numero de pagina 
void* serailizarPIDYNumPag(int pid, int numPag)
{   
    //Reservo el espacio necesario para el stream
    void* stream = malloc(sizeof(int) * 2 + sizeof(op_code));

    //Le digo su codigo de operacion
    op_code codeOp = PETICION_MARCO;

    //Copio en el stream los valores del PID y el numero de pagina
    memcpy(stream, &codeOp, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &pid, sizeof(int));
    memcpy(stream + sizeof(op_code) + sizeof(int), &numPag, sizeof(int));

    //Retorno el stream creado
	return stream;
}

//Funcion para enviar el PID y el numero de pagina a memoria
void enviarPIDYNumPag(int conexionMemoria, int pid, int numPag)
{   
    //creo un estream con el PID y el numero de pagina serializados
    void* stream = serailizarPIDYNumPag(pid, numPag);

    //Envio el stream ya cargado
    send(conexionMemoria, stream, sizeof(int) * 2 + sizeof(op_code), 0);

    //Libero el stream
    free(stream);
}

//Funcion para crear la tlb
void* crearTlb(int numEntradasTlb)
{   
    //Tamano de cada fila de la tlb
    int tamanoFila = 3 * sizeof(int) + sizeof(uint32_t);

    //Reservo espacio de memoria para la tlb
    void* espacioTlb = malloc(numEntradasTlb * tamanoFila + 1);

    //Inicializo la tlb en valores negativos para saber que filas estan libres
    int valorNegativo = -1;

    for(int i = 0; i < numEntradasTlb; i++)
    {
        memcpy(espacioTlb + tamanoFila * i, &valorNegativo, sizeof(int));
        memcpy(espacioTlb + sizeof(int) + tamanoFila * i, &valorNegativo, sizeof(int));
        memcpy(espacioTlb + sizeof(int) * 2 + tamanoFila * i, &valorNegativo, sizeof(int));
        memcpy(espacioTlb + sizeof(int) * 3 + tamanoFila * i, &valorNegativo, sizeof(int));
    }

    //Devuelvo la tlb con el espacio de memoria reservado
    return espacioTlb;
}

//Funcion para pedirle el marco a memoria debido a que no se encuentra en la tlb
int pedirMarcoMemoria(int pid, int numPag, int conexionMemoria)
{   
    //Envio el PID y el numero de pagina a memoria para que me devuelva el marco
    enviarPIDYNumPag(conexionMemoria, pid, numPag);

    //Recibo la respuesta de memoria y corroboro que el codigo de operacion sea el correcto
    if(recibirOperacion(conexionMemoria) != ENVIO_MARCO)
    {
        perror("Codigo de operacion invalido");
        exit(EXIT_FAILURE);
    }

    //Creo una variable para poder recibir el marco
    int numMarco;

    //Recibo el vnumero de marco
    recv(conexionMemoria, &numMarco, sizeof(int), MSG_WAITALL);

    //Devuelvo el numero de marco
    return numMarco;
}

//Funcion para calcular el marco debido a que no se encuentra en la tlb
int calcularNumMarco(int numPag, void* tlb, int pid, int numEntradasTlb, char* algoritmo, int conexionMemoria, t_log* logger)
{   
    //Variables para mover el puntero
    int posicionDePagina = 1;
    int posicionDeMarco = 2;
    int tamanoFila = 4;
    
    //Creo un puntero apuntando al principio de la tlb para recorrerla con mas facilidad
    int* ptr = tlb;

    //Inicializo el marco en -1 para saber si esta o no presente en la tlb
    int numMarco = -1;

    //Recorro la tlb buscando si se encuentra la pagina del proceso buscado
    for(int i = 0; i < numEntradasTlb; i++)
    {   
        //Si la pagina del proceso buscado coinicide con alguna de las filas de la tlb, cambio el numero de marco por el que se enceuntra en la tlb
        if(pid == *ptr && numPag == *(ptr + posicionDePagina))
        {   
            //Actualizo el numero de marco
            numMarco = *(ptr + posicionDeMarco);
        }
        //Avanzo una fila en la tlb 
        ptr += tamanoFila;
    }

    //Si el numero de marco sigue siendo -1, no se logro encontrar la pagina del proceso buscado en la tlb
    if((numMarco == -1))
    {   
        //Log obligatorio
        log_info(logger, "PID: %i - TLB MISS - Pagina: %i", pid, numPag);

        //Le pido el marco a memoria para
        numMarco = pedirMarcoMemoria(pid, numPag, conexionMemoria);

        //Corro el algoritmo correspondiente
        correrAlgoritmoTlb(tlb, algoritmo, numEntradasTlb, pid, numPag, numMarco, 1);
    }
    
    else if((numMarco != -1))
    {   
        //Log obligatorio
        log_info(logger, "PID: %i - TLB HIT - Pagina: %i", pid, numPag);  

        //Corro el algoritmo correspondiente
        correrAlgoritmoTlb(tlb, algoritmo, numEntradasTlb, pid, numPag, numMarco, 0);
    }

    //Log obligatorio
    log_info(logger, "PID: %i - OBTENER MARCO - Página: %i - Marco: %i", pid, numPag, numMarco);

    //Devuelvo el numero de marco
    return numMarco;
}

//Funcion para realizar el algoritmo FIFO
void correrAlgoritmoFIFO(void* tlb, int numEntradasTlb, int pid, int numPag, int numMarco)
{
    //Variables para mover el puntero
    int posicionDePagina = 1;
    int posicionDeMarco = 2;
    int tamanoFila = 4;
    
    //Creo un puntero apuntando al principio de la tlb para recorrerla con mas facilidad
    int* ptr = tlb;

    //Creo un iterador parar recorrer las filas/entradas de la tlb
    int i = 0;

    //Creo un while que recorre la tlb siempre y cuando no llegue al final ni encuentre una entrada/fila vacia
    while(*ptr != -1 && i < numEntradasTlb - 1)
    {   
            //Corro 3 posicion el puntero para saltar a la proxima entrada/fila
            ptr += tamanoFila;

            //Aumento i para corroborar de no pasarme del limite de la tlb
            i++;
    }

    //Si el ptr apunta a un numero diferente a -1 significa que esta apuntando al PID de la ultima entrada/fila, por lo que
    //ecorrio toda la tlb sin poder encontrar una fila/entrada vacia, asi que va a haber que reemplazar una ya usada
    if(*ptr != -1)
    {   
        //Reinicio el puntero al inicio de la tlb
        ptr = tlb;

        //Apunto hacia la fila que debo reemplazar segun el algoritmo
        ptr += (entradaAReemplazar - 1) * tamanoFila;

        //Si el numero de fila a reemplazar es la ultima, vuelvo al principio para seguir con la logica del algoritmo
        if(entradaAReemplazar == numEntradasTlb)
        {   
            //Reincio la variable de la enrada que debo reemplazar
            entradaAReemplazar = 0;
        }

        //Le sumo un valor para que en el proximo reemplazo, se reemplace la siguiente fila que corresponda
        entradaAReemplazar++;   
    }
    //De no haber entrado en el anterior if, el puntero salio del while apuntando a una fila que tiene como primer valor el -1
    //Esto quiere decir que esta fila no fue usada todavia, por lo que esta libre y se puede escribir en ella sin realizar un reemplazo


    //Cargo los datos correspondientes en el puntero, el cual apunta a diferentes direcciones dependiendo de si entro o no al anterior if

    //Cargo el pid en el primer espacio
    *ptr = pid;

    //Cargo el numero de pagina en el segundo espacio
    *(ptr + posicionDePagina) = numPag;

    //Cargo el numero de marco en eltercer espacio
    *(ptr + posicionDeMarco) = numMarco;
}

//Funcion para realizar el algoritmo LRU
void correrAlgoritmoLRU(void* tlb, int numEntradasTlb, int pid, int numPag, int numMarco, int miss)
{   
    //Variables para mover el puntero
    int posicionDePagina = 1;
    int posicionDeMarco = 2;
    int posicionDeAge = 3;
    int tamanoFila = 4;

    //Creo un puntero apuntando al principio de la tlb para recorrerla con mas facilidad
    int* ptr = tlb;

    //Creo un iterador para iterar por la tlb
    int i = 0;

    if(miss == 1)
    {   
        //Creo un while que recorre la tlb siempre y cuando no llegue al final ni encuentre una entrada/fila vacia
        while(*ptr != -1 && i < numEntradasTlb - 1)
        {   
            //Corro 3 posicion el puntero para saltar a la proxima entrada/fila
            ptr += tamanoFila;

            //Aumento i para corroborar de no pasarme del limite de la tlb
            i++;
        }

        //Si el puntero es diferente de 1, no hay ninguna fila vacia
        if(*ptr != -1)
        {   
            //Busco la entrada a reemplazar
            int entradaAReemplazarLRU = obtenerEntradaMasVieja(tlb, numEntradasTlb);

            //Reinicio el ptr al principio de la tlb
            ptr = tlb;

            //lo muevo a la entrada a reemplazar
            ptr += (entradaAReemplazarLRU - 1) * tamanoFila;
         }
        
        //Cargo el pid en el primer espacio
        *ptr = pid;

        //Cargo el numero de pagina en el segundo espacio
        *(ptr + posicionDePagina) = numPag;

        //Cargo el numero de marco en eltercer espacio
        *(ptr + posicionDeMarco) = numMarco;

        //Inicio el contador de aging
        *(ptr + posicionDeAge) = 0;
    } 

    //Si hay hit, solo se reincia la age de ña fila en la que se hizo hit
    else if (miss == 0)
    {   
        //Reinicio la age 
        reiniciarAgingUsado(numMarco, tlb);
    }

    //Sumo un age a cada fila
    sumarAging(tlb, numEntradasTlb);
}

//Funcion que crea el hilo del algorito para la TLB
void correrAlgoritmoTlb(void* tlb, char* algoritmoTlb, int numEntradasTlb, int pid, int numPag, int numMarco, int miss)
{   

    //Si el algoritmo es FIFO entra en el if
    if(strcmp(algoritmoTlb, "FIFO") == 0 && miss == 1)
    {   
        correrAlgoritmoFIFO(tlb, numEntradasTlb, pid, numPag, numMarco);
    }

    //Si el algoritmo es LRU entra en este otro if
    else if(strcmp(algoritmoTlb, "LRU") == 0)
    {
        correrAlgoritmoLRU(tlb, numEntradasTlb, pid, numPag, numMarco, miss);

    }
}