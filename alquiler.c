#include <stdlib.h>
#include <string.h>
#include "alquiler.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    imprime por pantalla la matriz de consumo con totales por fila
                (plan) y por columna (genero).
Parametros:     m: matriz de consumo.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void tablero_mostrar_ALU(t_matriz_consumo m)
{
    long total_general;
    int i, j;
    total_general = 0;

    if (!m)
        return;

    printf("%-10s", "Plan");
    for (j = 0; j < CANT_GENEROS; j++)
        printf("%18s", *(GENEROS + j));
    printf("%18s\n", "Total");
    for (i = 0; i < CANT_PLANES; i++)
    {
        long total_fila;
        total_fila = 0;
        printf("%-10s", *(PLANES + i));
        for (j = 0; j < CANT_GENEROS; j++)
        {
            printf("%18d", m[i][j]);
            total_fila += m[i][j];
        }
        total_general += total_fila;
        printf("%18ld\n", total_fila);
    }
    printf("%-10s", "Total");
    for (j = 0; j < CANT_GENEROS; j++)
    {
        long total_columna;
        total_columna = 0;
        for (i = 0; i < CANT_PLANES; i++)
            total_columna += m[i][j];
        printf("%18ld", total_columna);
    }
    printf("%18ld\n", total_general);
}

/******************************************************************************
Descripcion:    ejecuta el flujo de alquiler de un titulo: 1) habilitacion del
                socio (debe estar en el indice), 2) inventario (el titulo debe
                estar en el indice y con stock > 0), 3) descuenta una unidad
                de stock en el archivo, 4) incrementa la celda [plan][genero]
                de la matriz, 5) emite el comprobante.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                path_socios: ruta de socios.dat.
                path_titulos: ruta de titulos.dat.
                matriz: matriz de consumo a alimentar.
                fecha_proceso: fecha de proceso para el comprobante.
Retorno:        OK si el alquiler se concreto, ERROR si alguna etapa fallo.
Observaciones:  Si una etapa falla se informa el motivo y no se ejecutan las
                siguientes. No se exige cuota al dia.
******************************************************************************/
int alquilar_ALU(t_indice *ind_socios, t_indice *ind_titulos,
                 const char *path_socios, const char *path_titulos,
                 t_matriz_consumo matriz_consumo,
                 const t_fecha *fecha_proceso)
{
    char texto[TAM_LINEA];
    const char *p;
    t_reg_indice socio_indice, titulo_indice;
    t_socio socio;
    t_titulo titulo, original;
    FILE *archivo;
    int leido, resultado;
    int fila, columna;

    if(!ind_socios || !ind_titulos || !path_socios || !path_titulos ||
       !matriz_consumo || !fecha_proceso)
        return ERROR;

    printf("DNI del socio: ");
    leer_linea(texto, sizeof(texto));

    p = texto;
    while(*p >= '0' && *p <= '9')
        p++;

    socio_indice.id = strtol(texto, NULL, 10);

    if(p == texto || *p || socio_indice.id <= DNI_MIN ||
       socio_indice.id >= DNI_MAX)
    {
        printf("Socio inexistente o inactivo.\n");
        return ERROR;
    }

    if(indice_buscar_ALU(ind_socios, &socio_indice,
                         sizeof(socio_indice),
                         compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("Socio inexistente o inactivo.\n");
        return ERROR;
    }

    archivo = fopen(path_socios, "rb");

    if(!archivo)
    {
        printf("No se pudo abrir el archivo de socios.\n");
        return ERROR;
    }

    leido = socio_leer_ALU(archivo, socio_indice.nro_reg, &socio);
    fclose(archivo);

    if(!leido || socio.estado != 'A')
    {
        printf("No se pudo habilitar al socio.\n");
        return ERROR;
    }

    printf("ID del titulo: ");
    leer_linea(texto, sizeof(texto));

    p = texto;
    while(*p >= '0' && *p <= '9')
        p++;

    titulo_indice.id = strtol(texto, NULL, 10);

    if(p == texto || *p || titulo_indice.id <= 0)
    {
        printf("Titulo inexistente o inactivo.\n");
        return ERROR;
    }

    if(indice_buscar_ALU(ind_titulos, &titulo_indice,
                         sizeof(titulo_indice),
                         compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("Titulo inexistente o inactivo.\n");
        return ERROR;
    }

    archivo = fopen(path_titulos, "r+b");

    if(!archivo)
    {
        printf("No se pudo abrir el archivo de titulos.\n");
        return ERROR;
    }

    resultado = ERROR;

    if(titulo_leer(archivo, titulo_indice.nro_reg, &titulo))
    {
        if(titulo.estado == 'A' && titulo.stock > 0)
        {
            fila = plan_indice(socio.plan);
            columna = genero_indice(titulo.genero);

            if(fila >= 0 && fila < CANT_PLANES &&
               columna >= 0 && columna < CANT_GENEROS)
            {
                original = titulo;
                titulo.stock--;

                if(titulo_escribir(archivo, titulo_indice.nro_reg,
                                   &titulo))
                    resultado = OK;
                else
                {
                    printf("No se pudo actualizar el stock.\n");
                    clearerr(archivo);

                    if(!titulo_escribir(archivo, titulo_indice.nro_reg,
                                        &original))
                        printf("No se pudo restaurar el stock original.\n");
                }
            }
            else
                printf("No se puede registrar el consumo.\n");
        }
        else
            printf("Titulo no disponible o sin stock.\n");
    }
    else
        printf("No se pudo leer el titulo.\n");

    fclose(archivo);

    if(!resultado)
        return ERROR;

    matriz_consumo[fila][columna]++;

    printf("Alquiler confirmado - %02d/%02d/%04d\n"
           "Socio: %ld - %s\n"
           "Titulo: %d - %s\n",
           fecha_proceso->dia, fecha_proceso->mes, fecha_proceso->anio,
           socio.dni, socio.apellidos_nombres, titulo.id, titulo.titulo);

    return OK;
}
