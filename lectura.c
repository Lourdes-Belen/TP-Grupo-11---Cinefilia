#include <string.h>
#include "lectura.h"
#include "util.h"
#include "cadena.h"
#include "socio.h"
#include "titulo.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    funcion generica de lectura de archivos de texto. Por cada
                linea no vacia: parsear -> validar -> escribir al final del
                archivo binario e insertar en el indice. Ante el primer error
                (de formato o de validacion) se rechaza la linea invocando
                rechazar y se continua con la siguiente.
Parametros:     path_txt: ruta del archivo de texto (CSV).
                arch_bin: archivo binario destino abierto en "w+b" o "r+b".
                indice: indice de la entidad (se usa para validar duplicados y
                se va completando).
                reg: buffer de un registro de la entidad.
                tamanyo: tamanyo en bytes del registro.
                fecha_proceso: fecha de proceso.
                parsear: convierte una linea en registro.
                validar: valida el registro y deja el tipo de error.
                rechazar: accion ante un rechazo (auditar, ignorar, etc.).
                obtener_clave: devuelve la clave del registro para el indice.
                aceptados, rechazados: salida; contadores de registros.
Retorno:        OK si se pudo abrir el archivo de texto, ERROR en caso
                contrario.
Observaciones:  Se reutiliza para Socios.csv y Titulos.csv; lo especifico de
                cada entidad lo resuelven los punteros a funcion.
******************************************************************************/
int leer_archivo_texto_ALU(const char *path_txt, FILE *arch_bin, t_indice *indice,
                       void *reg, size_t tamanyo, const t_fecha *fecha_proceso,
                       int (*parsear)(const char *, void *),
                       int (*validar)(void *, const t_contexto *, char *),
                       void (*rechazar)(const void *, const char *, const char *, const t_fecha *),
                       long (*obtener_clave)(const void *),
                       unsigned *aceptados, unsigned *rechazados)
{
    FILE *texto;
    char linea[TAM_LINEA], error[TAM_TIPO_ERROR];
    t_contexto contexto;
    int resultado;
    int continuar = 1;
    resultado = 1;
    contexto.indice         = indice;
    contexto.arch_bin       = arch_bin;
    contexto.fecha_proceso  = fecha_proceso;
    contexto.id_modificando = 0;

    if (aceptados)
        *aceptados = 0;

    if (rechazados)
        *rechazados = 0;

    if (!path_txt || !arch_bin || !indice || !reg || !tamanyo || !fecha_proceso ||
        !parsear || !validar || !rechazar || !obtener_clave || !aceptados || !rechazados)
        return 0;

    texto = fopen(path_txt, "rt");
    if (!texto)
        return 0;

    while (continuar && fgets(linea, sizeof(linea), texto))
    {
        t_reg_indice elemento;
        long posicion;
        char *fin;
        int larga;
        int parseado;
        fin = linea;
        larga = 0;
        while (*fin && *fin != '\n')
            fin++;
        if (!*fin && !feof(texto))
        {
            int c = fgetc(texto);
            if (c != EOF && c != '\n')
                larga = 1;
            while (c != EOF && c != '\n')
                c = fgetc(texto);
        }
        *fin = '\0';
        if (*linea || larga)
        {
            memset(reg, 0, tamanyo);
            *error = '\0';
            parseado = parsear(linea, reg);
            if (larga || !parseado)
                strcpy(error, "FORMATO");
            else if (!validar(reg, &contexto, error) && !*error)
                strcpy(error, "FORMATO");
            if (*error)
            {
                rechazar(reg, linea, error, fecha_proceso);
                (*rechazados)++;
            }
            else
            {
                if (fseek(arch_bin, 0, SEEK_END) != 0 || (posicion = ftell(arch_bin)) < 0 ||(unsigned long)posicion % tamanyo != 0)
                {
                    resultado = 0;
                    continuar = 0;
                }
                if (continuar)
                {
                    elemento.id = obtener_clave(reg);
                    elemento.nro_reg = (unsigned)((unsigned long)posicion / tamanyo);
                    if (!indice_insertar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU))
                    {
                        resultado = 0;
                        continuar = 0;
                    }
                }
                if (continuar && (fwrite(reg, tamanyo, 1, arch_bin) != 1 || fflush(arch_bin) != 0))
                {
                    indice_eliminar_ALU(indice, &elemento, sizeof(elemento), compara_reg_indice_ALU);
                    resultado = 0;
                    continuar = 0;
                }
                if (continuar)
                    (*aceptados)++;
            }
        }
    }

    fclose(texto);

    return resultado;
}

/******************************************************************************
Descripcion:    compara dos t_reg_indice por su clave id.
Parametros:     a, b: punteros a t_reg_indice.
Retorno:        Negativo si a < b, 0 si son iguales, positivo si a > b.
Observaciones:  Se usa como cmp en todas las operaciones del TDA indice.
******************************************************************************/
int compara_reg_indice_ALU(const void *a, const void *b)
{
    long izquierda = ((const t_reg_indice *)a)->id;
    long derecha = ((const t_reg_indice *)b)->id;
    return (izquierda > derecha) - (izquierda < derecha);
}

/******************************************************************************
Descripcion:    construye los indices de socios y de titulos a partir de los
                archivos binarios existentes (solo registros activos).
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
Retorno:        1 si ambos binarios existen y se cargaron, 0 en caso
                contrario.
Observaciones:  -
******************************************************************************/
int cargar_indices_ALU(t_indice *ind_socios, t_indice *ind_titulos)
{
    int socios, titulos;
    if (!ind_socios || !ind_titulos || ind_socios == ind_titulos)
        return 0;

    socios = indice_cargar_ALU(ARCH_SOCIOS_BIN, ind_socios, sizeof(t_socio),clave_socio, activo_socio, compara_reg_indice_ALU);
    titulos = indice_cargar_ALU(ARCH_TITULOS_BIN, ind_titulos, sizeof(t_titulo),clave_titulo, activo_titulo, compara_reg_indice_ALU);

    if (!socios || !titulos)
    {
        indice_vaciar_ALU(ind_socios);
        indice_vaciar_ALU(ind_titulos);
        return 0;
    }
    return 1;
}

/******************************************************************************
Descripcion:    opcion 1 del menu: lee Socios.csv y Titulos.csv con la funcion
                generica de lectura, genera socios.dat y titulos.dat
                (sobrescribiendolos) y reconstruye los indices desde los
                binarios generados.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                fecha_proceso: fecha de proceso.
Retorno:        1 si los indices quedaron cargados, 0 en caso contrario.
Observaciones:  Los rechazos de socios se auditan en errores_socios.txt; los
                de titulos solo se cuentan.
******************************************************************************/
int generar_binarios_ALU(t_indice *ind_socios, t_indice *ind_titulos, const t_fecha *fecha_proceso)
{
    FILE *socios, *titulos, *entrada_socios, *entrada_titulos;
    t_socio socio;
    t_titulo titulo;

    unsigned aceptados_socios, rechazados_socios, aceptados_titulos, rechazados_titulos;

    int resultado_socios, resultado_titulos;

    if (!ind_socios || !ind_titulos || ind_socios == ind_titulos || !fecha_proceso)
        return 0;

    entrada_socios = fopen(ARCH_SOCIOS_CSV, "rt");
    if (entrada_socios)
        fclose(entrada_socios);

    entrada_titulos = fopen(ARCH_TITULOS_CSV, "rt");
    if (entrada_titulos)
        fclose(entrada_titulos);

    if (!entrada_socios || !entrada_titulos)
    {
        printf("No se pudieron abrir los CSV de entrada.\n");
        return 0;
    }

    socios = fopen(ARCH_SOCIOS_BIN, "w+b");
    if (!socios)
        return 0;

    titulos = fopen(ARCH_TITULOS_BIN, "w+b");
    if (!titulos)
    {
        fclose(socios);
        indice_vaciar_ALU(ind_socios);
        return 0;
    }

    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    resultado_socios = leer_archivo_texto_ALU(ARCH_SOCIOS_CSV, socios, ind_socios, &socio,
                         sizeof(socio), fecha_proceso, texto_a_socio_ALU, valida_socio_ALU,
                         rechaza_socio, clave_socio, &aceptados_socios, &rechazados_socios);
    resultado_titulos = leer_archivo_texto_ALU(ARCH_TITULOS_CSV, titulos, ind_titulos, &titulo,
                         sizeof(titulo), fecha_proceso, texto_a_titulo_ALU, valida_titulo_ALU,
                         rechaza_titulo, clave_titulo, &aceptados_titulos, &rechazados_titulos);
    fclose(socios);
    fclose(titulos);
    printf("Socios: %u aceptados, %u rechazados.\n", aceptados_socios, rechazados_socios);
    printf("Titulos: %u aceptados, %u rechazados.\n", aceptados_titulos, rechazados_titulos);

    if (!resultado_socios || !resultado_titulos)
    {
        indice_vaciar_ALU(ind_socios);
        indice_vaciar_ALU(ind_titulos);
        printf("La generacion no pudo completarse.\n");
        return 0;
    }
    return cargar_indices_ALU(ind_socios, ind_titulos);
}
