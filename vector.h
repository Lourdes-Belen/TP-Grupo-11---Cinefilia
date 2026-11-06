#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACIDAD 20

typedef struct
{
    void *vec;
    size_t tamElem;
    int cantElem;
    int capacidad;

}tVector;

typedef int (*Cmp)(const void *,const void*);

int crearVector(tVector *v, size_t tamanyo);
void destruirVector(tVector *v);

int insertarEnOrden(tVector *v, void *elemento, size_t tamanyo,int (*cmp)(const void *, const void *));
int recorrerVector(tVector *v, size_t tamanyo);

#endif // VECTOR_H_INCLUDED
