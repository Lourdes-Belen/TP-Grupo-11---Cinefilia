#include <string.h>
#include "novedad.h"
#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    recorre secuencialmente novedades_catalogo.dat y aplica cada
                operacion sobre el indice y titulos.dat: 'A' agrega si el ID
                no esta en el indice (validando como en la carga), 'B' pone
                estado 'B' en el archivo y elimina del indice, 'M' reemplaza
                titulo y genero y suma el stock (rechaza si quedaria
                negativo). Al final muestra el reporte de procesadas y
                rechazadas.
Parametros:     indice: indice de titulos.
                path_novedades: ruta de novedades_catalogo.dat.
                path_titulos: ruta de titulos.dat.
                fecha_proceso: fecha de proceso (para el contexto de
                validacion).
Retorno:        OK si se pudieron abrir los archivos, ERROR en caso contrario.
Observaciones:  Como el indice solo tiene activos, un 'A' sobre un ID dado de
                baja se agrega como registro nuevo al final del archivo.
******************************************************************************/
int novedades_procesar_ALU(t_indice *indice, const char *path_novedades,
                       const char *path_titulos, const t_fecha *fecha_proceso)
{
    FILE *novedades, *titulos;
    t_novedad novedad;
    unsigned procesadas = 0, rechazadas = 0;
    size_t leidos;
    int resultado = 1;
    if (!indice || !path_novedades || !path_titulos || !fecha_proceso)
        return 0;
    novedades = fopen(path_novedades, "rb");
    if (!novedades)
    {
        printf("No se pudo abrir el lote de novedades.\n");
        return 0;
    }
    titulos = fopen(path_titulos, "r+b");
    if (!titulos)
    {
        fclose(novedades);
        printf("No hay datos cargados.\n");
        return 0;
    }
    while ((leidos = fread(&novedad, 1, sizeof(novedad), novedades)) == sizeof(novedad))
    {
        t_reg_indice elemento;
        t_titulo original, actualizado;
        char error[TAM_TIPO_ERROR];
        int existe;
        t_contexto contexto;
        elemento.nro_reg = 0;
        elemento.id = novedad.titulo.id;
        *error = '\0';
        existe = indice_buscar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU) != NO_EXISTE;
        contexto.indice         = indice;
        contexto.arch_bin       = titulos;
        contexto.fecha_proceso  = fecha_proceso;
        contexto.id_modificando = 0;
        switch (novedad.codigo_operacion)
        {
            case OP_ALTA:
            {
                long posicion;
                actualizado = novedad.titulo;
                actualizado.estado = 'A';
                if (!valida_titulo_ALU(&actualizado, &contexto, error))
                    break;
                if (fseek(titulos, 0, SEEK_END) != 0 || (posicion = ftell(titulos)) < 0 ||
                    (unsigned long)posicion % sizeof(actualizado) ||
                    (unsigned long)(unsigned)((unsigned long)posicion / sizeof(actualizado)) != (unsigned long)posicion / sizeof(actualizado))
                {
                    resultado = 0;
                    break;
                }
                elemento.nro_reg = (unsigned)((unsigned long)posicion / sizeof(actualizado));
                if (!indice_insertar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU))
                {
                    resultado = 0;
                    break;
                }
                if (!titulo_escribir(titulos, elemento.nro_reg, &actualizado))
                {
                    indice_eliminar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU);
                    resultado = 0;
                }
                break;
            }
            case OP_BAJA:
            case OP_MODIFICACION:
                if (!existe)
                {
                    strcpy(error, "ID_INEXISTENTE");
                    break;
                }
                if (!titulo_leer(titulos, elemento.nro_reg, &original))
                {
                    resultado = 0;
                    break;
                }
                actualizado = original;
                if (novedad.codigo_operacion == OP_BAJA)
                    actualizado.estado = 'B';
                else
                {
                    long stock;
                    stock = (long)original.stock + novedad.titulo.stock;
                    actualizado = novedad.titulo;
                    actualizado.id = original.id;
                    actualizado.estado = original.estado;
                    contexto.id_modificando = original.id;
                    /* El stock se valida despues del titulo y del genero. */
                    actualizado.stock = stock < 0 ? -1 : (int)stock;
                    if (!valida_titulo_ALU(&actualizado, &contexto, error))
                        break;
                }
                if (!titulo_escribir(titulos, elemento.nro_reg, &actualizado))
                    resultado = 0;
                else if (novedad.codigo_operacion == OP_BAJA &&
                    !indice_eliminar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU))
                    resultado = 0;
                if (!resultado)
                {
                    if (!titulo_escribir(titulos, elemento.nro_reg, &original))
                        printf("No se pudo restaurar el titulo original.\n");
                }
                break;
            default:
                strcpy(error, "OPERACION");
                break;
        }
        if (!resultado)
        {
            printf("ID %d: error de archivo o memoria; lote interrumpido.\n", novedad.titulo.id);
            break;
        }
        if (*error)
        {
            rechazadas++;
            printf("ID %d rechazado: %s.\n", novedad.titulo.id, error);
        }
        else
            procesadas++;
    }
    if (leidos != 0 && leidos != sizeof(novedad))
    {
        printf("Lote incompleto o error de lectura.\n");
        resultado = 0;
    }
    fclose(novedades);
    fclose(titulos);
    printf("Novedades procesadas: %u; rechazadas: %u.\n", procesadas, rechazadas);
    return resultado;
}
