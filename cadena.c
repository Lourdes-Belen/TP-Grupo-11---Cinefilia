#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "cadena.h"



/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/
/******************************************************************************
Descripcion:    normaliza un campo Apellidos y Nombres segun el Anexo A.2 (con
                regla de coma).
Parametros:     s: cadena a normalizar (se modifica in place).
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void arreglar_apyn_ALU(char *s)
{
    char normalizado[TAM_NORMALIZADO];
    char *salida = normalizado;
    const char *entrada;
    size_t posicion_coma = 0;
    int separador_pendiente = 0;
    int inicio_palabra = 1;
    int cantidad_palabras = 0;
    int coma_encontrada = 0;

    if (!s)
        return;

    entrada = s;
    while (*entrada && salida < normalizado + sizeof(normalizado) - 2)
    {
        unsigned char c = (unsigned char)*entrada++;

        if (c == ',')
        {
            if (cantidad_palabras > 0 && !coma_encontrada)
            {
                posicion_coma = (size_t)(salida - normalizado);
                coma_encontrada = 1;
                separador_pendiente = 1;
            }
            continue;
        }

        if (isspace(c))
        {
            if (cantidad_palabras > 0)
                separador_pendiente = 1;
            continue;
        }

        if (separador_pendiente)
        {
            if (salida >= normalizado + sizeof(normalizado) - 3)
                break;
            if (cantidad_palabras == 1 && !coma_encontrada)
                posicion_coma = (size_t)(salida - normalizado);
            *salida++ = ' ';
            separador_pendiente = 0;
            inicio_palabra = 1;
        }

        if (inicio_palabra)
        {
            *salida++ = (char)toupper(c);
            inicio_palabra = 0;
            cantidad_palabras++;
        }
        else
            *salida++ = (char)tolower(c);
    }

    *salida = '\0';

    if (cantidad_palabras > 0)
    {
        if (posicion_coma == 0)
            posicion_coma = (size_t)(salida - normalizado);

        memmove(normalizado + posicion_coma + 1,
                normalizado + posicion_coma,
                (size_t)(salida - normalizado) - posicion_coma + 1);
        normalizado[posicion_coma] = ',';
        salida++;
    }

    memcpy(s, normalizado, (size_t)(salida - normalizado) + 1);
}

/******************************************************************************
Descripcion:    separa una linea CSV in place por SEPARADOR_CSV, reemplazando
                cada separador por '\0' y guardando en campos[] el puntero al
                inicio de cada campo.
Parametros:     linea: linea a separar (se modifica).
                campos: vector donde se guardan los punteros a cada campo.
                max: cantidad maxima de campos a separar.
Retorno:        Cantidad de campos encontrados (los vacios cuentan). Nunca mas
                de max.
Observaciones:  Si la linea tiene mas separadores que max, el ultimo campo
                conserva el resto de la linea.
******************************************************************************/
int separar_campos_ALU(char *linea, char *campos[], int max)
{
    int cantidad = 1;
    char *p = linea;

    if (!linea || !campos || max <= 0)
        return 0;
    *campos = linea;

    while (*p && cantidad < max)
    {
        if (*p == SEPARADOR_CSV)
        {
            *p = '\0';
            *(campos + cantidad++) = p + 1;
        }
        p++;
    }

    return cantidad;
}
