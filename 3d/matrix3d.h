#ifndef _MATRIX_3D_H
#define _MATRIX_3D_H

#include "vector3d.h"

#define MATRIX_SIZE 4

typedef float Matrix[MATRIX_SIZE][MATRIX_SIZE];

void view_matrix(Matrix matrix);

void matrix_mul(Matrix matrix, Vector3D src_vec, Vector3D dest_vec);

void matrix_create_projection(Matrix matrix, float near, float far, float fov,
			      float aspectRatio);

void matrix_create_rotation_x(Matrix matrix, float theta);

void matrix_create_rotation_y(Matrix matrix, float theta);

void matrix_create_rotation_z(Matrix matrix, float theta);

#endif
