#include "mesh3d.h"
#include "../core/log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void read_float_obj(char *token, float *dest)
{
	*dest = atof(token);
}

void read_int_obj(char *token, int *dest)
{
	*dest = atoi(token);
}

Mesh3D read_obj(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		ERROR_LOG("File does not exist or cannot be read: %s\n",
			  filename);
		return (Mesh3D) { 0 };
	}
	DEBUG_LOG("Reading obj: %s\n", filename);

	Mesh3D mesh = { 0 };

	size_t max_vectors = MESH_MAX_TRIANGLES * TRIANGLE_3D_POINTS;
	Vector3D vectors[max_vectors];
	size_t vector_num = 0;

	char read = '\0';
	char line[128];
	while (read != EOF) {
		size_t line_pos = 0;
		while ((read = fgetc(file)) != '\n') {
			if (read == EOF) break;
			line[line_pos++] = read;
		}
		line[line_pos] = '\0';
		// DEBUG_LOG("Line (%lu): %s\n", line_pos, line);

		// What are we reading?
		char *token = strtok(line, " ");
		switch (line[0]) {
		// Vector.
		case 'v': {
			// Read each point.
			for (size_t vec_pos = VECTOR_3D_X;
			     vec_pos < VECTOR_3D_NUM; vec_pos++) {
				// Read ahead.
				token = strtok(NULL, " ");
				// Convert to float.
				read_float_obj(token,
					       &vectors[vector_num][vec_pos]);
			}
			vector_num++;
		}
		break;
		// Face.
		case 'f': {
			int faces[6];
			size_t face_index = 0;
			while (token != NULL) {
				token = strtok(NULL, " ");
				if (token == NULL) break;

				read_int_obj(token, &(faces[face_index]));
				faces[face_index]--;

				face_index++;
			}
			for (size_t i = 0; i < TRIANGLE_3D_POINTS; i++) {
				copy_vec(vectors[faces[i]],
					 mesh.triangles[mesh.num_triangles][i]);
			}
			mesh.num_triangles++;
		}
		break;
		// Not supported.
		default:
			break;
		}
	}

	fclose(file);

	return mesh;
}

Mesh3D init_cube()
{
	return (Mesh3D) {
		       .triangles = {
			       // SOUTH
			       {
				       { 0.0f, 0.0f, 0.0f },
				       { 0.0f, 1.0f, 0.0f },
				       { 1.0f, 1.0f, 0.0f },
			       },
			       {
				       { 0.0f, 0.0f, 0.0f },
				       { 1.0f, 1.0f, 0.0f },
				       { 1.0f, 0.0f, 0.0f }
			       },
			       // EAST
			       {
				       { 1.0f, 0.0f, 0.0f },
				       { 1.0f, 1.0f, 0.0f },
				       { 1.0f, 1.0f, 1.0f }
			       },
			       {
				       { 1.0f, 0.0f, 0.0f },
				       { 1.0f, 1.0f, 1.0f },
				       { 1.0f, 0.0f, 1.0f }
			       },
			       // NORTH
			       {
				       { 1.0f, 0.0f, 1.0f },
				       { 1.0f, 1.0f, 1.0f },
				       { 0.0f, 1.0f, 1.0f }
			       },
			       {
				       { 1.0f, 0.0f, 1.0f },
				       { 0.0f, 1.0f, 1.0f },
				       { 0.0f, 0.0f, 1.0f }
			       },
			       // WEST
			       {
				       { 0.0f, 0.0f, 1.0f },
				       { 0.0f, 1.0f, 1.0f },
				       { 0.0f, 1.0f, 0.0f },
			       },
			       {
				       { 0.0f, 0.0f, 1.0f },
				       { 0.0f, 1.0f, 0.0f },
				       { 0.0f, 0.0f, 0.0f },
			       },
			       // TOP
			       {
				       { 0.0f, 1.0f, 0.0f },
				       { 0.0f, 1.0f, 1.0f },
				       { 1.0f, 1.0f, 1.0f }
			       },
			       {
				       { 0.0f, 1.0f, 0.0f },
				       { 1.0f, 1.0f, 1.0f },
				       { 1.0f, 1.0f, 0.0f }
			       },
			       // BOTTOM
			       {
				       { 1.0f, 0.0f, 1.0f },
				       { 0.0f, 0.0f, 1.0f },
				       { 0.0f, 0.0f, 0.0f },
			       },
			       {
				       { 1.0f, 0.0f, 1.0f },
				       { 0.0f, 0.0f, 0.0f },
				       { 1.0f, 0.0f, 0.0f }
			       },
		       },
		       .num_triangles = 12
	};
}
