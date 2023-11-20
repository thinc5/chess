#include "vector3d.h"
#include "../core/log.h"

#include <string.h>
#include <math.h>
#include <stdbool.h>

#define M_PI 3.14159265358979323846

void view_vec(Vector3D vec)
{
	DEBUG_LOG("Vec (%p): %.5f %.5f %.5f\n", (void *)vec, vec[VECTOR_3D_X],
		  vec[VECTOR_3D_Y],
		  vec[VECTOR_3D_Z]);
}

void copy_vec(Vector3D src, Vector3D dest)
{
	memcpy(dest, src, sizeof(Vector3D));
}

void add_vec(Vector3D *src, Vector3D add)
{
	for (size_t index = 0; index < VECTOR_3D_NUM; index++) {
		(*src)[index] += add[index];
	}
}

void mul_vec(Vector3D *src, Vector3D mul)
{
	for (size_t index = 0; index < VECTOR_3D_NUM; index++) {
		(*src)[index] *= mul[index];
	}
}
