#include <stdio.h>
#include "mapping.h"

int main() {
    mapping_init();
    
    int corners[] = {-12, -32, 0,-32, 0, -7, 5, -7, 5, -14, 25, -14, 25, 0, 0, 0, 0, 47, -12,47};
    
    mapping_init_with_array(corners, 20);
    mapping_connect_points();
    //mapping_print();
    
    int ret = mapping_isWall(5,-10);
    printf("ret = %d\n", ret);
    return 0;
}
