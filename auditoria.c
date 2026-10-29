#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auditoria.h"
#include "lectura.h"
#include "cadena.h"
#include "util.h"
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    agrega al final del archivo de auditoria una linea con el
                formato DNI;TIPO_VALIDACION;FECHA_PROCESO.
Parametros:     path: ruta del archivo de texto de errores.
                dni_texto: DNI tal como vino en la entrada.
                tipo_error: nombre de la validacion que fallo.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  El archivo se abre en modo append; si no existe se crea.
******************************************************************************/
int auditoria_registrar_ALU(const char *path, const char *dni_texto,
                        const char *tipo_error, const t_fecha *fecha_proceso)
{
    FILE *archivo;
    int resultado;

    if (!path || !dni_texto || !tipo_error || !fecha_proceso)
        return 0;

    archivo = fopen(path, "at");
    if (!archivo)
        return 0;

    resultado = fprintf(archivo, "%s;%s;%02d/%02d/%04d\n", dni_texto, tipo_error,
                         fecha_proceso->dia, fecha_proceso->mes, fecha_proceso->anio) >= 0;
    fclose(archivo);
    return resultado;
}

/******************************************************************************
Descripcion:    lee el archivo de auditoria y muestra, por cada par (DNI, tipo
                de validacion), la cantidad de incidencias registradas y el
                total general.
Parametros:     path: ruta del archivo de texto de errores.
Retorno:        OK si se pudo procesar (aunque el archivo no exista), ERROR si
                fallo la memoria.
Observaciones:  Acumula en un vector dinamico que crece con realloc.
******************************************************************************/
int auditoria_mostrar_ALU(const char *path)
{
    FILE *archivo;
    char linea[TAM_LINEA], clave[TAM_LINEA], *campos[4];
    char *claves = NULL;
    unsigned long *cantidades = NULL, total = 0;
    size_t cantidad = 0, capacidad = 0;
    int resultado = 1;
    int continuar = 1;

    if (!path)
        return 0;

    archivo = fopen(path, "rt");
    if (!archivo)
    {
        printf("No hay auditoria disponible. Total de incidencias: 0\n");
        return 1;
    }

    while (continuar && fgets(linea, sizeof(linea), archivo))
    {
        char *fin = linea;
        size_t posicion = 0;
        int c;

        while (*fin && *fin != '\n')
            fin++;

        if (!*fin && !feof(archivo))
        {
            c = fgetc(archivo);
            while (c != '\n' && c != EOF)
                c = fgetc(archivo);
            resultado = 0;
            continuar = 0;
        }

        *fin = '\0';

        if (continuar && separar_campos_ALU(linea, campos, 4) == 3)
        {
            if (strlen(*campos) + strlen(*(campos + 1)) + 2 < sizeof(clave))
            {
                strcpy(clave, *campos);
                strcat(clave, ";");
                strcat(clave, *(campos + 1));
            }
            else
            {
                resultado = 0;
                continuar = 0;
            }

            while (continuar && posicion < cantidad &&
                   strcmp(claves + posicion * TAM_LINEA, clave))
            {
                posicion++;
            }

            if (continuar && posicion == cantidad)
            {
                if (cantidad == capacidad)
                {
                    size_t nueva = capacidad ?
                        capacidad + capacidad / 2 + 1 : CANTIDAD_ELEMENTOS;
                    char *nuevas_claves = NULL;
                    unsigned long *nuevas_cantidades = NULL;

                    if (nueva <= capacidad ||
                        nueva > (size_t)-1 / TAM_LINEA ||
                        nueva > (size_t)-1 / sizeof(*cantidades))
                    {
                        resultado = 0;
                        continuar = 0;
                    }
                    else
                    {
                        nuevas_claves = realloc(claves, nueva * TAM_LINEA);
                        if (!nuevas_claves)
                        {
                            resultado = 0;
                            continuar = 0;
                        }
                        else
                        {
                            claves = nuevas_claves;
                            nuevas_cantidades = realloc(cantidades,
                                                        nueva * sizeof(*cantidades));
                            if (!nuevas_cantidades)
                            {
                                resultado = 0;
                                continuar = 0;
                            }
                            else
                            {
                                cantidades = nuevas_cantidades;
                                capacidad = nueva;
                            }
                        }
                    }
                }

                if (continuar)
                {
                    strcpy(claves + cantidad * TAM_LINEA, clave);
                    *(cantidades + cantidad) = 0;
                    cantidad++;
                }
            }

            if (continuar)
            {
                (*(cantidades + posicion))++;
                total++;
            }
        }
    }

    fclose(archivo);

    if (resultado)
    {
        size_t i;
        printf("DNI;TIPO_VALIDACION;INCIDENCIAS\n");
        for (i = 0; i < cantidad; i++)
            printf("%s;%lu\n", claves + i * TAM_LINEA, *(cantidades + i));
        printf("Total de incidencias: %lu\n", total);
    }
    else
        printf("No se pudo completar la lectura de auditoria.\n");

    free(claves);
    free(cantidades);

    return resultado;
}
