#include <string.h>
#include "util.h"
#include "cadena.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    opcion Salir: copia socios.dat y titulos.dat con la fecha de
                proceso en el nombre, vuelca la matriz de consumo a disco y
                libera la memoria de ambos indices.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                matriz: matriz de consumo.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void salir_y_persistir_ALU(t_indice *ind_socios, t_indice *ind_titulos,
                           t_matriz_consumo matriz_consumo,
                           const t_fecha *fecha_proceso)
{
    char fecha[9], path[TAM_LINEA];

    if(fecha_proceso && fecha_correcta(fecha_proceso))
    {
        fecha_a_texto_compacto(fecha_proceso, fecha);

        strcpy(path, PREFIJO_SOCIOS_BKP);
        strcat(path, fecha);
        strcat(path, ".dat");

        if(archivo_existe(ARCH_SOCIOS_BIN))
        {
            if(!archivo_copiar(ARCH_SOCIOS_BIN, path))
                printf("No se pudo respaldar socios.\n");
            else
                printf("Respaldo: %s\n", path);
        }

        strcpy(path, PREFIJO_TITULOS_BKP);
        strcat(path, fecha);
        strcat(path, ".dat");

        if(archivo_existe(ARCH_TITULOS_BIN))
        {
            if(!archivo_copiar(ARCH_TITULOS_BIN, path))
                printf("No se pudo respaldar titulos.\n");
            else
                printf("Respaldo: %s\n", path);
        }

        strcpy(path, PREFIJO_MATRIZ);
        strcat(path, fecha);
        strcat(path, ".dat");

        if(!matriz_consumo || !matriz_consumo_persistir(matriz_consumo, path))
            printf("No se pudo persistir la matriz.\n");
        else
            printf("Matriz de consumo: %s\n", path);
    }
    else
        printf("Fecha invalida: no se pudo persistir.\n");

    indice_destruir_ALU(ind_socios);
    indice_destruir_ALU(ind_titulos);

    printf("Memoria liberada.\n");
}
