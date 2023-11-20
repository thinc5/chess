#ifndef _TRIANGLE_3D_H
#define _TRIANGLE_3D_H

#include "vector3d.h"

#include <stddef.h>

#define TRIANGLE_3D_POINTS 3

typedef Vector3D Triangle3D[TRIANGLE_3D_POINTS];

void view_tri(Triangle3D tri);

void copy_tri(Triangle3D src, Triangle3D dest);

void add_vec_to_try(Triangle3D src, Vector3D add);

void mul_vec_to_try(Triangle3D src, Vector3D mul);

void calculate_tri_normal(Triangle3D tri, Vector3D vec);

#endif
