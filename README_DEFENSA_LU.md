# Practica para la defensa individual - Cinefilia

Este archivo propone cinco ejercicios parecidos a los cambios que pueden
pedirse durante la defensa. Estan pensados para resolverse directamente sobre
el TP, respetando la arquitectura existente:

- registros en archivos binarios;
- un `t_indice` ordenado por clave;
- baja logica mediante el campo `estado`;
- funciones de lectura y escritura;
- TDA Fecha;
- TDA Vector implementado por el indice;
- ordenamiento y algoritmo MERGE.

## Reglas generales para practicar

1. Trabajar siempre con las funciones `_ALU`.
2. No recorrer el archivo binario completo cuando el indice ya permite ubicar
   los registros.
3. Si una funcionalidad modifica un registro, actualizar tambien el indice si
   cambia su estado o su clave.
4. No eliminar fisicamente registros: una baja debe ser logica.
5. Validar entradas y controlar errores de apertura, lectura, escritura y
   memoria.
6. Probar cada ejercicio con datos validos, claves inexistentes, registros
   dados de baja y archivos vacios.

## Ejercicio 1 - Consulta puntual de un titulo

Agregar una opcion **Consultar titulo por ID**.

### Requerimiento

Solicitar un ID, buscarlo utilizando `indice_buscar_ALU` y mostrar todos los
datos del titulo: ID, nombre, genero, stock y estado. Si el ID no existe o el
titulo esta inactivo, informar claramente la situacion.

### Temas que practica

- TDA indice;
- busqueda binaria;
- uso de `t_reg_indice.nro_reg`;
- lectura de un registro puntual con `titulo_leer_ALU`;
- validacion de entrada numerica;
- integracion de una nueva opcion al menu.

### Condiciones de correccion

- No recorrer secuencialmente `titulos.dat`.
- Usar el `nro_reg` devuelto por el indice para leer el archivo.
- No confundir la posicion del elemento dentro del vector del indice con la
  posicion del registro dentro del archivo.
- No modificar el indice ni el archivo al consultar.

### Variantes para repregunta

- Consultar por genero y mostrar todos los titulos de ese genero.
- Mostrar un mensaje diferente si existe el ID pero el registro esta dado de
  baja.

## Ejercicio 2 - Baja de titulos sin stock

Agregar una opcion **Dar de baja titulos agotados**.

### Requerimiento

Recorrer los elementos activos del indice de titulos. Para cada uno, leer el
registro desde el archivo y, si su stock es cero, cambiar su estado a `'B'`,
escribirlo y eliminarlo del indice.

Al finalizar, informar cuantos titulos fueron dados de baja.

### Temas que practica

- recorrido de un TDA Vector;
- lectura y escritura de archivos binarios;
- baja logica;
- `indice_eliminar_ALU`;
- cuidado al eliminar mientras se recorre un vector;
- consistencia entre archivo e indice.

### Condiciones de correccion

- Abrir el archivo en modo `r+b`.
- No incrementar el indice mientras se elimina.
- Resolver correctamente el corrimiento producido por
  `indice_eliminar_ALU`.
- Si una escritura falla, no eliminar el elemento del indice y devolver un
  resultado de error.
- Los titulos con stock positivo deben permanecer sin cambios.

### Pregunta tipica del docente

¿Por que no conviene incrementar el indice despues de eliminar un elemento?
Explicar que el siguiente elemento ocupa la posicion actual por el
desplazamiento interno del vector.

## Ejercicio 3 - Reporte de socios con cuota vencida ordenado por fecha

Agregar una opcion **Listar socios con cuota vencida**.

### Requerimiento

Recorrer los socios activos usando el indice, leer cada registro y guardar en
un vector auxiliar los socios cuya cuota no esta al dia para la fecha de
proceso.

Ordenar el vector auxiliar por:

1. fecha de ultima cuota ascendente;
2. DNI ascendente en caso de empate.

Mostrar DNI, apellido y nombre, plan, fecha de ultima cuota y cantidad de dias
de atraso.

### Temas que practica

- TDA Vector;
- TDA Fecha;
- `compara_fechas` y `diferencia_dias`;
- ordenamiento de estructuras;
- comparador con segundo criterio;
- capacidad dinamica y control de memoria.

### Condiciones de correccion

- Usar la fecha de proceso, no la fecha del sistema.
- No ordenar el indice original de socios.
- No alterar el orden por DNI del indice.
- Calcular el atraso de forma consistente y no mostrar valores negativos.
- Liberar toda memoria reservada para el vector auxiliar.

### Variantes para repregunta

- Orden descendente por cantidad de dias de atraso.
- Filtrar solo socios de un plan indicado por teclado.
- Mostrar tambien la categoria del socio.

## Ejercicio 4 - Procesamiento de novedades mediante MERGE

Agregar un tipo de novedad al catalogo: **baja masiva por genero**.

### Requerimiento

Leer un archivo de novedades que contiene IDs de titulos ordenados
ascendentemente. El archivo puede tener IDs repetidos y debe procesarse junto
con el indice de titulos mediante MERGE, sin buscar cada ID desde cero.

Para cada ID de novedad:

- si coincide con el ID actual del indice, dar de baja logicamente el titulo;
- si el ID no existe, registrarlo como rechazado;
- si aparece repetido, procesarlo una sola vez;
- conservar en el indice solamente los titulos activos.

### Temas que practica

- algoritmo MERGE;
- dos secuencias ordenadas;
- avance independiente de archivo e indice;
- baja logica;
- lectura y escritura binaria;
- eliminacion del indice;
- manejo de duplicados.

### Condiciones de correccion

- Aprovechar que el indice esta ordenado por ID.
- No hacer una llamada a `indice_buscar_ALU` por cada novedad como solucion
  principal.
- No desordenar el indice.
- Mantener sincronizados archivo e indice.
- Informar cantidad de bajas y cantidad de IDs inexistentes.

### Pregunta tipica del docente

¿Que cambia si el archivo de novedades no esta ordenado? Explicar que antes
de aplicar MERGE habria que ordenarlo o utilizar otra estrategia, porque MERGE
requiere dos secuencias ordenadas.

## Ejercicio 5 - Reporte integrado de consumo por plan y genero

Agregar una opcion **Reporte de disponibilidad y consumo**.

### Requerimiento

Construir un reporte que muestre, para cada combinacion de plan y genero:

- cantidad de alquileres registrados en `t_matriz_consumo`;
- cantidad de titulos activos de ese genero;
- stock total disponible de ese genero;
- porcentaje de participacion de cada plan dentro del total de alquileres.

El reporte debe recorrer los titulos usando el indice, acumular los datos en
estructuras auxiliares y mostrar totales por fila y por columna.

### Temas que practica

- matriz de consumo;
- vectores o arreglos auxiliares;
- indice de titulos;
- TDA Fecha para imprimir la fecha de proceso;
- porcentajes y acumuladores;
- ordenamiento opcional de generos por stock total;
- integracion de varias estructuras del TP.

### Condiciones de correccion

- Contar solamente titulos activos presentes en el indice.
- No modificar la matriz de consumo.
- Evitar division por cero cuando no haya alquileres.
- Usar tipos suficientemente grandes para los totales.
- Separar el calculo de los datos de la impresion.

### Variante avanzada

Mostrar primero los tres generos con mayor stock total. Para ello, copiar los
acumuladores a un vector auxiliar y ordenarlo sin modificar el arreglo
`GENEROS`.

## Orden recomendado de resolucion

1. Ejercicio 1: busqueda puntual.
2. Ejercicio 2: baja y corrimiento del indice.
3. Ejercicio 3: vector auxiliar, fechas y ordenamiento.
4. Ejercicio 4: MERGE y consistencia entre archivo e indice.
5. Ejercicio 5: integracion y acumuladores.

## Checklist antes de dar cada ejercicio por terminado

- ¿La funcion valida punteros y entradas?
- ¿Usa el indice cuando corresponde?
- ¿Lee el registro correcto mediante `nro_reg`?
- ¿Controla errores de `fopen`, lectura, escritura y memoria?
- ¿Mantiene el indice ordenado?
- ¿Mantiene coherentes archivo e indice?
- ¿Funciona con cero elementos?
- ¿Funciona cuando no encuentra la clave?
- ¿Probaste al menos un caso de borde?
- ¿La nueva opcion esta conectada al menu y compila con `-Wall -Wextra`?

## Forma de simular la defensa

Para cada ejercicio, intentar trabajar con este limite:

- 5 minutos para explicar la solucion;
- 25 minutos para implementarla;
- 10 minutos para probar casos normales y casos de error.

Al terminar, explicar de memoria:

1. cual es la clave utilizada;
2. donde esta el registro fisicamente;
3. que posicion ocupa dentro del indice;
4. que pasa con el indice despues de una baja;
5. por que el algoritmo elegido conserva el orden.
