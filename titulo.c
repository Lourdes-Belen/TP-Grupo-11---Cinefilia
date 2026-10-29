#include <stdlib.h>
#include <string.h>
#include "titulo.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 4 campos en t_titulo. Los campos
                numericos invalidos quedan en valores que fallan la validacion
                (id = 0, stock = -1). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_titulo destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_titulo_ALU(const char *linea, void *reg)
{
    char copia[TAM_LINEA], *campos[CAMPOS_TITULO + 1];
    char *fin;
    int i;

    t_titulo *t = (t_titulo *)reg;
    if (!linea || !t)
        return 0;

    memset(t, 0, sizeof(*t));
    t->estado = 'A';
    t->stock = -1;

    if (strlen(linea) >= sizeof(copia))
        return 0;
    strcpy(copia, linea);
    fin = copia;

    while (*fin && *fin != '\n')
        fin++;
    *fin = '\0';

    if (separar_campos_ALU(copia, campos, CAMPOS_TITULO + 1) != CAMPOS_TITULO)
        return 0;

    for (i = 0; i < 2; i++)
    {
        const char *campo = *(campos + (i == 0 ? 0 : 3)), *p = campo;
        while (*p >= '0' && *p <= '9')
            p++;
        if (p != campo && !*p)
        {
            if (i == 0)
                t->id = atoi(campo);
            else
                t->stock = atoi(campo);
        }
    }

    if (strlen(*(campos + 1)) < sizeof(t->titulo))
        strcpy(t->titulo, *(campos + 1));
    if (strlen(*(campos + 2)) < sizeof(t->genero))
        strcpy(t->genero, *(campos + 2));

    return 1;
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.2 en orden: ID > 0 y no
                existente en el indice, titulo normalizado no vacio, genero
                dentro de GENEROS[] y stock >= 0. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_titulo (se modifica al normalizar el titulo).
                ctx: contexto con indice e id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el titulo es valido, ERROR en caso contrario.
Observaciones:  Se pasa por puntero como validar a leer_archivo_texto. Se reutiliza en las altas del
                lote de novedades.
******************************************************************************/
int valida_titulo_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_titulo *t = (t_titulo *)reg;
    t_reg_indice clave;
    const char *error = NULL;
    if (!t || !ctx || !ctx->indice || !tipo_error)
        return 0;
    *tipo_error = '\0';
    clave.id = t->id;
    clave.nro_reg = 0;
    if (t->id <= 0 || (ctx->id_modificando != t->id &&
        indice_buscar_ALU(ctx->indice, &clave, sizeof(clave), compara_reg_indice_ALU) != NO_EXISTE))
        error = "ID";
    else if (!memchr(t->titulo, '\0', sizeof(t->titulo)))
        error = "TITULO";
    else
    {
        arreglar_titulo(t->titulo);
        if (!*t->titulo)
            error = "TITULO";
    }
    if (!error && (!memchr(t->genero, '\0', sizeof(t->genero)) || genero_indice(t->genero) < 0))
        error = "GENERO";
    if (!error && t->stock < 0)
        error = "STOCK";
    if (error)
    {
        strcpy(tipo_error, error);
        return 0;
    }
    return 1;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por ID y solo con activos) y lista
                los titulos leyendo cada registro del archivo.
Parametros:     indice: indice de titulos.
                path: ruta de titulos.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int titulos_listar_ALU(const t_indice *indice, const char *path)
{
    FILE *archivo;
    const t_reg_indice *p;
    unsigned i;
    int resultado = 1;
    if (!indice || !path)
        return 0;
    archivo = fopen(path, "rb");
    if (!archivo)
    {
        printf("No hay datos cargados.\n");
        return 0;
    }
    titulos_encabezado();
    p = (const t_reg_indice *)indice->vindice;
    for (i = 0; i < indice->cantidad_elementos_actual; i++, p++)
    {
        t_titulo t;
        if (!titulo_leer(archivo, p->nro_reg, &t))
        {
            resultado = 0;
            break;
        }
        titulo_mostrar(&t);
    }
    fclose(archivo);
    if (resultado)
        printf("Total de titulos activos: %u\n", indice->cantidad_elementos_actual);

    return resultado;
}
