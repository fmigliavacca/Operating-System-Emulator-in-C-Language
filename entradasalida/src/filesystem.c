#include <../include/filesystem.h>

t_bitmap * bitmap;
t_metadata *valores_metadata;

FILE* fBloques;

//--------- BLOQUES.dat-------

void mostrarBitmap(t_bitarray* bitarray, t_log* logger)
{
    bool estadoBloque;
    
    //muestro por pantalla el bitarray
    for(int i = 0; i < bitarray_get_max_bit(bitarray); i++)
    {
        estadoBloque = bitarray_test_bit(bitarray, i);
        if (estadoBloque)
        {
            log_info(logger, "bloque %d: 1", i);
        }
        else
        {
            log_info(logger, "bloque %d: 0", i);
        }
    }
}

t_valores_bloques cargarValores(t_config* config)
{
    t_valores_bloques valoresBloque;

	valoresBloque.blockSize = config_get_int_value(config, "BLOCK_SIZE");
	valoresBloque.blockCount = config_get_int_value(config, "BLOCK_COUNT");
    valoresBloque.retrasoCompactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    return valoresBloque;
}

void crearArchivoDeBloques(char* pathDialfs, uint32_t blockCount, uint32_t blockSize, t_log* logger)
{
    //Reservo el espacio y creo el path completo donde se encontrara mi nuevo archivo
    char* pathBloque = malloc(strlen("bloques.dat") + strlen(pathDialfs) + 1);

    strcpy(pathBloque, pathDialfs);
    strcat(pathBloque, "bloques.dat");

    FILE* nArchivo = fopen(pathBloque, "ab+");

    uint32_t tamanioArchivo = blockCount * blockSize;

    truncate(pathBloque, tamanioArchivo);
    
    fclose(nArchivo);

    free(pathBloque);
}

void iniciarBitmap(char* pathDialfs, uint32_t blockCount,t_log* logger)
{
    //Reservo el espacio y creo el path completo donde se encontrara mi nuevo archivo
    char* pathBitmap = malloc(strlen("bitmap.dat") + strlen(pathDialfs) + 1);

    strcpy(pathBitmap, pathDialfs);
    strcat(pathBitmap, "bitmap.dat");
    

    bitmap = malloc(sizeof(t_bitmap));

    int fdArchivo = open(pathBitmap, O_RDWR | O_CREAT, 0644);

    //Asignamos tamaño al bitmap
    bitmap->tamanio = (uint32_t)ceil(blockCount / 8);

    if (ftruncate(fdArchivo, bitmap->tamanio) == -1)
    {
        log_info(logger,"Error truncando el archivo bitmap");
    }

    bitmap->posicion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE , MAP_SHARED, fdArchivo, 0); 
    bitmap->bitarray = bitarray_create_with_mode(bitmap->posicion, bitmap->tamanio, LSB_FIRST);

    if (!bitmap->bitarray)
    {
        log_error(logger, "Error creando el bitarray");
        munmap(bitmap->posicion, bitmap->tamanio);
        close(fdArchivo);
        free(bitmap);
    }

    if(msync(bitmap->posicion, bitmap->tamanio, MS_SYNC) == -1)
    {
        log_error(logger, "Error sincronizando el bitmap");
    }

    close(fdArchivo);

    free(pathBitmap);
}

//Funcion para devolver los bloques libres
t_list* bloquesLibres(int cantBloques, t_bitarray* bloque)
{
    t_list* bloquesLibres = list_create();
    int i = 0;
    int j = 0;

    //Mientras j sea menor a la cantidad de bloques pedida e i sea menor a la cantidad maxima de bloques
    while((j < cantBloques) && (i < bitarray_get_max_bit(bloque)))
    {
        //Me fijo si el bloque esta ocupado
        if(!bitarray_test_bit(bloque, i))
        {	
            //Si no lo esta entonces almaceno su index en la lista de bloquesLibres
			int* aux = malloc(sizeof(int));
            *aux = i;

            list_add(bloquesLibres, aux);

            j++;
        }
        i++;
    }

    if (j < cantBloques) {
        list_destroy_and_destroy_elements(bloquesLibres,free);
        return NULL;
    }

    return bloquesLibres;
}

int buscarPrimerBloque()
{
    int i=0;
    bool bloqueUsado;

    //RECORRO EL BITARRAY Y SI ES 0 ENTONCES LO DEVUELVE 
    for(i=0; i < bitarray_get_max_bit(bitmap->bitarray); i++)
    {
        bloqueUsado = bitarray_test_bit(bitmap->bitarray, i);
        if(!bloqueUsado){
            return i;
            break;
        }
    }
    return -1;
}

void ocuparBloques(t_list* ocuparBloques , t_bitarray* bloque)
{
    int i = 0;
	int* n;

    //Pongo en 1 todos los bits de los bloques que ocupe
    while(i < (list_size(ocuparBloques)))
    {
    	n = list_get(ocuparBloques, i);
        bitarray_set_bit(bloque, *n);
        i++;
    }
}

void liberarBloques(int bloqueInicial, int cantidadDeBloques)
{
    if(cantidadDeBloques == 0)
    {
        bitarray_clean_bit(bitmap->bitarray, bloqueInicial);
    }

    else
    {
        int bloqueFinal = bloqueInicial + cantidadDeBloques;

        //Limpio el bitarray
        for(int i = bloqueInicial; i < bloqueFinal; i++)
        {
            bitarray_clean_bit(bitmap->bitarray, i);
        }
    }
}

t_metadata* crearMetadata(int bloqueInicial)
{
    t_metadata* nuevoArchivo = malloc(sizeof(t_metadata));

    nuevoArchivo->bloqueInicial = bloqueInicial;
    nuevoArchivo->tamanioArchivo = 0;

    return nuevoArchivo;
}

void cargarTamanioMetadata(t_metadata* valores, uint32_t nuevoTamanio)
{
    valores->tamanioArchivo = nuevoTamanio;
    return;
}

//Funcion para crear un nuevo archivo en el FS
void crearArchivo(char* pathDialfs, char* nombreArchivoACrear, t_log* logger)
{   
    //Reservo el espacio y creo el path completo donde se encontrara mi nuevo archivo
    char* pathNuevoArchivo = malloc(strlen(nombreArchivoACrear) + strlen(pathDialfs) + 1);

    strcpy(pathNuevoArchivo, pathDialfs);
    strcat(pathNuevoArchivo, nombreArchivoACrear);

    //Busco un bloque libre en bitarray
    t_list* listaBloquesLibres = bloquesLibres(1, bitmap->bitarray);

    //Obtengo el primer bloque
    int* bloqueInicial = list_get(listaBloquesLibres, 0);

    //Creo el archivo en cuestion
    FILE* archivo = fopen(pathNuevoArchivo, "w");

    //Escribo en archivo la info del metadata
    fprintf(archivo, "BLOQUE_INICIAL = %d\n", *bloqueInicial);
    fprintf(archivo, "TAMANIO_ARCHIVO = %d", 0);

    ocuparBloques(listaBloquesLibres, bitmap->bitarray);

    log_info(logger, "Archivo \"%s\" creado", nombreArchivoACrear);

    fclose(archivo);
    free(pathNuevoArchivo);
    free(nombreArchivoACrear);
    list_destroy_and_destroy_elements(listaBloquesLibres,free);
}

//------- BORRAR ARCHIVO ----------

t_metadata extraerDatos(char* path)
{
    t_config* datos = config_create(path);
    t_metadata metadata;

    metadata.bloqueInicial = config_get_int_value(datos, "BLOQUE_INICIAL");
    metadata.tamanioArchivo = config_get_int_value(datos, "TAMANIO_ARCHIVO");

    config_destroy(datos);

    return metadata;
}

void borrarArchivo(char* pathDialfs, char* nombreArchivoABorrar, t_log* logger, int tamanioBloque)
{
    //RECIBIR PATH

    char* archivoABorrar = malloc(strlen(nombreArchivoABorrar) + strlen(pathDialfs) + 1);

    strcpy(archivoABorrar, pathDialfs);
    strcat(archivoABorrar, nombreArchivoABorrar);

    t_metadata metadata = extraerDatos(archivoABorrar);

    if(remove(archivoABorrar) == 0)
    {
        log_info(logger, "Archivo \"%s\" borrado", nombreArchivoABorrar);

        int bloques = ceil((double) metadata.tamanioArchivo / (double)tamanioBloque);

        liberarBloques(metadata.bloqueInicial, bloques);
    }

    else
    {
        log_info(logger, "El archivo que se intenta borrar no existe"); 
    }

    free(archivoABorrar);
} 

//------- TRUNCAR ARCHIVO ----------

void truncarArchivo(char* pathDialfs, char* nombreArchivoATruncar, int blockSize, uint32_t nuevoTamanio, int retrasoCompactacion, t_log* logger)
{
    //RECIBIR PATH
    t_metadata* valoresMetadata = malloc(sizeof(t_metadata));

    char* pathArchivo = malloc(strlen(nombreArchivoATruncar) + strlen(pathDialfs) + 1);
    
    strcpy(pathArchivo, pathDialfs);
    strcat(pathArchivo, nombreArchivoATruncar);

    t_config *configMetadata = config_create(pathArchivo);

    int bloqueInicial = config_get_int_value(configMetadata, "BLOQUE_INICIAL");
    int tamanioActual = config_get_int_value(configMetadata, "TAMANIO_ARCHIVO");   


    int bloquesAModificar;
    int tamanioBloque = blockSize;
    int nuevosBloques = ceil((double)nuevoTamanio / (double)tamanioBloque);
    int bloquesActuales = ceil((double) tamanioActual / (double)tamanioBloque);

    //Los bloques actuales nunca pueden ser 0
    if(bloquesActuales == 0)
    {
        bloquesActuales = 1;
    }

    valoresMetadata->bloqueInicial = bloqueInicial;
    valoresMetadata->tamanioArchivo = nuevoTamanio;

    actualizarMetadata(pathArchivo,valoresMetadata);

    if(nuevosBloques > bloquesActuales)
    {
        bloquesAModificar = nuevosBloques - bloquesActuales;

        if(verificarDisponibilidadBloques(bloqueInicial + bloquesActuales, bloquesAModificar))
        {
            //Seteo los bits
            setearNBits(bloqueInicial, nuevosBloques);
        }
        else
        { 
            //Compacto el bitmap para poder truncar el archivo
            compactarBitMap(pathDialfs, nombreArchivoATruncar, retrasoCompactacion, blockSize, logger);
        }

    }

    else if(bloquesActuales > nuevosBloques)
    {
        bloquesAModificar = bloquesActuales - nuevosBloques;
        liberarBloques(bloqueInicial + nuevosBloques, bloquesAModificar);
    }

    log_info(logger, "Archivo \"%s\" truncado! %d", nombreArchivoATruncar, nuevoTamanio);

    free(pathArchivo);
    free(valoresMetadata);
    config_destroy(configMetadata);
}

bool verificarDisponibilidadBloques(int bloqueInicial, int bloquesAModificar)
{
    for(int i = 0; i < bloquesAModificar; i++)
    {
        if(bitarray_test_bit(bitmap->bitarray, i + bloqueInicial))
        {
            return false;
        }
    }
    return true;
}

void actualizarMetadata(char* path, t_metadata* archMetadata)
{
    t_config* modificarMetadata = config_create(path);

    char bloqueInicial[12];
    sprintf(bloqueInicial, "%d", archMetadata->bloqueInicial);
    char tamanio[12];
    sprintf(tamanio, "%d", archMetadata->tamanioArchivo);

    config_set_value(modificarMetadata,"TAMANIO_ARCHIVO", tamanio);
    config_set_value(modificarMetadata,"BLOQUE_INICIAL",  bloqueInicial);

    config_save(modificarMetadata);
    config_destroy(modificarMetadata);
}

//--------- ESCRIBIR -----------

void escribirArchivo(char* pathDialfs, char* nombreArchivoAEscribir, uint32_t regPuntero, uint32_t tamanioContenido, void* contenido, int blockSize, t_log* logger)
{
    //RECIBIR PATH
    char* pathArchivo = malloc(strlen(nombreArchivoAEscribir) + strlen(pathDialfs) + 1);
    strcpy(pathArchivo, pathDialfs);
    strcat(pathArchivo, nombreArchivoAEscribir);

    t_config* configArchivo = config_create(pathArchivo);

    int tamanioBloquesFS = blockSize;
    int primerBloque = config_get_int_value(configArchivo, "BLOQUE_INICIAL");
    int tamanioArchivo = config_get_int_value(configArchivo, "TAMANIO_ARCHIVO");

    char* pathBloque = malloc(strlen("bloques.dat") + strlen(pathDialfs) + 1);

    strcpy(pathBloque, pathDialfs);
    strcat(pathBloque, "bloques.dat");


    if(tamanioArchivo > tamanioContenido)
    {
        //Abrimos el archivo en modo lectura y escritura
        FILE* archivoBloques = fopen(pathBloque, "rb+");

        if(archivoBloques != NULL)
        {
            //Nos posicionamos en el desplazamiento del primer bloque
            fseek(archivoBloques, (primerBloque * tamanioBloquesFS) + regPuntero, SEEK_SET);
            fwrite(contenido, 1, tamanioContenido, archivoBloques);
    
            log_info(logger, "Archivo escrito correctamente");

            fclose(archivoBloques);
        }

        else
        {
            log_info(logger, "No se pudo abrir el archivo");
        }
    }

    else
    {
        log_error(logger, "No entra el contenido en el archivo");
    }

    free(pathBloque);
    free(pathArchivo);
    config_destroy(configArchivo);
}

void* leerArchivo(char* pathDialfs, char* nombreArchivoALeer, uint32_t regPuntero, uint32_t tamanioAleer, int blockSize, t_log* logger)
{
    // RECIBIR PATH
    char* pathArchivo = malloc(strlen(nombreArchivoALeer) + strlen(pathDialfs) + 1);
    strcpy(pathArchivo, pathDialfs);
    strcat(pathArchivo, nombreArchivoALeer);

    t_config* configArchivo = config_create(pathArchivo);

    void* contenido = malloc(tamanioAleer);

    int tamanioBloquesFS = blockSize;
    int primerBloque = config_get_int_value(configArchivo, "BLOQUE_INICIAL");

    char* pathBloque = malloc(strlen("bloques.dat") + strlen(pathDialfs) + 1);
    strcpy(pathBloque, pathDialfs);
    strcat(pathBloque, "bloques.dat");

    //Abrimos el archivo en modo lectura y escritura
    FILE* archivoBloques = fopen(pathBloque, "rb");

    if(archivoBloques != NULL)
    {
        //Nos posicionamos en el desplazamiento del primer bloque
        fseek(archivoBloques, (primerBloque * tamanioBloquesFS) + regPuntero, SEEK_SET);
        fread(contenido, 1, tamanioAleer, archivoBloques);

        log_info(logger, "El contenido del archivo es %s", (char*)contenido);

        fclose(archivoBloques);
    }

    else
    {
        log_info(logger, "No se pudo abrir el archivo");
    }

    free(pathArchivo);
    free(pathBloque);
    config_destroy(configArchivo);

    return contenido;
}

//Funcion para compactar el bit map
void compactarBitMap(char* pathDialfs, char* nombreArchivo, int retrasoCompactacion, int blockSize, t_log* logger)
{
    log_info(logger, "Inicio compactacion.");

    int tamanioBloquesFS = blockSize;

    //Obtengo el path del archivo que causo la compactacion
    char* archivoProblematico = malloc(strlen(nombreArchivo) + strlen(pathDialfs) + 1);
    strcpy(archivoProblematico, pathDialfs);
    strcat(archivoProblematico, nombreArchivo);

    //Obtengo la metadata del archivo que la causo, este ira al final, luego libero los bloques de este
    t_metadata metadata = extraerDatos(archivoProblematico);
    liberarBloques(metadata.bloqueInicial, metadata.tamanioArchivo);

    //Compacto todos los archivos menos el que causo la compactacion
    int contadorBloques = compactarPrimerosArchivos(pathDialfs, nombreArchivo, tamanioBloquesFS);

    //Le doy a la metadata del archivo que causo la compactacion sus nuevos valores
    metadata.bloqueInicial = contadorBloques;

    if(metadata.tamanioArchivo == 0)
    {
        contadorBloques += 1;
    }

    else
    {
        contadorBloques += ceil((double)metadata.tamanioArchivo / tamanioBloquesFS);
    }

    //Actualizo la metadata del archivo con los nuevos valores
    actualizarMetadata(archivoProblematico, &metadata);

    //Seteo los bits necesarios
    setearNBits(0, contadorBloques);

    //Hago el sleep de retraso de compactacion
    usleep(retrasoCompactacion * 1000);

    log_info(logger, "Fin compactacion.");

    free(archivoProblematico);
}

int compactarPrimerosArchivos(char* pathDialfs, char* nombreArchivo, int tamanioBloquesFS)
{
    //Creo las estructuras del directorio de dialfs
    DIR* directorioFS = opendir(pathDialfs);
    struct dirent* archivoDirectorio;

    //Declaro las funciones que necesito para modificar
    int contadorBloques = 0;
    t_metadata aux;

    //Leo hasta que no hay mas archivos en el directorio
    while((archivoDirectorio = readdir(directorioFS))!=NULL)
    {
        //Si el directorio es .txt y no es el culpable, lo voy compactando
        if(strstr(archivoDirectorio->d_name, ".txt") != NULL && strcmp(archivoDirectorio->d_name, nombreArchivo) != 0) 
        {
            //Reservo espacio para el path de este archivo .txt
            char* pathArchivo = malloc(strlen(archivoDirectorio->d_name) + strlen(pathDialfs) + 1);
            strcpy(pathArchivo, pathDialfs);
            strcat(pathArchivo, archivoDirectorio->d_name);

            //Extraigo los datos y libero sus bloques
            aux = extraerDatos(pathArchivo);
            liberarBloques(aux.bloqueInicial, aux.tamanioArchivo);

            //Actualizo su metadata e incremento el contador
            aux.bloqueInicial = contadorBloques;
            if(aux.tamanioArchivo == 0)
            {
                contadorBloques += 1;
            }
            else
            {
                contadorBloques += ceil((double)aux.tamanioArchivo / tamanioBloquesFS);
            }
            actualizarMetadata(pathArchivo, &aux);

            free(pathArchivo);
        }
    }

    //Cierro el directorio y retorno el contador
    closedir(directorioFS);

    return contadorBloques;
}

//Funcion que setea desde el bit indicado una cierta cantidad de bits
void setearNBits(int inicio, int cantidadBits)
{
    for(int i = inicio; i<cantidadBits; i++)
    {
        bitarray_set_bit(bitmap->bitarray, i);
    }
}

//-------- Inicio ------------

void iniciarArchivos(char* pathConfig, char* pathDialfs, t_log* logger)
{   
    t_config* config = iniciarConfig(pathConfig);

    t_valores_bloques valores = cargarValores(config);
    
    iniciarBitmap(pathDialfs, valores.blockCount, logger);
    log_info(logger, "Archivo de bitmap creado!");

    crearArchivoDeBloques(pathDialfs, valores.blockCount, valores.blockSize, logger);
    log_info(logger, "Archivo de bloques creado!");

    config_destroy(config);
}