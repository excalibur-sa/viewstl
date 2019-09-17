#ifndef VIEWSTL_STL_H
#define VIEWSTL_STL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STL_TYPE_ASCII 0
#define STL_TYPE_BINARY 1



typedef struct stl_transform_struct {
    float pan_x;
    float pan_y;
    float rot_x;
    float rot_y;
    float scale;
    float orth_scale;
    float z_depth;
} STL_transform;

typedef  struct stl_tri_struct {
    float normal[3];
    float vertex_a[3];
    float vertex_b[3];
    float vertex_c[3];
    uint16_t attr;
} STL_triangle;

typedef struct stl_extents_struct {
    float x_max; float y_max; float z_max;
    float x_min; float y_min; float z_min;
    float ext_max;
} STL_extents;

typedef struct stl_struct {
    uint8_t header[80];
    uint32_t tris_size;
    uint32_t _tris_malloc_size;
    int type;

    STL_triangle* tris;
    STL_extents extents;
    STL_transform transform;
} STL_data;

void readStlAscii(FILE *f, STL_data *stl);
void readStlBinary(FILE *f, STL_data *stl);

#endif //VIEWSTL_STL_H
