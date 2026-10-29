#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "validaciones.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    calcula el CUIL con el algoritmo modulo 11 del Anexo A.1:
                prefijo 20 (hombre) o 27 (mujer), DNI a 8 digitos y digito
                verificador. Si el resto es 1 el prefijo pasa a 23 y Z es 9
                (hombre) o 4 (mujer).
Parametros:     dni: numero de documento.
                sexo: 'H' o 'M'.
                dest: buffer de al menos TAM_CUIL bytes donde se deja "XY-
                NNNNNNNN-Z".
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void calcular_cuil_ALU(long dni, char sexo, char *dest)
{
    const int pesos[] = {5, 4, 3, 2, 7, 6, 5, 4, 3, 2};
    char digitos[11];
    int prefijo = sexo == 'H' ? PREFIJO_HOMBRE : PREFIJO_MUJER;
    int suma = 0, resto, verificador;
    int i;
    if (!dest)
        return;
    *dest = '\0';
    if (dni < 0 || dni > 99999999L || (sexo != 'H' && sexo != 'M'))
        return;

    sprintf(digitos, "%02d%08ld", prefijo, dni);

    for(i = 0; i < 10; i++)
        suma += (*(digitos + i) - '0')  *   (*(pesos + i));
    resto = suma % 11;
    verificador = resto == 0 ? 0 : 11 - resto;

    if (resto == 1)
    {
        prefijo = PREFIJO_RESTO1;
        verificador = sexo == 'H' ? 9 : 4;
    }

    sprintf(dest, "%02d-%08ld-%c", prefijo, dni, '0' + verificador);
}

/******************************************************************************
Descripcion:    valida un email segun el Anexo A.3: hasta 30 caracteres sin
                espacios, una unica '@', parte local de letras/digitos/puntos
                que no empieza ni termina con punto, y dominio de
                letras/digitos/guiones/puntos con al menos un punto, que no
                empieza ni termina con punto o guion y cuyo ultimo segmento
                tiene al menos 2 letras.
Parametros:     email: cadena a validar.
Retorno:        1 si es valido, 0 en caso contrario.
Observaciones:  No verifica unicidad; eso lo hace socio_email_existe.
******************************************************************************/
int email_valido_ALU(const char *email)
{
    const char *arroba = NULL;
    const char *ultimo_punto = NULL;
    const char *caracter;
    size_t longitud;
    size_t cantidad_arrobas = 0;
    size_t longitud_segmento_final;

    if (!email)
        return 0;

    longitud = strlen(email);

    if (longitud == 0 || longitud > EMAIL_MAX_LEN)
        return 0;

    for (caracter = email; *caracter != '\0'; caracter++)
    {
        if (*caracter == ' ')
            return 0;

        if (*caracter == '@')
        {
            cantidad_arrobas++;
            arroba = caracter;
        }
    }

    if (cantidad_arrobas != 1)
        return 0;

    if (arroba == email || *(arroba + 1) == '\0')
        return 0;

    if (*email == '.' || *(arroba - 1) == '.')
        return 0;

    for (caracter = email; caracter < arroba; caracter++)
    {
        if (!(isalpha((unsigned char)*caracter) ||
              isdigit((unsigned char)*caracter) ||
              *caracter == '.'))
        {
            return 0;
        }
    }

    if (*(arroba + 1) == '.' || *(arroba + 1) == '-' ||
        *(email + longitud - 1) == '.' ||
        *(email + longitud - 1) == '-')
    {
        return 0;
    }

    for (caracter = arroba + 1; *caracter != '\0'; caracter++)
    {
        if (*caracter == '.')
        {
            ultimo_punto = caracter;
        }
        else if (!(isalpha((unsigned char)*caracter) ||
                   isdigit((unsigned char)*caracter) ||
                   *caracter == '-'))
        {
            return 0;
        }
    }

    if (!ultimo_punto)
        return 0;

    longitud_segmento_final = strlen(ultimo_punto + 1);

    if (longitud_segmento_final < 2)
        return 0;

    for (caracter = ultimo_punto + 1; *caracter != '\0'; caracter++)
    {
        if (!isalpha((unsigned char)*caracter))
            return 0;
    }

    return 1;
}
