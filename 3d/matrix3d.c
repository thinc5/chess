#include "matrix3d.h"
#include "../core/log.h"

#include <math.h>

#define M_PI 3.14159265358979323846

void view_matrix(Matrix matrix)
{
	for (size_t matrix_index = 0; matrix_index < MATRIX_SIZE;
	     matrix_index++) {
		DEBUG_LOG("%.4f %.4f %.4f %.4f\n",
			  matrix[0][matrix_index],
			  matrix[1][matrix_index],
			  matrix[2][matrix_index],
			  matrix[3][matrix_index]);
	}
}

void matrix_mul(Matrix matrix, Vector3D src_vec, Vector3D dest_vec)
{
	// Perform initial multiplication.
	for (size_t point = 0; point < VECTOR_3D_NUM; point++) {
		float result = (src_vec[VECTOR_3D_X] * matrix[point][0]) +
			       (src_vec[VECTOR_3D_Y] * matrix[point][1]) +
			       (src_vec[VECTOR_3D_Z] * matrix[point][2]) +
			       matrix[point][3];
		dest_vec[point] = result;
	}

	float w = (src_vec[VECTOR_3D_X] * matrix[0][3]) +
		  (src_vec[VECTOR_3D_Y] * matrix[1][3]) +
		  (src_vec[VECTOR_3D_Z] * matrix[2][3]) + matrix[3][3];

	// If we are not dealing with a w value of zero, we can divide our points by w.
	if (w != 0.0f) {
		for (size_t point = 0; point < VECTOR_3D_NUM; point++) {
			dest_vec[point] = dest_vec[point] / w;
		}
	}
}

void matrix_create_projection(Matrix matrix, float near, float far, float fov,
			      float aspectRatio)
{
	// Calculate fov radians.
	float fovRad = 1.0f / tanf(fov * 0.5f * (float)M_PI / 180.f);
	float fovRandAspect = aspectRatio * fovRad;

	matrix[0][0] = fovRandAspect;
	matrix[1][1] = fovRad;
	matrix[2][2] = far / (far - near);
	matrix[3][2] = (-far * near) / (far - near);
	matrix[2][3] = 1.0f;
	matrix[3][3] = 0.0f;
}

// X rotation.
void matrix_create_rotation_x(Matrix matrix, float theta)
{
	matrix[0][0] = 1;
	matrix[1][1] = cosf(theta * 0.5f);
	matrix[1][2] = sinf(theta * 0.5f);
	matrix[2][1] = -sinf(theta * 0.5f);
	matrix[2][2] = cosf(theta * 0.5f);
	matrix[3][3] = 1;
}

// Y rotation.
void matrix_create_rotation_y(Matrix matrix, float theta)
{
	matrix[0][0] = cosf(theta * 0.5f);
	matrix[0][2] = -sinf(theta * 0.5f);
	matrix[1][1] = 1;
	matrix[2][0] = sinf(theta * 0.5f);
	matrix[2][2] = cosf(theta * 0.5f);
	matrix[3][3] = 1;
}

// Z rotation.
void matrix_create_rotation_z(Matrix matrix, float theta)
{
	matrix[0][0] = cosf(theta);
	matrix[0][1] = sinf(theta);
	matrix[1][0] = -sinf(theta);
	matrix[1][1] = cosf(theta);
	matrix[2][2] = 1;
	matrix[3][3] = 1;
}
