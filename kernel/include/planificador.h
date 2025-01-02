#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

//------------BIBLIOTECAS------------

#include <../../utils/include/utils.h>
#include <../../utils/include/comunicacion.h>
#include <../../kernel/include/estructuras.h>

//------------DECLARACION DE SEMAFOROS------------

extern sem_t semPCBNew;
extern sem_t semPCBReady;
extern sem_t semAtenderIO;
extern sem_t semPCBBlocked;
extern sem_t semPlanificador;
extern sem_t semPCBExecuting;
extern sem_t semPCBMultiProgramacion;

//------------DECLARACION DE ESTRUCTURAS PLANIFICACION------------

//Estructura para los argumentos del hilo del Quantum
typedef struct 
{
	int* PIDExecuting;
	char* pathCarpeta;
	int overFlowProgramacion;
	t_log* logger;
	t_lista* colaNew;
	t_lista* colaReady;
	t_lista* colaBlocked;
	t_lista* listaRecursos;
	t_lista* interfacesSTDIN;
	t_lista* colaPrioritaria;
	t_lista* interfacesSTDOUT;
	t_lista* interfacesDIALFS;
	t_lista* interfacesGenericas;
	t_valores valores;
	tipo_comunicacion rol;
	t_conexiones conexiones;
}argsPlanificacion;

//Estructura para los argumentos del hilo del Quantum
typedef struct 
{
	t_log* logger;
	t_lista* colaNew;
	t_lista* colaReady;
}argsHiloReady;

//Estructura para los argumentos del hilo del Quantum
typedef struct 
{
	int estado;
    int quantum;
	int conexion;
	t_log* logger;
	pthread_t* hilo;
}argsQuantum;

//Estructura para los argumentos de las interfaces IO
typedef struct 
{
    char* operando1;
    char* operando2Nombre;
	u_int32_t operando2;
    u_int32_t operando3;
    u_int32_t operando4;
    u_int32_t operando5;
	tipoInstruccionInterfaz instruccion;
}t_operandosKernel;

//Estructura para los argumentos del hilo de Interfaces
typedef struct 
{
	int PID;
	int quantum;
	char* algoritmo;
	t_log* logger;
	t_lista* colaReady;
	t_lista* colaBlocked;
	t_lista* listaInterfaz;
	t_lista* colaPrioritaria;
	t_operandosKernel* operandos;
}argsAtencionIO;

//Estructura para administrar los recursos
typedef struct
{
	char* nombre;
	t_list* PID;
	sem_t disponible;
}t_recursos;

//Estructura para los argumentos del manejo de recursos
typedef struct
{
	t_log* logger;
	t_PCB* proceso;
	t_lista* blocked;
	t_lista* listaRecursos;
	t_lista* listaPlanificador;
	t_recursos* recurso;
}argsManejoRecursos;

//------------DECLARACION DE FUNCIONES------------

//FUNCION PRINCIPAL

void* hiloPlanificador(void*);

//FUNCIONES PARA ENCOLAR PROCESOS

void mostrarPIDS(t_log*, t_list*);
void encolarNew(t_PCB*, t_lista*, t_log*);
void encolarBlocked(t_PCB*, t_lista*, t_log*);
void agregarPCBAReady(t_log*, t_lista*, t_PCB*);
void agregarPrimeroEnReady(argsManejoRecursos*);
t_PCB* obtenerPCBAReady(t_lista*, t_log*);

//FUNCIONES PARA EL HILO DE PASAR A READY

void* hiloPasarAReady(void*);
void crearHiloPasarAReady(argsPlanificacion* args);

//PLANIFICADORES

int algoritmoFIFO(argsPlanificacion*, t_lista*);
void planificadorFIFO(argsPlanificacion*);
void planificadorRR(argsPlanificacion*);
void planificadorVRR(argsPlanificacion*);

//FUNCIONES PARA EL HILO DEL QUANTUM

void* hiloQuantum(void*);
void enviarInterrupcionKernelCPU(op_code, int);
argsQuantum* crearHiloQuantum(int, int, t_log*);

//FUNCIONES PARA RECIBIR EL CONTEXTO DE EJECUCION

void avisoFinalizacion(int, uint32_t);
void verificarPlanificacionActiva(void);
void recibirContextoDelPCB(argsPlanificacion*);
void finalizarProceso(argsPlanificacion*, t_PCB*);
op_code checkFinalizacion(argsPlanificacion*, op_code);

//FUNCIONES PARA EL MANEJO DE INTERFACES

void* hiloInterfaz(void*);
void recepcionTipoYNombre(argsPlanificacion*, t_PCB*);
void recepcionInterfaz(argsPlanificacion*, t_PCB*, t_temporal*);
void realizarInstruccionIO(argsAtencionIO*, t_elemento_interfaces*);
void crearHiloInterfaz(argsPlanificacion*, t_operandosKernel*, tipo_interfaz, t_PCB*);
bool existeEnLista(t_lista*, char*);
bool verificarProcesoFinalizado(t_lista*, int);
t_PCB* obtenerPCBPorPID(t_lista*, int);
tipo_interfaz buscarInterfaz(argsPlanificacion*, char*);
t_lista* devolverColaInterfaz(argsPlanificacion*, tipo_interfaz);
t_operandosKernel* recibirOperandosRestantes(argsPlanificacion*, t_operandosKernel*);
t_operandosKernel* obtenerInstruccionInterfaz(argsPlanificacion*, tipo_interfaz, char*);

//FUNCIONES PARA EL MANEJO DE RECURSOS

int* buscarInstanciaPID(int, t_list*);
void* pedirRecursoHilo(void*);
void liberarTodososRecursos(t_PCB*, t_lista*);
void pedirRecurso(argsManejoRecursos*, t_temporal*);
void liberarRecurso(argsManejoRecursos*, bool, t_temporal*);
void recepcionRecursos(argsPlanificacion*, t_PCB*, t_temporal*);
t_recursos* buscarRecurso(char*, t_lista*);
t_recursos* tieneAlgunRecurso(t_PCB*, t_lista*);
t_recursos* buscarRecursoPorNombre(char*, t_lista*);

#endif