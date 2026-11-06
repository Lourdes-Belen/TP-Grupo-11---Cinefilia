#include "vector.h"
#include "socio.h"

int comparar_morosos(const void *a, const void *b)
{
    const t_socio *s1 = (const t_socio*)a;
    const t_socio *s2 = (const t_socio*)b;

    int r = compara_fechas(&s1->fecha_ultima_cuota, &s2->fecha_ultima_cuota);
    if (r != 0)
        return r;

    if (s1->dni < s2->dni) return -1;
    if (s1->dni > s2->dni) return 1;
    return 0;
}

int crearVector(tVector *v, size_t tamanyo){
    v->capacidad = CAPACIDAD;

    v->vec = malloc(v->capacidad * tamanyo);

    if(!v->vec)
    {
        printf("Error al reservar memoria \n");
        return ERROR;
    }

    v->tamElem = tamanyo;
    v->cantElem = 0;

    return OK;
}

void destruirVector(tVector *v){
    free(v->vec);
    v->vec = NULL;
    v->cantElem = 0;
    v->capacidad = 0;
}

int insertarEnOrden(tVector *v, void *elemento, size_t tamanyo,int (*cmp)(const void *, const void *))
{
    char *p = v->vec;
    size_t pos = 0;

    if(v->cantElem == v->capacidad){
        return ERROR;
    }

    while(pos < (size_t)v->cantElem && cmp(p + pos* tamanyo,elemento) < 0){
        pos++;
    }

    memmove(p + (pos+1) * tamanyo,
            p + pos * tamanyo,
            (v->cantElem - pos) * tamanyo);

    memcpy(p + pos * tamanyo,elemento,tamanyo);
    v->cantElem++;

    return OK;
}

