#ifndef CICLODEINSTRUCCION_H_
#define CICLODEINSTRUCCION_H_

//------------BIBLIOTECAS------------

#include <../include/mmu.h>
#include <../include/estructuras.h>
#include <../../utils/include/comunicacion.h>

//------------DECLARACION DE FUNCIONES------------

//FUNCIONES COMUNICACION

int enviarMOV_IN(uint8_t, int, int, uint32_t);
int recibirRespuestaRESIZE(int, t_PCB*, int*);
char* recibirStreamInstruccion(int, t_log*);
void enviarPCyPID(t_PCB, int, t_log*);
void enviarRESIZE(uint32_t, int, int );
void enviarPCBConCodOp(t_PCB, op_code, int);
void enviarMOV_OUT(uint8_t, int, int, int, uint32_t);
void enviarIO_FS_TRUNCATE(t_operandos, int, uint32_t);
void enviarIO_GEN_SLEEP(t_operandos, tipo_interfaz, int);
void enviarWAITOSIGNAL(char*, int, tipoInstruccionesRecursos);
void enviarSTDINOSTDOUT(tipo_interfaz, char*, uint32_t, uint32_t, int); 
void enviarIO_FS_CREATEoIO_FS_DELETE(t_operandos, int, tipoInstruccionInterfaz);
void enviarCOPY_STRING(int, uint32_t, uint32_t, uint32_t, int, uint8_t, t_log*);
void enviarIO_FS_WRITEoIO_FS_READ(t_operandos, int, uint32_t, uint32_t, uint32_t, tipoInstruccionInterfaz);

//FUNCIONES SERIALIZACION

void* serializarRESIZE(uint32_t, int);
void* serializarPCyPID(uint32_t, uint32_t);
void* serializarPCBConCodOp(t_PCB, op_code);
void* serializarMOV_IN(uint8_t, int, uint32_t);
void* serializarMOV_OUT(uint8_t, int, int, uint32_t);
void* serializarIO_FS_TRUNCATE(t_operandos, uint32_t);
void* serializarIO_GEN_SLEEP(t_operandos, tipo_interfaz);
void* serializarWAITOSIGNAL(char*, tipoInstruccionesRecursos, int);
void* serializarSTDINOSTDOUT(tipo_interfaz, char*, uint32_t, uint32_t);
void* serializarIO_FS_CREATEoIO_FS_DELETE(t_operandos, tipoInstruccionInterfaz);
void* serializarIO_FS_WRITEoIO_FS_READ(t_operandos, uint32_t, uint32_t, uint32_t, tipoInstruccionInterfaz);

//FUNCIONES CICLO DE EJECUCION

int calcularTamanoOPerando(char*);
char* etapaFetch(t_PCB*,int, t_log*);
void actualizarTLB(void*, int, int, int);
void realizarInstruccionEXIT(t_PCB*, int*);
void realizarInstruccionSET(char*, t_dictionary*);
void realizarInstruccionSUM(char*, t_dictionary*);
void realizarInstruccionSUB(char*, t_dictionary*);
void realizarInstruccionJNZ(char*, t_dictionary*);
void manejarInterrupciones(t_PCB, int, int*, t_log*);
void realizarInstruccionWAIT(char*, int, t_PCB*, int*);
void realizarInstruccionSIGNAL(char*, int, t_PCB*, int*);
void realizarInstruccionIO_GEN_SLEEP(t_PCB*, char*, int, int*);
void cargarDiccionarioRegistros(t_registrosDeCPU*, t_dictionary*);
void realizarInstruccionIO_FS_TRUNCATE(t_PCB*, char*, int, t_dictionary*, int*);
void realizarInstruccionRESIZE(char*, t_dictionary*, t_PCB*, int, int*, void*, int);
void realizarInstruccionMOV_IN(char*, t_dictionary*, void*, int, int, char*, int, int, t_log*);
void realizarInstruccionMOV_OUT(char*, t_dictionary*, void*, int, int, char*, int, int, t_log*);
void realizarInstruccionCOPY_STRING(char*, t_dictionary*, int, void*, int, int, char*, int, t_log*);
void realizarInstruccionIO_FS_CREATEoIO_FS_DELETE(t_PCB*, char*, int, tipoInstruccionInterfaz, int*);
void realizarCicloInstruccion(t_PCB*, t_dictionary*, int, int, t_log*, void*, int, int, int, char*, int*);
void realizarInstruccionIO_FS_WRITEoIO_FS_READ(t_PCB*, t_dictionary*, char*, int, tipoInstruccionInterfaz, int*);
void etapaExecute(t_instruccionEntera*,t_PCB*, t_dictionary*, int, void*, int, int, int, char*, int, t_log*, int*);
void realizarInstruccion(t_instruccionEntera*, t_PCB*, t_dictionary*, int, void*, int, int, int, char*, int, t_log*, int*);
void realizarInstruccionIO_STDIN_READoIO_STDOUT_WRITE(t_PCB*, char*, t_dictionary*, void*, int, int, char*, int, int, t_log*, tipo_interfaz, int*);
t_operandos separarOperandos(char*);
t_instruccionEntera* etapaDecode(char*);
t_instruccionEntera* separarInstruccion(char*);

#endif