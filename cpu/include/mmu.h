#ifndef CPU_MMU_H_
#define CPU_MMU_H_

//------------BIBLIOTECAS------------

#include <../include/tlb.h>
#include <../../utils/include/utils.h>

//------------DECLARACION DE FUNCIONES------------

int traducirDirecLogica(int, int, void*, int,  int, char*, int, t_log*);

#endif