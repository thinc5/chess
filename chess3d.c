#include "core/board.h"
#include "core/game.h"
#include "core/logic.h"
#include "core/display.h"
#include "core/log.h"

#include "./3d/vector3d.h"
#include "./3d/triangle3d.h"
#include "./3d/matrix3d.h"
#include "./3d/mesh3d.h"
#include "./3d/render3d.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define ASPECT_RATIO (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH
#define MAX_FPS (1000 / 300)

#define DISPLAY_COLOUR_BLACK 0, 0, 0, 255
#define DISPLAY_COLOUR_SELECTED 255, 0, 0, 255
#define DISPLAY_COLOUR_MOVE 0, 0, 0, 200
#define DISPLAY_COLOUR_CAPTURE 255, 0, 0, 200
#define DISPLAY_COLOUR_BLACK_TILE 125, 125, 125, 255
#define DISPLAY_COLOUR_WHITE 255, 255, 255, 255

#define DEFAULT_BACKGROUND_COLOUR DISPLAY_COLOUR_BLACK

#define IMG_WIDTH 360
#define IMG_HEIGHT 120
#define PIECE_WIDTH IMG_WIDTH / 6
#define PIECE_HEIGHT IMG_HEIGHT / 2
#define TILE_DIM (WINDOW_HEIGHT / BOARD_SIZE)
#define X_OFFSET ((WINDOW_WIDTH - (TILE_DIM * BOARD_SIZE)) / 2)

typedef enum {
	PROGRAM_STATE_RUNNING,
	PROGRAM_STATE_QUIT
} EProgramState;

typedef struct Data {
	EProgramState state;
	bool debug;
	bool full_screen;
	// Renderer
	SDL_Window *window;
	SDL_Renderer *renderer;

	// Resources
	TTF_Font *font;
	SDL_Surface *surface;
	SDL_Texture *tiles;

	Mesh3D obj;

	uint32_t this_frame;
	uint32_t last_frame;

	// Current game.
	ChessGame game;

	// Flags for testing
	bool projection;
	bool rotate_x;
	bool rotate_y;
	bool rotate_z;
	bool cull_normals;
	bool render_frame;
	bool render_fill;
} Data;

static void chess_event_loop(Data* data, SDL_Event *event)
{
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
		static int x, y;
		SDL_GetMouseState(&x, &y);
		DEBUG_LOG("Click %d %d\n", x, y);
		break;
	}
	default:
		break;
	}
}

static void event_loop(Data *data)
{
	static SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Check top level input first
		if (event.type == SDL_QUIT) {
			data->state = PROGRAM_STATE_QUIT;
			return;
		}
		// Rest of the events.
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_F10: {
				data->full_screen ?
				SDL_SetWindowFullscreen(data->window, 0) :
				SDL_SetWindowFullscreen(data->window,
							SDL_WINDOW_FULLSCREEN_DESKTOP);
				data->full_screen = !data->full_screen;
			};
				break;
			case SDL_SCANCODE_P:
				data->projection = !data->projection;
				break;
			case SDL_SCANCODE_X:
				data->rotate_x = !data->rotate_x;
				break;
			case SDL_SCANCODE_Y:
				data->rotate_y = !data->rotate_y;
				break;
			case SDL_SCANCODE_Z:
				data->rotate_z = !data->rotate_z;
				break;
			case SDL_SCANCODE_F:
				data->render_frame = !data->render_frame;
				break;
			case SDL_SCANCODE_L:
				data->render_fill = !data->render_fill;
				break;
			case SDL_SCANCODE_N:
				data->cull_normals = !data->cull_normals;
				break;
			default:
				break;
			}
		default:
			break;
		}
		chess_event_loop(data, &event);
	}
}

static Vector3D scale_vec =
{ 0.5f * (float)WINDOW_WIDTH, 0.5f * (float)WINDOW_HEIGHT, 0.0f };

static Vector3D offset_vec = { 1.0f, 1.0f, 0.0f };

static Vector3D translate_vec = { 0.0f, 0.0f, 5.0f };

static void render_loop(Data *data)
{
	SDL_RenderClear(data->renderer);
	static Matrix projection_matrix;
	static Matrix rotation_matrix_x;
	static Matrix rotation_matrix_y;
	static Matrix rotation_matrix_z;

	static Triangle3D starting_tri;
	static Triangle3D working_tri;

	static Vector3D normals;

	static float theta = 0.0f;
	theta += 1.0f * (float)(data->this_frame / data->last_frame) / 100.0f;

	matrix_create_projection(projection_matrix, 0.01f,
				 100000.0f,
				 90.0f,
				 ASPECT_RATIO);
	matrix_create_rotation_x(rotation_matrix_x, -180.00f);
	matrix_create_rotation_y(rotation_matrix_y, theta);
	matrix_create_rotation_z(rotation_matrix_z, theta);

	SDL_SetRenderDrawColor(data->renderer, DISPLAY_COLOUR_CAPTURE);

	// Need to render each tri separately.
	for (size_t tri_index = 0; tri_index < data->obj.num_triangles;
	     tri_index++) {
		// Copy tri for transformations.
		copy_tri(data->obj.triangles[tri_index], starting_tri);

		// 1. Rotation.
		for (size_t vec_index = 0; vec_index < VECTOR_3D_NUM;
		     vec_index++) {
			if (data->rotate_z) {
				matrix_mul(rotation_matrix_z,
					   starting_tri[vec_index],
					   working_tri[vec_index]);
				copy_vec(working_tri[vec_index],
					 starting_tri[vec_index]);
			}
			if (data->rotate_y) {
				matrix_mul(rotation_matrix_y,
					   starting_tri[vec_index],
					   working_tri[vec_index]);
				copy_vec(working_tri[vec_index],
					 starting_tri[vec_index]);
			}
			if (data->rotate_x) {
				matrix_mul(rotation_matrix_x,
					   starting_tri[vec_index],
					   working_tri[vec_index]);
				copy_vec(working_tri[vec_index],
					 starting_tri[vec_index]);
			}
		}

		// 2. Translation.
		add_vec_to_try(starting_tri, translate_vec);

		// 3. Normals
		if (data->cull_normals) {
			calculate_tri_normal(starting_tri, normals);
			if (normals[VECTOR_3D_Z] > 0.0f)
				continue;
		}

		// 4. Projection.
		if (data->projection) {
			for (size_t vec_index = 0; vec_index < VECTOR_3D_NUM;
			     vec_index++) {
				matrix_mul(projection_matrix,
					   starting_tri[vec_index],
					   working_tri[vec_index]);
			}
			copy_tri(working_tri, starting_tri);
		}

		// 5. Offset (Camera world pos?).
		add_vec_to_try(starting_tri, offset_vec);

		// 6. Scale.
		mul_vec_to_try(starting_tri, scale_vec);

		// Render triangle.
		draw_triangle_frame(data->renderer, starting_tri);

		draw_triangle_fill(data->renderer, starting_tri);
	}

	SDL_SetRenderDrawColor(data->renderer, DEFAULT_BACKGROUND_COLOUR);
	SDL_RenderPresent(data->renderer);
}

static void game_loop(Data *data)
{
	data->this_frame = SDL_GetTicks();
	data->last_frame = data->this_frame;
	init_chess_game(&data->game);
	data->obj = read_obj("al.obj");
	SDL_ShowWindow(data->window);

	while (data->state == PROGRAM_STATE_RUNNING) {
		data->this_frame = SDL_GetTicks();
		event_loop(data);
		render_loop(data);


		// Cap framerate.
		if (data->last_frame > data->this_frame - MAX_FPS)
			SDL_Delay(data->last_frame + MAX_FPS -
				  data->this_frame);

		data->last_frame = data->this_frame;
	}
	SDL_HideWindow(data->window);
}

static void init_rendering(Data *data)
{
	data->window =
		SDL_CreateWindow("TEST",
				 SDL_WINDOWPOS_UNDEFINED,
				 SDL_WINDOWPOS_UNDEFINED,
				 WINDOW_WIDTH, WINDOW_HEIGHT,
				 SDL_WINDOW_HIDDEN);
	data->renderer = SDL_CreateRenderer(data->window, -1,
					    SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(data->renderer, 0, 0, 0, 255);
	SDL_RenderSetLogicalSize(data->renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_SetRenderDrawBlendMode(data->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(data->renderer, DEFAULT_BACKGROUND_COLOUR);

	data->surface = IMG_Load("./piece_tilemap.png");
	data->tiles =
		SDL_CreateTextureFromSurface(data->renderer, data->surface);
	SDL_SetRenderDrawColor(data->renderer, DEFAULT_BACKGROUND_COLOUR);
}

static void free_rendering(Data *data)
{
	SDL_DestroyTexture(data->tiles);
	SDL_FreeSurface(data->surface);
	TTF_CloseFont(data->font);
	SDL_DestroyRenderer(data->renderer);
	SDL_DestroyWindow(data->window);
}

static bool init_modules(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		return false;

	if (TTF_Init() != 0)
		return false;

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		return false;

	return true;
}

static void quit_modules(void)
{
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	SDL_VideoQuit();
	SDL_AudioQuit();
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	if (!init_modules())
		return 1;
	srand(time(NULL));

	Data data =
	{ .state = PROGRAM_STATE_RUNNING, .projection = true,
	  .render_frame = true, 0 };
	init_rendering(&data);

	game_loop(&data);

	free_rendering(&data);
	quit_modules();
	return 0;
}
