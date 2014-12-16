#include "vtUtilities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lpc17xx_gpio.h"
#include "mapping.h"

/*
 Private function API.
 Add points located between two points (x1,y1) (x2, y2)
 */
static void connect_points(int x1, int y1, int x2, int y2) {
    int i = 0;
    int x, y, distance;
    if ( x1 > x2) {         // X-axis case 1
        if ( y1 == y2){
            x = x2 + 1;
            distance = (x1- x2) -1;
            
            for ( i = 0; i < distance; i++) {
                mapping_add(x++, y1);
            }
        }
    }
    else if ( x1 < x2) {    // x-axis case 2
        if ( y1 == y2) {
            x = x1 + 1;
            distance = (x2 - x1) -1;
            
            for ( i = 0; i < distance; i++) {
                mapping_add(x++, y1);
            }
        }
    }
    else {                  // y-axis case
        if ( y1 > y2 ) {
            y = y2 + 1;
            distance = (y1 - y2) - 1;
            
            for ( i = 0; i < distance; i++) {
                mapping_add(x1, y++);
            }
        }
        else if ( y1 < y2) {
            y = y1 + 1;
            distance = (y2 - y1) - 1;
            
            for ( i = 0; i < distance; i++) {
                mapping_add(x1, y++);
            }
        }
    }
}

/*
    Public Function API
 */

void mapping_init() {
    vector_init(&vec);
}

void mapping_init_with_array(int* xyArray, int length) {
    vector_init(&vec);
    
    printf("Length: %d \n", length);
    
    int i;
    
    for (i = 0; i < length; i += 2) {
        mapping_add(xyArray[i], xyArray[i+1]);
    }
    
}

void mapping_add(int x, int y) {
    XYcoordinates* xy = malloc(sizeof(XYcoordinates));
    xy->x = x;
    xy->y = y;
    
    vector_add(&vec, xy);
}

void mapping_connect_points() {
    int numOfPoints = vec.size;
    XYcoordinates* point1;
    XYcoordinates* point2;
    int i = 0;
    for ( i = 0; i < numOfPoints; i ++) {
        point1 = vector_get_value(&vec, i);
        point2 = vector_get_value(&vec, i + 1);

        connect_points(point1->x, point1->y, point2->x, point2->y);
    }
    
}

void mapping_delete(int x, int y) {
    int i;
    
    for ( i =0; i < vec.size; i++) {
        XYcoordinates *ret;
        ret = vector_get_value(&vec, i);
        
        if ( ret->x == x && ret->y == y) {
            vector_delete(&vec, i);
            break;
        }
    }
    
}

void mapping_print(){
    int i;
    XYcoordinates* ret;
    
    printf(" Size: %d / %d\n\n", vec.size, vec.total);
    for (i=0; i< vec.size; i++) {
        ret = vector_get_value(&vec, i);
        printf("[%d]\tX=%d \tY=%d\n", i, ret->x, ret->y);
    }
    
}

int mapping_isWall(int x, int y) {
    int size = vector_size(&vec);
    printf("Size = %d \n", size);
    XYcoordinates* ret;
    int i = 0;
    for (i = 0; i< size; i++ ){
        ret = vector_get_value(&vec, i);
        
        if ( ret->x == x && ret->y == y) {
            return 1;
        }
    }
    
    return 0;
}