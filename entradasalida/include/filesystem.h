#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

//------------BIBLIOTECAS------------

#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <../include/inicioYFin.h>
#include <../include/estructuras.h>
#include <../../utils/include/utils.h>
#include <../../utils/include/comunicacion.h>

//------------ DECLARACION DE FUNCIONES ------------

int buscarPrimerBloque(void);
int compactarPrimerosArchivos(char*, char*, int);
bool esIgualBloque(void*);
bool verificarDisponibilidadBloques(int, int);
void setearNBits(int, int);
void iniciarBloques(t_config *);
void ocuparBloques(t_list*, t_bitarray*);
void iniciarArchivos(char*, char*, t_log*);
void iniciarBitmap(char*, uint32_t, t_log*);
void compactarBitMap(char*, char*, int, int, t_log*);
void crearArchivoDeBloques(char*, uint32_t , uint32_t, t_log*);
t_list* bloquesLibres(int, t_bitarray*);
t_metadata *crearMetadata(int);
t_valores_bloques *crearBloque(char *);
t_valores_bloques cargarValores(t_config* );

//----------- PETICIONES ---------------

void crearArchivo(char*, char*, t_log* );
void actualizarMetadata(char*, t_metadata*);
void borrarArchivo(char*, char*, t_log*, int);
void cargarTamanioMetadata(t_metadata*, uint32_t);
void truncarArchivo(char*, char*, int, uint32_t, int, t_log*);
void* leerArchivo(char*, char*, uint32_t, uint32_t, int, t_log*);
void escribirArchivo(char*, char*, uint32_t, uint32_t, void*, int, t_log*);

#endif