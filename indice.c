#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    toma memoria para nmemb elementos e inicializa la estructura
                vacia.
Parametros:     indice: TDA indice.
                nmemb: cantidad inicial de elementos (CANTIDAD_ELEMENTOS).
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int indice_crear_ALU(t_indice *indice, size_t nmemb, size_t tamanyo)
{
    void *memoria;

    if (!indice)
        return 0;

    if (nmemb == 0 || tamanyo == 0)
        return 0;

    memoria = malloc(nmemb * tamanyo);

    if (!memoria)
        return 0;

    indice->vindice = memoria;
    indice->cantidad_elementos_actual = 0;
    indice->cantidad_elementos_maxima = (unsigned)nmemb;

    return 1;
}

/******************************************************************************
Descripcion:    redimensiona el arreglo interno del indice con realloc. La
                nueva capacidad es la actual multiplicada por INCREMENTO (30%
                mas); la calcula la propia funcion.
Parametros:     indice: TDA indice.
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Si el producto no aumenta la capacidad (arreglos muy chicos)
                se garantiza al menos un elemento mas.
******************************************************************************/
int indice_redimensionar_ALU(t_indice *indice, size_t tamanyo)
{
    void *nuevo;
    size_t capacidad;

    if (!indice || !tamanyo)
        return 0;

    capacidad = (size_t)(indice->cantidad_elementos_maxima * INCREMENTO);

    if (capacidad <= indice->cantidad_elementos_maxima)
    capacidad = (size_t)indice->cantidad_elementos_maxima + 1;

    nuevo = realloc(indice->vindice, capacidad * tamanyo);

    if (!nuevo)
        return 0;

    indice->vindice = nuevo;
    indice->cantidad_elementos_maxima = (unsigned)capacidad;

    return 1;
}

/******************************************************************************
Descripcion:    inserta un elemento manteniendo el orden segun la clave. Si el
                arreglo esta lleno invoca indice_redimensionar.
Parametros:     indice: TDA indice.
                registro: elemento a insertar.
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario
                (clave duplicada o falta de memoria).
Observaciones:  -
******************************************************************************/
int indice_insertar_ALU(t_indice *indice, const void *registro, size_t tamanyo,int (*cmp)(const void *, const void *)){
    size_t izquierda = 0, derecha, posicion, medio;
    int comparacion;
    char *base;
    void *copia;

    if (!indice || !registro || !cmp || !tamanyo ||
            indice->cantidad_elementos_actual > indice->cantidad_elementos_maxima ||
            (indice->cantidad_elementos_actual && !indice->vindice))
        return 0;

    derecha = indice->cantidad_elementos_actual;
    base = (char *)indice->vindice;

    while (izquierda < derecha)
    {
        medio = izquierda + (derecha - izquierda) / 2;
        comparacion = cmp(registro, base + medio * tamanyo);
        if (comparacion == 0)
            return 0;
        if (comparacion < 0)
            derecha = medio;
        else
            izquierda = medio + 1;
    }
    posicion = izquierda;
    /* La copia protege incluso un registro que apunte dentro del arreglo. */
    copia = malloc(tamanyo);
    if (!copia)
        return 0;

    memcpy(copia, registro, tamanyo);
    if (indice_lleno_ALU(indice) && !indice_redimensionar_ALU(indice, tamanyo))
    {
        free(copia);
        return 0;
    }

    base = (char *)indice->vindice;

    memmove(base + (posicion + 1) * tamanyo, base + posicion * tamanyo,
            (indice->cantidad_elementos_actual - posicion) * tamanyo);
    memcpy(base + posicion * tamanyo, copia, tamanyo);
    indice->cantidad_elementos_actual++;

    free(copia);

    return 1;
}

/******************************************************************************
Descripcion:    elimina el elemento cuya clave coincide con la de registro,
                desplazando los siguientes.
Parametros:     indice: TDA indice.
                registro: elemento a eliminar (solo importa su clave).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR si la clave no existe.
Observaciones:  -
******************************************************************************/
int indice_eliminar_ALU(t_indice *indice, const void *registro, size_t tamanyo,int (*cmp)(const void *, const void *)){
    size_t izquierda = 0, derecha, medio;
    int comparacion;
    char *base;

    if (!indice || !registro || !tamanyo || !cmp || !indice->vindice)
        return 0;

    base = (char *)indice->vindice;
    derecha = indice->cantidad_elementos_actual;

    while (izquierda < derecha)
    {
        medio = izquierda + (derecha - izquierda) / 2;
        comparacion = cmp(registro, base + medio * tamanyo);
        if (comparacion == 0)
        {
            memmove(base + medio * tamanyo, base + (medio + 1) * tamanyo,(indice->cantidad_elementos_actual - medio - 1) * tamanyo);
            indice->cantidad_elementos_actual--;
            return 1;
        }
        if (comparacion < 0)
            derecha = medio;
        else
            izquierda = medio + 1;
    }
    return 0;
}

/******************************************************************************
Descripcion:    busca la clave de registro en el indice. Si existe copia el
                elemento completo (nro_reg incluido) en registro.
Parametros:     indice: TDA indice.
                registro: elemento a buscar (entrada/salida).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        NO_EXISTE si la clave no existe; en caso contrario la posicion
                que ocupa dentro del arreglo.
Observaciones:  La comparacion entre claves se realiza exclusivamente mediante
                cmp.
******************************************************************************/
int indice_buscar_ALU(const t_indice *indice, void *registro, size_t tamanyo,int (*cmp)(const void *, const void *)){
    size_t izquierda = 0, derecha, medio;
    int comparacion;
    const char *base;

    if (!indice || !registro || !cmp || !tamanyo || !indice->vindice)
        return NO_EXISTE;

    base = (const char *)indice->vindice;
    derecha = indice->cantidad_elementos_actual;

    while (izquierda < derecha)
    {
        medio = izquierda + (derecha - izquierda) / 2;
        comparacion = cmp(registro, base + medio * tamanyo);
        if (comparacion == 0)
        {
            memmove(registro, base + medio * tamanyo, tamanyo);
            return (int)medio;
        }
        if (comparacion < 0)
            derecha = medio;
        else
            izquierda = medio + 1;
    }
    return NO_EXISTE;
}

/******************************************************************************
Descripcion:    determina si el indice contiene 0 (cero) elementos.
Parametros:     indice: TDA indice.
Retorno:        OK si esta vacio, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_vacio_ALU(const t_indice *indice)
{
    return !indice || indice->cantidad_elementos_actual == 0;
}

/******************************************************************************
Descripcion:    determina si el indice alcanzo su capacidad maxima actual.
Parametros:     indice: TDA indice.
Retorno:        OK si esta lleno, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_lleno_ALU(const t_indice *indice)
{
    return indice && indice->cantidad_elementos_actual >= indice->cantidad_elementos_maxima;
}

/******************************************************************************
Descripcion:    deja el indice vacio sin liberar la memoria del arreglo.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void indice_vaciar_ALU(t_indice *indice)
{
    if (indice)
        indice->cantidad_elementos_actual = 0;
}

/******************************************************************************
Descripcion:    libera toda la memoria dinamica del indice.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  Luego de invocarla el indice no puede usarse sin volver a
                llamar a indice_crear.
******************************************************************************/
void indice_destruir_ALU(t_indice *indice)
{
    if (indice)
    {
        free(indice->vindice);
        indice->vindice = NULL;
        indice->cantidad_elementos_actual = 0;
        indice->cantidad_elementos_maxima = 0;
    }
}

/******************************************************************************
Descripcion:    construye el indice a partir de un archivo binario de
                registros. Recorre el archivo secuencialmente y por cada
                registro activo inserta un t_reg_indice (nro_reg = posicion en
                el archivo, id = clave obtenida con obtener_clave).
Parametros:     path: ruta al archivo binario.
                indice: TDA indice (ya creado; se vacia antes de cargar).
                tamanyo_reg: tamanyo en bytes de cada registro del archivo.
                obtener_clave: funcion que devuelve la clave (long) de un
                registro.
                es_activo: funcion que devuelve distinto de 0 si el registro
                tiene estado 'A'.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Los registros con estado 'B' se omiten. El archivo no necesita
                estar ordenado: el orden lo garantiza indice_insertar.
******************************************************************************/
int indice_cargar_ALU(const char *path, t_indice *indice, size_t tamanyo_reg,
                  long (*obtener_clave)(const void *),
                  int (*es_activo)(const void *),
                  int (*cmp)(const void *, const void *))
{
    FILE *archivo;
    void *registro;
    t_indice nuevo;
    t_reg_indice elemento;
    unsigned numero = 0;
    int resultado = 1;
    size_t leidos = 0;

    if (!path || !indice || !tamanyo_reg || !obtener_clave || !es_activo || !cmp)
        return 0;

    indice_vaciar_ALU(indice);

    archivo = fopen(path, "rb");

    if (!archivo)
        return 0;

    registro = malloc(tamanyo_reg);
    if (!registro || !indice_crear_ALU(&nuevo, CANTIDAD_ELEMENTOS, sizeof(elemento)))
    {
        free(registro);
        fclose(archivo);
        return 0;
    }

    while ((leidos = fread(registro, 1, tamanyo_reg, archivo)) == tamanyo_reg)
    {
        if (es_activo(registro))
        {
            elemento.id = obtener_clave(registro);
            elemento.nro_reg = numero;
            if (!indice_insertar_ALU(&nuevo, &elemento, sizeof(elemento), cmp))
            {
                resultado = 0;
                break;
            }
        }
        numero++;
    }
    if (leidos != 0 && leidos != tamanyo_reg)
        resultado = 0;
    fclose(archivo);

    free(registro);

    if (resultado)
    {
        indice_destruir_ALU(indice);
        *indice = nuevo;
    }
    else
        indice_destruir_ALU(&nuevo);
    return resultado;
}
