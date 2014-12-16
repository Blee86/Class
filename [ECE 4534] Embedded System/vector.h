/*
 Author : Yosub Lee
 Date   : 10/25/2014
 */
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdio.h>
#include <stdlib.h>

#define INIT_SIZE 10

typedef struct vector {
    void ** item;
    int total;
    int size;
} vector;

/* Public API */

void vector_init(vector *vec);
int vector_size(vector *vec);
int vector_capacity(vector *vec);
void vector_add(vector *vec, void *item);
void vector_set_value(vector *vec, int index, void *item);
void* vector_get_value(vector *vec, int index);
void vector_delete(vector *vec, int index);
void vector_free(vector *vec);
#endif