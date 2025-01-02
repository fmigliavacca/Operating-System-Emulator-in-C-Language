#ifndef CPU_ESTRUCTURAS_H_
#define CPU_ESTRUCTURAS_H_

//Struct del config
typedef struct
{
    int cantidadEntradasTlb;
    char* ipMemoria;
    char* algoritmoTlb;
    char* puertoMemoria;
    char* puertoEscuchaDispatch;
    char* puertoEscuchaInterrupt;
}t_valores;

//Struct de la instruccion
typedef struct
{
    char* operandos;
    char* instruccion;
}t_instruccionEntera;  

//Struct de los operandos de una instruccion
typedef struct 
{
    char* operando1;
    char* operando2;
    char* operando3;
    char* operando4;
    char* operando5;
}t_operandos;

typedef enum
{
    SET,
    SUM,
    SUB,
    JNZ,
    EXIT,
    WAIT,
    SIGNAL,
    RESIZE,
    MOV_IN,
    MOV_OUT,
    IO_FS_READ,
    IO_FS_WRITE,
    COPY_STRING,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_FS_TRUNCATE,
    IO_STDOUT_WRITE
}t_enumInstrucciones;

typedef struct
{
    void* tlb;
    int pid;
    int numPag;
    int numMarco;
    int numEntradasTlb;
}t_argsHiloFIFO;

#endif