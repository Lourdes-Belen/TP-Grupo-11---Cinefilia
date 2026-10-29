#include <stdlib.h>
#include <string.h>
#include "socio.h"
#include "cadena.h"
#include "util.h"
#include "auditoria.h"
#include "config.h"
#include "menu.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 10 campos en t_socio. Los campos
                con formato invalido quedan con valores que haran fallar la
                validacion correspondiente (dni = 0, fecha 0/0/0, sexo '?',
                etc.). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_socio destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_socio_ALU(const char *linea, void *reg)
{
    /* TODAS las declaraciones al inicio */
    char copia[TAM_LINEA];
    char *campos[CAMPOS_SOCIO + 1];
    char *destinos[5];
    size_t capacidades[5];
    int posiciones[5];
    int posiciones_fecha[3];
    t_fecha *fechas[3];
    t_socio *s = (t_socio *)reg;
    const char *p;
    char *fin;
    long dni;
    int i, formato;

    if (!linea || !s)
        return 0;
    memset(s, 0, sizeof(*s));
    s->estado = 'A';
    s->sexo = '?';
    if (strlen(linea) >= sizeof(copia))
        return 0;
    strcpy(copia, linea);

    fin = copia;
    while (*fin && *fin != '\r' && *fin != '\n')
        fin++;
    *fin = '\0';

    if (separar_campos_ALU(copia, campos, CAMPOS_SOCIO + 1) != CAMPOS_SOCIO)
        return 0;

    p = *campos;
    while (*p >= '0' && *p <= '9')
        p++;
    dni = strtol(*campos, NULL, 10);
    if (p != *campos && !*p && dni > DNI_MIN && dni < DNI_MAX)
        s->dni = dni;

    /* Inicializar arreglos de destinos, capacidades y posiciones */
    *(destinos + 0) = s->cuil;
    *(destinos + 1) = s->apellidos_nombres;
    *(destinos + 2) = s->categoria;
    *(destinos + 3) = s->plan;
    *(destinos + 4) = s->email_tutor;

    *(capacidades + 0) = sizeof(s->cuil);
    *(capacidades + 1) = sizeof(s->apellidos_nombres);
    *(capacidades + 2) = sizeof(s->categoria);
    *(capacidades + 3) = sizeof(s->plan);
    *(capacidades + 4) = sizeof(s->email_tutor);

    *(posiciones + 0) = 1;
    *(posiciones + 1) = 2;
    *(posiciones + 2) = 6;
    *(posiciones + 3) = 8;
    *(posiciones + 4) = 9;

    for (i = 0; i < 5; i++)
    {
        const char *campo = *(campos + *(posiciones + i));
        if (strlen(campo) < *(capacidades + i))
            strcpy(*(destinos + i), campo);
    }

    if (strlen(*(campos + 4)) == 1)
        s->sexo = **(campos + 4);

    *(fechas + 0) = &s->fecha_nacimiento;
    *(fechas + 1) = &s->fecha_afiliacion;
    *(fechas + 2) = &s->fecha_ultima_cuota;

    *(posiciones_fecha + 0) = 3;
    *(posiciones_fecha + 1) = 5;
    *(posiciones_fecha + 2) = 7;

    for (i = 0; i < 3; i++)
    {
        const char *campo = *(campos + *(posiciones_fecha + i));
        formato = strlen(campo) == TAM_FECHA_TXT - 1;
        for (p = campo; formato && *p; p++)
        {
            size_t posicion = (size_t)(p - campo);
            formato = (posicion == 2 || posicion == 5) ? *p == '/' : (*p >= '0' && *p <= '9');
        }
        if (formato)
            texto_a_fecha(campo, *(fechas + i));
    }

    /* Un email demasiado largo no debe convertirse en el vacio de un adulto. */
    if (strlen(*(campos + 9)) >= sizeof(s->email_tutor))
        strcpy(s->email_tutor, "@");
    return 1;
}
/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.1 en orden (DNI, Sexo,
                CUIL, Apellidos y Nombres, Fecha de nacimiento, Fecha de
                afiliacion, Categoria, Fecha de ultima cuota, Plan, Email
                tutor). Normaliza apellidos y nombres. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_socio a validar (se modifica al normalizar).
                ctx: contexto con indice, archivo binario, fecha de proceso e
                id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el socio es valido, ERROR en caso contrario.
Observaciones:  Si ctx->id_modificando == dni no se considera duplicado y se
                excluye ese DNI al verificar unicidad del email. Cumple con el
                puntero validar de leer_archivo_texto.
******************************************************************************/

int valida_socio_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_socio *s;
    t_reg_indice clave;
    char calculado[TAM_CUIL];
    const char *error;
    unsigned i;
    const t_reg_indice *p;
    t_socio otro;

    s = (t_socio *)reg;
    error = NULL;

    if(!s || !ctx || !ctx->indice || !ctx->fecha_proceso || !tipo_error)
        return ERROR;

    *tipo_error = '\0';

    clave.id = s->dni;
    clave.nro_reg = 0;

    if(s->dni <= DNI_MIN || s->dni >= DNI_MAX ||
       (ctx->id_modificando != s->dni &&
        indice_buscar_ALU(ctx->indice, &clave, sizeof(clave),
                          compara_reg_indice_ALU) != NO_EXISTE))
    {
        error = "DNI";
    }
    else if(s->sexo != 'H' && s->sexo != 'M')
    {
        error = "SEXO";
    }
    else
    {
        calcular_cuil_ALU(s->dni, s->sexo, calculado);

        if(!memchr(s->cuil, '\0', sizeof(s->cuil)) ||
           strcmp(s->cuil, calculado) != 0)
            error = "CUIL";
    }

    if(!error)
    {
        if(!memchr(s->apellidos_nombres, '\0',
                   sizeof(s->apellidos_nombres)))
            error = "APELLIDOS_NOMBRES";
        else
        {
            arreglar_apyn_ALU(s->apellidos_nombres);

            if(!*s->apellidos_nombres ||
               !strcmp(s->apellidos_nombres, ","))
                error = "APELLIDOS_NOMBRES";
        }
    }

    if(!error &&
       (!fecha_correcta(&s->fecha_nacimiento) ||
        edad(&s->fecha_nacimiento, ctx->fecha_proceso) < EDAD_MINIMA))
        error = "FECHA_NACIMIENTO";

    if(!error &&
       (!fecha_correcta(&s->fecha_afiliacion) ||
        compara_fechas(&s->fecha_afiliacion,
                       &s->fecha_nacimiento) <= 0 ||
        compara_fechas(&s->fecha_afiliacion,
                       ctx->fecha_proceso) > 0))
        error = "FECHA_AFILIACION";

    if(!error &&
       (!memchr(s->categoria, '\0', sizeof(s->categoria)) ||
        strcmp(s->categoria,
               edad(&s->fecha_nacimiento, ctx->fecha_proceso) < EDAD_ADULTO ?
               CAT_MENOR : CAT_ADULTO) != 0))
        error = "CATEGORIA";

    if(!error &&
       (!fecha_correcta(&s->fecha_ultima_cuota) ||
        compara_fechas(&s->fecha_ultima_cuota,
                       &s->fecha_afiliacion) < 0 ||
        compara_fechas(&s->fecha_ultima_cuota,
                       ctx->fecha_proceso) > 0))
        error = "FECHA_ULTIMA_CUOTA";

    if(!error &&
       (!memchr(s->plan, '\0', sizeof(s->plan)) ||
        plan_indice(s->plan) < 0))
        error = "PLAN";

    if(!error)
    {
        if(!memchr(s->email_tutor, '\0', sizeof(s->email_tutor)) ||
           (!strcmp(s->categoria, CAT_ADULTO) && *s->email_tutor) ||
           (!strcmp(s->categoria, CAT_MENOR) &&
            !email_valido_ALU(s->email_tutor)))
        {
            error = "EMAIL_TUTOR";
        }
        else if(*s->email_tutor)
        {
            if(!ctx->arch_bin)
                error = "EMAIL_TUTOR";
            else
            {
                p = (const t_reg_indice *)ctx->indice->vindice;

                for(i = 0;
                    i < ctx->indice->cantidad_elementos_actual && !error;
                    i++, p++)
                {
                    if(p->id == ctx->id_modificando)
                        continue;

                    if(!socio_leer_ALU(ctx->arch_bin, p->nro_reg, &otro) ||
                       !memchr(otro.email_tutor, '\0',
                               sizeof(otro.email_tutor)) ||
                       !strcmp(otro.email_tutor, s->email_tutor))
                        error = "EMAIL_TUTOR";
                }
            }
        }
    }

    if(error)
    {
        strcpy(tipo_error, error);
        return ERROR;
    }

    return OK;
}
/******************************************************************************
Descripcion:    lee el registro nro_reg del archivo binario de socios.
Parametros:     arch: archivo abierto en modo binario.
                nro_reg: numero de registro.
                s: socio destino.
Retorno:        OK si se pudo leer, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_leer_ALU(FILE *arch, unsigned nro_reg, t_socio *s)
{

    if(fseek(arch,nro_reg * sizeof(t_socio), SEEK_SET)!=0)
        return ERROR;

    if(fread(s,sizeof(t_socio),1,arch)!=1)
        return ERROR;

    return OK;
}

/******************************************************************************
Descripcion:    escribe el socio en la posicion nro_reg del archivo binario y
                hace flush.
Parametros:     arch: archivo abierto en modo de escritura binaria.
                nro_reg: numero de registro.
                s: socio a grabar.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_escribir_ALU(FILE *arch, unsigned nro_reg, const t_socio *s)
{
    if (fseek(arch,nro_reg * sizeof(*s), SEEK_SET) != 0)
        return ERROR;

    if(fwrite(s,sizeof(t_socio),1,arch)!=1)
        return ERROR;

    if(fflush(arch) != 0)
        return ERROR;

    return OK;
}
/******************************************************************************
Descripcion:    aplica la regla del Anexo A.4: el socio esta al dia si la
                ultima cuota es del mes de la fecha de proceso, o del mes
                anterior cuando el dia de la fecha de proceso es <= 10.
Parametros:     ultima_cuota: fecha de ultima cuota paga.
                fecha_proceso: fecha de proceso.
Retorno:        1 si esta al dia, 0 en caso contrario.
Observaciones:  -
******************************************************************************/
int cuota_al_dia_ALU(const t_fecha *ultima_cuota,const t_fecha *fecha_proceso)
{
    int mes_anterior, anio_anterior;

    if(!ultima_cuota || !fecha_proceso ||
       !fecha_correcta(ultima_cuota) ||
       !fecha_correcta(fecha_proceso))
        return 0;

    if(ultima_cuota->mes == fecha_proceso->mes &&
       ultima_cuota->anio == fecha_proceso->anio)
        return 1;

    if(fecha_proceso->mes == 1)
    {
        mes_anterior = 12;
        anio_anterior = fecha_proceso->anio - 1;
    }
    else
    {
        mes_anterior = fecha_proceso->mes - 1;
        anio_anterior = fecha_proceso->anio;
    }

    if(fecha_proceso->dia <= DIA_LIMITE_CUOTA &&
       ultima_cuota->mes == mes_anterior &&
       ultima_cuota->anio == anio_anterior)
        return 1;

    return 0;
}
/******************************************************************************
Descripcion:    opcion Alta: carga un socio por teclado, lo valida con
                valida_socio, lo agrega al final de socios.dat con estado 'A'
                y lo inserta en el indice. Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si el socio fue agregado, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_alta_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    char linea[TAM_LINEA], valor[TAM_LINEA];
    char dni_texto[TAM_LINEA], error[TAM_TIPO_ERROR];
    const char *etiquetas[] = {
        "DNI",
        "CUIL (XY-NNNNNNNN-Z)",
        "Apellidos y nombres",
        "Nacimiento (dd/mm/aaaa)",
        "Sexo (H/M)",
        "Afiliacion (dd/mm/aaaa)",
        "Categoria (MENOR/ADULTO)",
        "Ultima cuota (dd/mm/aaaa)",
        "Plan (BASIC/PREMIUM/VIP/FAMILY)",
        "Email tutor (vacio si ADULTO, '-' para borrar)"
    };
    const char *separador;
    size_t usados, longitud_valor, longitud_sep;
    t_socio socio;
    FILE *archivo;
    t_contexto contexto;
    int valido, resultado, i;
    long posicion;
    t_reg_indice elemento;

    if(!indice || !path || !fecha_proceso)
        return ERROR;

    linea[0] = '\0';
    dni_texto[0] = '\0';
    usados = 0;

    for(i = 0; i < CAMPOS_SOCIO; i++)
    {
        printf("%s: ", etiquetas[i]);
        leer_linea(valor, sizeof(valor));

        if(i == 0)
            strcpy(dni_texto, valor);

        if(i == 9 && strcmp(valor, "-") == 0)
            valor[0] = '\0';

        separador = "";
        if(i != 0)
            separador = ";";

        longitud_sep = strlen(separador);
        longitud_valor = strlen(valor);

        if(longitud_sep + longitud_valor >= sizeof(linea) - usados)
        {
            auditoria_registrar_ALU(ARCH_ERRORES, dni_texto,
                                    "FORMATO", fecha_proceso);
            printf("Alta rechazada: FORMATO.\n");
            return ERROR;
        }

        strcat(linea, separador);
        strcat(linea, valor);

        usados += longitud_sep + longitud_valor;
    }

    archivo = fopen(path, "r+b");

    if(!archivo && !archivo_existe(path))
        archivo = fopen(path, "w+b");

    if(!archivo)
    {
        printf("No se pudo abrir el archivo de socios.\n");
        return ERROR;
    }

    resultado = ERROR;

    contexto.indice = indice;
    contexto.arch_bin = archivo;
    contexto.fecha_proceso = fecha_proceso;
    contexto.id_modificando = 0;

    valido = texto_a_socio_ALU(linea, &socio);

    if(valido)
        valido = valida_socio_ALU(&socio, &contexto, error);
    else
        strcpy(error, "FORMATO");

    if(!valido)
    {
        if(!auditoria_registrar_ALU(ARCH_ERRORES, dni_texto,
                                    error, fecha_proceso))
            printf("No se pudo guardar la auditoria.\n");

        printf("Alta rechazada: %s.\n", error);
    }
    else
    {
        if(fseek(archivo, 0, SEEK_END) == 0)
        {
            posicion = ftell(archivo);

            if(posicion >= 0 && posicion % sizeof(t_socio) == 0)
            {
                elemento.nro_reg = (unsigned)(posicion / sizeof(t_socio));
                elemento.id = socio.dni;

                if(indice_insertar_ALU(indice, &elemento,sizeof(elemento),compara_reg_indice_ALU))
                {
                    resultado = socio_escribir_ALU(archivo,elemento.nro_reg,&socio);

                    if(!resultado)
                        indice_eliminar_ALU(indice, &elemento,sizeof(elemento),compara_reg_indice_ALU);
                }
            }
        }
    }

    fclose(archivo);

    if(resultado)
        printf("Socio agregado.\n");

    return resultado;
}

/******************************************************************************
Descripcion:    opcion Modificacion: pide el DNI, lo busca en el indice (un
                socio dado de baja no se puede modificar), muestra los datos
                actuales, pide los nuevos (Enter conserva), valida y
                sobrescribe el registro. El DNI y el estado no se modifican.
                Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se modifico, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_modificacion_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    char valor[TAM_LINEA], anterior[TAM_LINEA];
    char linea[TAM_LINEA], dni_texto[TAM_LINEA];
    char error[TAM_TIPO_ERROR];
    const char *etiquetas[] = {
        "CUIL (XY-NNNNNNNN-Z)",
        "Apellidos y nombres",
        "Nacimiento",
        "Sexo",
        "Afiliacion",
        "Categoria",
        "Ultima cuota",
        "Plan (BASIC/PREMIUM/VIP/FAMILY)",
        "Email tutor (vacio si ADULTO, '-' para borrar)"
    };
    const char *p;
    char *destino;
    const char *origen;
    t_socio original, nuevo;
    t_reg_indice elemento;
    t_contexto contexto;
    FILE *archivo;
    size_t usados, longitud;
    int i, valido, resultado;

    if(!indice || !path || !fecha_proceso)
        return ERROR;

    printf("DNI del socio a modificar: ");
    leer_linea(dni_texto, sizeof(dni_texto));

    p = dni_texto;
    while(*p >= '0' && *p <= '9')
        p++;

    elemento.id = strtol(dni_texto, NULL, 10);

    if(p == dni_texto || *p ||
       elemento.id <= DNI_MIN || elemento.id >= DNI_MAX)
    {
        auditoria_registrar_ALU(ARCH_ERRORES, dni_texto,
                                "DNI", fecha_proceso);
        printf("DNI invalido.\n");
        return ERROR;
    }

    if(indice_buscar_ALU(indice, &elemento, sizeof(elemento),
                         compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("El socio no existe o esta dado de baja.\n");
        return ERROR;
    }

    archivo = fopen(path, "r+b");

    if(!archivo)
        return ERROR;

    resultado = ERROR;

    if(socio_leer_ALU(archivo, elemento.nro_reg, &original))
    {
        socio_mostrar(&original);

        sprintf(linea, "%ld", original.dni);
        usados = strlen(linea);

        for(i = 0; i < CAMPOS_SOCIO - 1; i++)
        {
            switch(i)
            {
                case 0:
                    strcpy(anterior, original.cuil);
                    break;

                case 1:
                    strcpy(anterior, original.apellidos_nombres);
                    break;

                case 2:
                    sprintf(anterior, "%02d/%02d/%04d",
                            original.fecha_nacimiento.dia,
                            original.fecha_nacimiento.mes,
                            original.fecha_nacimiento.anio);
                    break;

                case 3:
                    *anterior = original.sexo;
                    *(anterior + 1) = '\0';
                    break;

                case 4:
                    sprintf(anterior, "%02d/%02d/%04d",
                            original.fecha_afiliacion.dia,
                            original.fecha_afiliacion.mes,
                            original.fecha_afiliacion.anio);
                    break;

                case 5:
                    strcpy(anterior, original.categoria);
                    break;

                case 6:
                    sprintf(anterior, "%02d/%02d/%04d",
                            original.fecha_ultima_cuota.dia,
                            original.fecha_ultima_cuota.mes,
                            original.fecha_ultima_cuota.anio);
                    break;

                case 7:
                    strcpy(anterior, original.plan);
                    break;

                default:
                    strcpy(anterior, original.email_tutor);
                    break;
            }

            printf("%s [%s] (Enter conserva): ",
                   *(etiquetas + i), anterior);

            leer_linea(valor, sizeof(valor));

            if(i == 8 && strcmp(valor, "-") == 0)
            {
                *valor = '\0';
                *anterior = '\0';
            }

            if(*valor == '\0')
                strcpy(valor, anterior);

            longitud = strlen(valor);

            if(usados + longitud + 2 > sizeof(linea))
            {
                auditoria_registrar_ALU(ARCH_ERRORES, dni_texto,
                                        "FORMATO", fecha_proceso);
                printf("Modificacion rechazada: FORMATO.\n");
                i = CAMPOS_SOCIO;
            }
            else
            {
                *(linea + usados) = ';';
                usados++;

                destino = linea + usados;
                origen = valor;

                while(*origen != '\0')
                {
                    *destino = *origen;
                    destino++;
                    origen++;
                }

                *destino = '\0';
                usados += longitud;
            }
        }

        if(i == CAMPOS_SOCIO - 1)
        {
            contexto.indice = indice;
            contexto.arch_bin = archivo;
            contexto.fecha_proceso = fecha_proceso;
            contexto.id_modificando = original.dni;

            valido = texto_a_socio_ALU(linea, &nuevo);

            if(valido)
            {
                nuevo.dni = original.dni;
                nuevo.estado = original.estado;

                valido = valida_socio_ALU(&nuevo, &contexto, error);
            }
            else
                strcpy(error, "FORMATO");

            if(valido)
            {
                resultado = socio_escribir_ALU(archivo,
                                               elemento.nro_reg,
                                               &nuevo);

                if(!resultado)
                {
                    socio_escribir_ALU(archivo, elemento.nro_reg,
                                       &original);
                }
            }
            else
            {
                if(!auditoria_registrar_ALU(ARCH_ERRORES, dni_texto,
                                            error, fecha_proceso))
                    printf("No se pudo guardar la auditoria.\n");

                printf("Modificacion rechazada: %s.\n", error);
            }
        }
    }

    fclose(archivo);

    if(resultado)
        printf("Socio modificado.\n");

    return resultado;
}

/******************************************************************************
Descripcion:    baja logica de socio: busca el DNI en el indice, pone estado
                'B' en el registro del archivo y lo elimina del indice.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se dio de baja, ERROR si no existe o ya estaba dado de
                baja.
Observaciones:  -
******************************************************************************/
int socio_baja_ALU(t_indice *indice, const char *path)
{
    char texto[TAM_LINEA];
    t_reg_indice elemento;
    t_socio original, baja;
    FILE *archivo;
    const char *p;
    int resultado;

    if(!indice || !path)
        return ERROR;

    printf("DNI del socio a dar de baja: ");
    leer_linea(texto, sizeof(texto));

    p = texto;

    while(*p >= '0' && *p <= '9')
        p++;

    elemento.id = strtol(texto, NULL, 10);

    if(p == texto || *p ||
       elemento.id <= DNI_MIN || elemento.id >= DNI_MAX)
    {
        printf("El socio no existe o ya esta dado de baja.\n");
        return ERROR;
    }

    if(indice_buscar_ALU(indice, &elemento, sizeof(elemento),
                         compara_reg_indice_ALU) == NO_EXISTE)
    {
        printf("El socio no existe o ya esta dado de baja.\n");
        return ERROR;
    }

    archivo = fopen(path, "r+b");

    if(!archivo)
        return ERROR;

    resultado = ERROR;

    if(socio_leer_ALU(archivo, elemento.nro_reg, &original))
    {
        baja = original;
        baja.estado = 'B';

        resultado = socio_escribir_ALU(archivo,
                                       elemento.nro_reg,
                                       &baja);

        if(resultado)
        {
            resultado = indice_eliminar_ALU(indice, &elemento,
                                            sizeof(elemento),
                                            compara_reg_indice_ALU);

            if(!resultado)
            {
                clearerr(archivo);
                socio_escribir_ALU(archivo, elemento.nro_reg,
                                   &original);
            }
        }
    }

    fclose(archivo);

    if(resultado)
        printf("Socio dado de baja.\n");

    return resultado;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por DNI y solo con activos) y
                lista los socios leyendo cada registro del archivo.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_listar_ALU(const t_indice *indice, const char *path)
{
    FILE *archivo;
    int resultado = 1;
    const t_reg_indice *p;
    unsigned i;
    if (!indice || !path)
        return 0;
    archivo = fopen(path, "rb");
    if (!archivo)
    {
        printf("No hay datos cargados.\n");
        return 0;
    }
    p = (const t_reg_indice *)indice->vindice;
    for (i = 0; i < indice->cantidad_elementos_actual; i++, p++)
    {
        t_socio s;
        if (!socio_leer_ALU(archivo, p->nro_reg, &s))
        {
            resultado = 0;
            break;
        }
        socio_mostrar(&s);
    }
    fclose(archivo);
    if (resultado)
        printf("Total de socios activos: %u\n", indice->cantidad_elementos_actual);
    return resultado;
}

/******************************************************************************
Descripcion:    recorre el indice y exporta a un archivo binario los socios
                con plan PREMIUM o VIP y cuota al dia (el estado 'A' lo
                garantiza el indice). Sobrescribe el archivo de exportacion
                anterior.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                path_export: ruta de beta_testers.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo exportar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_exportar_ALU(const t_indice *indice, const char *path,const char *path_export, const t_fecha *fecha_proceso)
{
    FILE *origen, *destino;
    unsigned cantidad, i;
    int resultado;
    const t_reg_indice *p;
    t_socio socio;

    if(!indice || !path || !path_export || !fecha_proceso ||
       strcmp(path, path_export) == 0)
        return ERROR;

    origen = fopen(path, "rb");

    if(!origen)
        return ERROR;

    destino = fopen(path_export, "wb");

    if(!destino)
    {
        fclose(origen);
        return ERROR;
    }

    cantidad = 0;
    resultado = OK;
    i = 0;
    p = (const t_reg_indice *)indice->vindice;

    while(i < indice->cantidad_elementos_actual && resultado)
    {
        if(socio_leer_ALU(origen, p->nro_reg, &socio))
        {
            if(strcmp(socio.plan, "PREMIUM") == 0 ||
               strcmp(socio.plan, "VIP") == 0)
            {
                if(cuota_al_dia_ALU(&socio.fecha_ultima_cuota,
                                    fecha_proceso))
                {
                    if(fwrite(&socio, sizeof(t_socio), 1, destino) == 1)
                        cantidad++;
                    else
                        resultado = ERROR;
                }
            }
        }
        else
            resultado = ERROR;

        p++;
        i++;
    }

    fclose(origen);
    fclose(destino);

    if(resultado)
        printf("Socios exportados: %u\n", cantidad);
    else
        printf("No se pudo completar la exportacion.\n");

    return resultado;
}

/******************************************************************************
Descripcion:    opcion 2 del menu: submenu para elegir entre alta y
                modificacion manual de socio.
Parametros:     ind_socios: indice de socios.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void abm_socio_ALU(t_indice *indice, const t_fecha *fecha_proceso)
{
    char respuesta[TAM_LINEA];

    printf("A - Alta\nM - Modificacion\nV - Volver\nOpcion: ");
    leer_linea(respuesta, sizeof(respuesta));

    if(*respuesta == 'A' || *respuesta == 'a')
        socio_alta_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
    else
    {
        if(*respuesta == 'M' || *respuesta == 'm')
            socio_modificacion_manual_ALU(indice, ARCH_SOCIOS_BIN,
                                          fecha_proceso);
        else
        {
            if(*respuesta != 'V' && *respuesta != 'v')
                printf("Opcion invalida.\n");
        }
    }
}
