//------------BIBLIOTECAS------------

#include <../include/mmu.h>

//------------DEFINICION DE FUNCIONES------------

//Funcion para traducir la direccion logica a fisica
int traducirDirecLogica(int direcLogica, int tamPagina, void* tlb, int pid,  int numEntradasTlb, char* algoritmoTlb, int conexionMemoria, t_log* logger)
{   
    
    //Caclculo el numero de pagina 
    int numPag = floor(direcLogica / tamPagina);
    
    //Creo una variable para el numero de marco
    int numMarco;
    if(tlb != NULL)
    {
        //Calculo el marco buscandolo en la tlb
        numMarco = calcularNumMarco(numPag, tlb, pid, numEntradasTlb, algoritmoTlb, conexionMemoria, logger);
    }

    else
    {
        //Le pido el marco a memoria
        numMarco = pedirMarcoMemoria(pid, numPag, conexionMemoria);
    }

    //Calculo el desplazamiento 
    int desplazamiento = direcLogica - numPag * tamPagina;

    //Calculo la direccion fisica
    int direcFisica = numMarco * tamPagina + desplazamiento;

    //Devuelvo la direccion fisica
    return direcFisica;
    
   

    
}