#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fecha.h"
#include "util.h"
#include "indice.h"      /* OK / ERROR */

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    solicita la fecha de proceso por teclado hasta obtener una
                valida. Si el usuario solo presiona Enter toma la fecha actual
                del sistema.
Parametros:     fecha_proceso: fecha destino.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void pedir_fecha_proceso_ALU(t_fecha *fecha_proceso){

    char linea[TAM_LINEA];
    int fecha_valida = 0;

    if (!fecha_proceso)
        return;

    while (!fecha_valida)
    {
        char *fin_linea;
        int formato_valido = 1;
        int entrada_completa = 0;

        printf("Fecha de proceso (dd/mm/aaaa, Enter = hoy): ");

        if (fgets(linea, sizeof(linea), stdin) == NULL)
        {
            fecha_actual(fecha_proceso);
            return;
        }

        fin_linea = linea;
        while (*fin_linea != '\0' &&
               *fin_linea != '\n')
        {
            fin_linea++;
        }

        if (*fin_linea == '\n')
        {
            *fin_linea = '\0';
            entrada_completa = 1;
        }
        else if (feof(stdin))
        {
            entrada_completa = 1;
        }
        else
        {
            int caracter;

            /*
             * La entrada supero el tamanio del buffer. Se descarta el resto
             * para que la proxima lectura empiece en una linea nueva.
             */
            caracter = getchar();

            /* EOF vale -1 e indica el final de la entrada o del archivo. */
            while (caracter != '\n' && caracter != EOF)
            {
                caracter = getchar();
            }
        }

        if (!entrada_completa)
        {
            printf("Fecha invalida.\n");
            continue;
        }

        if (linea[0] == '\0')
        {
            fecha_actual(fecha_proceso);
            return;
        }

        if (strlen(linea) != TAM_FECHA_TXT - 1)
            formato_valido = 0;

        if (formato_valido)
        {
            int posicion;

            for (posicion = 0; posicion < TAM_FECHA_TXT - 1; posicion++)
            {
                if ((posicion == 2 || posicion == 5) && linea[posicion] != '/')
                    formato_valido = 0;
                else if (posicion != 2 && posicion != 5 &&
                         (linea[posicion] < '0' || linea[posicion] > '9'))
                    formato_valido = 0;
            }
        }

        if (formato_valido &&
            texto_a_fecha(linea, fecha_proceso) &&
            fecha_correcta(fecha_proceso) && fecha_proceso ->anio >= 1601)
        {
            fecha_valida = 1;
        }
        else
        {
            printf("Fecha invalida.\n");
        }
    }
}
