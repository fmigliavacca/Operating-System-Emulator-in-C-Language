#include <../include/serializacion.h>

//Funcion para serializar un paquete
void* serializarPaquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void* serializarFS_CREATE(char* nombreArchivo, u_int32_t PID)
{
	int longitudNombre = strlen(nombreArchivo) + 1;

	tipoDePeticion cop = CREAR_ARCHIVOFS;

	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(int) + longitudNombre);

	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(tipoDePeticion), &PID, sizeof(uint32_t));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(uint32_t), &longitudNombre, sizeof(int));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(uint32_t) + sizeof(int), nombreArchivo, longitudNombre);

	return stream;
}

void* serializarFS_DELETE(char* nombreArchivo, u_int32_t PID)
{
	int longitudNombre = strlen(nombreArchivo) + 1;

	tipoDePeticion cop = BORRAR_ARCHIVOFS;

	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(uint32_t) + sizeof(int) + longitudNombre);

	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(tipoDePeticion), &PID, sizeof(uint32_t));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(uint32_t), &longitudNombre, sizeof(int));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(uint32_t) + sizeof(int), nombreArchivo, longitudNombre);

	return stream;
}

void* serializarFS_TRUNCATE(char* nombreArchivo, uint32_t tamanioNuevo, u_int32_t PID)
{
	int longitudNombre = strlen(nombreArchivo) + 1;

	tipoDePeticion cop = TRUNCAR_ARCHIVO;

	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(int) + longitudNombre + sizeof(uint32_t));

	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(tipoDePeticion) , &PID, sizeof(uint32_t));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(u_int32_t) , &longitudNombre, sizeof(int));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(int), nombreArchivo, longitudNombre);
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(int) + longitudNombre, &tamanioNuevo, sizeof(uint32_t));

	return stream;
}

void* serializarFS_WRITE(char* nombreArchivo, uint32_t PID, uint32_t dirLogica, uint8_t tamanioAEscribir, uint32_t punteroArchivo)
{
	int longitudNombre = strlen(nombreArchivo) + 1;

	tipoDePeticion cop = ESCRIBIR_ARCHIVO;

	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 3 + sizeof(uint8_t));

	memcpy(stream, &cop, sizeof(tipoDePeticion));
    memcpy(stream + sizeof(tipoDePeticion), &longitudNombre, sizeof(int));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int), nombreArchivo, longitudNombre);
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre, &PID, sizeof(uint32_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t), &dirLogica, sizeof(uint32_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 2, &tamanioAEscribir, sizeof(uint8_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 2 + sizeof(uint8_t), &punteroArchivo, sizeof(uint32_t));

	return stream;
}

void* serializarFS_READ(char* nombreArchivo, uint32_t PID, uint32_t dirLogica, uint8_t tamanioALeer, uint32_t punteroArchivo)
{
	int longitudNombre = strlen(nombreArchivo) + 1;

	tipoDePeticion cop = LEER_ARCHIVO;

	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 3 + sizeof(uint8_t));

	memcpy(stream, &cop, sizeof(tipoDePeticion));
    memcpy(stream + sizeof(tipoDePeticion), &longitudNombre, sizeof(int));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int), nombreArchivo, longitudNombre);
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre, &PID, sizeof(uint32_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t), &dirLogica, sizeof(uint32_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 2, &tamanioALeer, sizeof(uint8_t));
    memcpy(stream + sizeof(tipoDePeticion) + sizeof(int) + longitudNombre + sizeof(uint32_t) * 2 + sizeof(uint8_t), &punteroArchivo, sizeof(uint32_t));

	return stream;
}

//Funcion para serializar el PCB
void* serializarPCB(t_PCB pcb)
{	
	//Reservo el espacio necesario para el stream
	void* stream = malloc(sizeof(op_code) + sizeof(t_PCB));

	//Le digo su codigo de operacion
	op_code cop = PCB;

	//Copio en el stream todos los valores del PCB serializados
	memcpy(stream, &cop, sizeof(op_code));
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

//Funcion para deserializar el PCB
void deserializarPCB(void* stream, t_PCB* pcb)
{
	//Copio la cadena de bits del stream al puntero del PCB
	memcpy(&(pcb->PC), stream + sizeof(uint32_t) * 0 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->PID), stream + sizeof(uint32_t) * 1 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->quantum), stream + sizeof(uint32_t) * 2 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.PC), stream + sizeof(uint32_t) * 3 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.EAX), stream + sizeof(uint32_t) * 4 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.EBX), stream + sizeof(uint32_t) * 5 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.ECX), stream + sizeof(uint32_t) * 6 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.EDX), stream + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.SI), stream + sizeof(uint32_t) * 8 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.DI), stream + sizeof(uint32_t) * 9 + sizeof(uint8_t) * 0, sizeof(u_int32_t));
	memcpy(&(pcb->registrosDeCPU.AX), stream + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 0, sizeof(u_int8_t));
	memcpy(&(pcb->registrosDeCPU.BX), stream + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 1, sizeof(u_int8_t));
	memcpy(&(pcb->registrosDeCPU.CX), stream + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 2, sizeof(u_int8_t));
	memcpy(&(pcb->registrosDeCPU.DX), stream + sizeof(uint32_t) * 10 + sizeof(uint8_t) * 3, sizeof(u_int8_t));
}

//Funcion para serializar la interfaz
void* serializarInterfaz(int longitud, char* nombreInterfaz, tipo_interfaz tipoInterfaz) 
{
	//Reservo espacio en memoria para el stream
    void* stream = malloc(sizeof(op_code) + sizeof(int) + longitud + sizeof(tipo_interfaz));

	//Le digo su codigo de operacion
	op_code cop = INTERFAZ;

	//Copio en el stream los valores de las interfaces serializadas
	memcpy(stream, &cop, sizeof(op_code));
	memcpy(stream + sizeof(op_code), &longitud, sizeof(int));
	memcpy(stream + sizeof(op_code) + sizeof(int), nombreInterfaz, longitud);
	memcpy(stream + sizeof(op_code) + sizeof(int) + longitud, &tipoInterfaz, sizeof(tipo_interfaz));

	//Retorno el stream
	return stream;
}

//Funcion para deserializar la interfaz
void deserealizarInterfaz(void* stream, int longitud, char* nombreInterfaz, tipo_interfaz* tipoInterfaz)
{	
	memcpy(nombreInterfaz, stream, longitud);

	//Copio la cadena de bits del stream al puntero interfaz
	memcpy(tipoInterfaz, stream + longitud, sizeof(tipo_interfaz));
}

//Funcion para serializar un sleep
void* serializarSleep(u_int32_t PID, u_int8_t sleep)
{
	//Reservo el espacio necesario para el stream
	void* stream = malloc(sizeof(tipoDePeticion) + sizeof(u_int32_t) + sizeof(u_int8_t));

	//Le digo su codigo de operacion
	tipoDePeticion cop = PETICION_GENERICA;

	//Copio en el stream todos los valores del PCB serializados
	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(tipoDePeticion), &PID, sizeof(u_int32_t));
	memcpy(stream + sizeof(tipoDePeticion) + sizeof(u_int32_t), &sleep, sizeof(u_int8_t));

	//Retorno el stream creado
	return stream;
}

void* serializarSTDIN(t_peticion_std dirFisica)
{
	void* stream = malloc(sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t));

	tipoDePeticion cop = PETICION_STDIN;

	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 0, &dirFisica.PID, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 1, &dirFisica.direcFisica, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 2, &dirFisica.tamanio, sizeof(uint8_t));

	return stream;
}

void* serializarSTDOUT(t_peticion_std dirFisica)
{
	void* stream = malloc(sizeof(int) + sizeof(uint32_t) * 2 + sizeof(uint8_t));
	
	tipoDePeticion cop = PETICION_STDOUT;

	memcpy(stream, &cop, sizeof(tipoDePeticion));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 0, &dirFisica.PID, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 1, &dirFisica.direcFisica, sizeof(uint32_t));
	memcpy(stream + sizeof(int) + sizeof(uint32_t) * 2, &dirFisica.tamanio, sizeof(uint8_t));

	return stream;
}

//Funcion para deserializar un sleep
void deserializarSleep(void* stream, u_int8_t* sleep)
{
	//Copio la cadena de bits del stream al puntero del sleep
	memcpy(sleep, stream, sizeof(u_int8_t));

}

//Descerializa el identificador del proceso enviado por Kernell a Memoria
t_identificadorDePseudocodigo deserializarIdentificador(void* stream)
{
	t_identificadorDePseudocodigo identificador;
	
	memcpy(&identificador.PID, stream, sizeof(uint32_t));
	memcpy(&identificador.largoCadena, stream + sizeof(uint32_t), sizeof(uint8_t));
	memcpy(&identificador.path, stream + sizeof(uint32_t) + sizeof(uint8_t), identificador.largoCadena);

	return identificador;
}

//Descerializa el peticion de instruccion
t_peticionInstruccion deserializarPeticionInstruccion(void* stream)
{
	t_peticionInstruccion peticion;
	memcpy(&peticion.PC,stream,sizeof(uint32_t));
	memcpy(&peticion.PID,stream+sizeof(uint32_t),sizeof(uint32_t));
	return peticion;
}