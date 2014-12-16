#include "vtUtilities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lpc17xx_gpio.h"
#include "vector.h"

// Private function
static void vector_resize(vector *vec, int newSize) {
    void **item = realloc(vec->item, sizeof(void *) *newSize);
    
    if (item) {
        vec->item = item;
        vec->total = newSize;
    }
}

// Public function Implementation
void vector_init(vector *vec) {
    vec->total = INIT_SIZE;
    vec->size = 0;
    vec->item = malloc(sizeof(void *) * vec->total);
}


int vector_size(vector *vec) {
    return vec->size;
}

int vector_capacity(vector *vec) {
    return vec->total;
}

void vector_add(vector *vec, void *item) {
    if ( vec->size == vec->total )          // It's full
        vector_resize(vec, 2 * vec->total );
    
    vec->item[vec->size] = item;
    vec->size++;
}

void vector_set_value(vector *vec, int index, void *item) {
    if ( index >= 0 && index < vec->size )  // Index should be less than current size
        vec->item[index] = item;
}

void* vector_get_value(vector *vec, int index){
    if ( index >= 0 && index < vec->size )  // Index should be less than current size
        return vec->item[index];
    else
        return NULL;
}

void vector_delete(vector *vec, int index) {
    if ( index >=0 && index < vec->size ) {
        vec->item[index] = NULL;
        vec->size--;
        
        int i = 0;
        
        // Rearrange
        for ( i = index; i < vec->total - 1; i++ ) {
            vec->item[i] = vec->item[i+1];
            vec->item[i+1] = NULL;
        }
        if (vec->size == vec->total / 4) {
            vector_resize(vec, vec->total/2);
        }
    }
    else {
        return;
    }
}


void vector_free(vector *vec) {
    free(vec->item);
}