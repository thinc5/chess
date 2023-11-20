#ifndef _MESH_3D_H
#define _MESH_3D_H

#include "mesh3d.h"
#include "triangle3d.h"

#include <stddef.h>

#define MESH_MAX_TRIANGLES 20000

typedef struct {
	Triangle3D triangles[MESH_MAX_TRIANGLES];
	size_t num_triangles;
} Mesh3D;

Mesh3D read_obj(const char *filename);

Mesh3D init_cube();

#endif
