/*
 Author : Yosub Lee
 Date   : 10/25/2014
 
 Mapping for ECE 4514, Embedded Systems Class
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include <stdio.h>
#include <math.h>
#include "vector.h"

typedef struct XYcoordinates {
    int x;
    int y;
} XYcoordinates;

static vector vec;

/*
 Initialize mapping data. (Allocating memory to the vector)
*/
void mapping_init();

/*
 Initialize mapping data with array of xy coordinates
 xyArray    = 1-dimensional array of XY coordinates
 length     = Length of xyArray
 */
void mapping_init_with_array(int* xyArray, int length);
/* 
 Add new coordinate.
 x  : x coordinate
 y  : y coordinate
 */
void mapping_add(int x, int y);

/*
 Delete a point
 x  : x coordinate
 y  : y coordinate
 */
void mapping_delete(int x, int y);

/*
 Print out all points
 
 ! Debug purpose
 */
void mapping_print();


////// API for Wall //////////////
/*
    Check whether it is a wall or not.
    As you know, C does not have a boolean type, unless you includes <stdbool.h>
 
    !! This function works based on added points            !!
    !! so Use after add all initial map x y coordinates!    !!
 
 Return :
    0   - False
    1   - True
 */
int mapping_isWall(int x, int y);

void mapping_connect_points();
#endif
