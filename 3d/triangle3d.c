#include "triangle3d.h"
#include "vector3d.h"

#include <string.h>
#include <math.h>

void view_tri(Triangle3D tri)
{
	for (size_t vec_index = 0; vec_index < VECTOR_3D_NUM; vec_index++) {
		view_vec(tri[vec_index]);
	}
}

void copy_tri(Triangle3D src, Triangle3D dest)
{
	memcpy(dest, src, (sizeof(Triangle3D)));
}

void add_vec_to_try(Triangle3D src, Vector3D add)
{
	for (size_t index = 0; index < TRIANGLE_3D_POINTS; index++) {
		add_vec((Vector3D *)src[index], add);
	}
}

void mul_vec_to_try(Triangle3D src, Vector3D mul)
{
	for (size_t index = 0; index < TRIANGLE_3D_POINTS; index++) {
		mul_vec((Vector3D *)src[index], mul);
	}
}

// Assuming that the first tri is the origin and tris are
// constructed in clockwise order.
void calculate_tri_normal(Triangle3D tri, Vector3D vec)
{
	Vector3D line1 = {
		tri[1][VECTOR_3D_X] - tri[0][VECTOR_3D_X],
		tri[1][VECTOR_3D_Y] - tri[0][VECTOR_3D_Y],
		tri[1][VECTOR_3D_Z] - tri[0][VECTOR_3D_Z]
	};

	Vector3D line2 = {
		tri[2][VECTOR_3D_X] - tri[0][VECTOR_3D_X],
		tri[2][VECTOR_3D_Y] - tri[0][VECTOR_3D_Y],
		tri[2][VECTOR_3D_Z] - tri[0][VECTOR_3D_Z]
	};
	vec[VECTOR_3D_X] = (line1[VECTOR_3D_Y] * line2[VECTOR_3D_Z]) -
			   (line1[VECTOR_3D_Z] * line2[VECTOR_3D_Y]);
	vec[VECTOR_3D_Y] = (line1[VECTOR_3D_Z] * line2[VECTOR_3D_X]) -
			   (line1[VECTOR_3D_X] * line2[VECTOR_3D_Z]);
	vec[VECTOR_3D_Z] = (line1[VECTOR_3D_X] * line2[VECTOR_3D_Y]) -
			   (line1[VECTOR_3D_Y] * line2[VECTOR_3D_X]);

	// Normalize normals.
	float l = sqrtf(
		(vec[VECTOR_3D_X] * vec[VECTOR_3D_X]) +
		(vec[VECTOR_3D_Y] * vec[VECTOR_3D_Y]) +
		(vec[VECTOR_3D_Z] * vec[VECTOR_3D_Z])
		);
	for (size_t vec_index = 0; vec_index < VECTOR_3D_NUM; vec_index++) {
		vec[vec_index] /= l;
	}
}
