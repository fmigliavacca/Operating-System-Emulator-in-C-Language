#include <../include/utils.h>

//Inicializar cola y su semaforo
t_cola* inicializarColasYSemaforos()
{
    //Cada cola tiene su mutex y su respectiva cola
    t_cola* colaGenerica = malloc(sizeof(t_cola));
    colaGenerica->cola = queue_create();
    pthread_mutex_init(&(colaGenerica->mutex), NULL);
    sem_init(&(colaGenerica->elementos), 0, 0);
    return colaGenerica;
}

//Inicializar lista y su semaforo
t_lista* inicializarListasYSemaforos()
{
    //Cada lista tiene su mutex y su respectiva lista
    t_lista* listaGenerica = malloc(sizeof(t_lista));
    listaGenerica->lista = list_create();
    pthread_mutex_init(&(listaGenerica->mutex), NULL);
    return listaGenerica;
}