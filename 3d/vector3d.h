#ifndef _VECTOR_3D_H
#define _VECTOR_3D_H

#define VECTOR_3D_X 0
#define VECTOR_3D_Y 1
#define VECTOR_3D_Z 2
#define VECTOR_3D_NUM 3

typedef float Vector3D[VECTOR_3D_NUM];

void view_vec(Vector3D vec);

void copy_vec(Vector3D src, Vector3D dest);

void add_vec(Vector3D *src, Vector3D add);

void mul_vec(Vector3D *src, Vector3D mul);

#endif
