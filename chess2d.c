#include "core/board.h"
#include "core/game.h"
#include "core/logic.h"
#include "core/display.h"
#include "core/log.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define DISPLAY_COLOUR_BLACK 0, 0, 0, 255
#define DISPLAY_COLOUR_SELECTED 255, 0, 0, 255
#define DISPLAY_COLOUR_MOVE 0, 0, 0, 200
#define DISPLAY_COLOUR_CAPTURE 255, 0, 0, 200
#define DISPLAY_COLOUR_BLACK_TILE 125, 125, 125, 255
#define DISPLAY_COLOUR_WHITE 255, 255, 255, 255

#define DEFAULT_BACKGROUND_COLOUR DISPLAY_COLOUR_WHITE

#define IMG_WIDTH 360
#define IMG_HEIGHT 120
#define PIECE_WIDTH IMG_WIDTH / 6
#define PIECE_HEIGHT IMG_HEIGHT / 2
#define TILE_DIM (WINDOW_HEIGHT / BOARD_SIZE)
#define X_OFFSET ((WINDOW_WIDTH - (TILE_DIM * BOARD_SIZE)) / 2)

// From stack.
void draw_circle(SDL_Renderer *renderer, size_t centre_x, size_t centre_y,
		 size_t radius)
{
	const size_t diameter = (radius * 2);

	int x = (radius - 1);
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = (tx - diameter);

	while (x >= y) {
		SDL_RenderDrawPoint(renderer, centre_x + x, centre_y - y);
		SDL_RenderDrawPoint(renderer, centre_x + x, centre_y + y);
		SDL_RenderDrawPoint(renderer, centre_x - x, centre_y - y);
		SDL_RenderDrawPoint(renderer, centre_x - x, centre_y + y);
		SDL_RenderDrawPoint(renderer, centre_x + y, centre_y - x);
		SDL_RenderDrawPoint(renderer, centre_x + y, centre_y + x);
		SDL_RenderDrawPoint(renderer, centre_x - y, centre_y - x);
		SDL_RenderDrawPoint(renderer, centre_x - y, centre_y + x);

		if (error <= 0) {
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0) {
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

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

	uint32_t last_frame;

	// Current game.
	ChessGame game;
} Data;

static int screen_to_piece_loc(int x, int y)
{
	int x_loc = (x - X_OFFSET) / TILE_DIM;
	int y_loc = BOARD_SIZE - 1 - (y / TILE_DIM);
	return x_loc + (y_loc * BOARD_SIZE);
}

static SDL_Rect render_piece_loc(PlayPiece piece)
{
	size_t indexes[PIECE_NUM_PIECES] = {
		[PIECE_PAWN] = 5,
		[PIECE_BISHOP] = 4,
		[PIECE_KNIGHT] = 3,
		[PIECE_ROOK] = 2,
		[PIECE_KING] = 1,
		[PIECE_QUEEN] = 0
	};

	return (SDL_Rect) {
		       .x = indexes[piece.type] * PIECE_WIDTH,
		       .y = (piece.colour != COLOUR_WHITE ? 0 : PIECE_HEIGHT),
		       .w = PIECE_WIDTH,
		       .h = PIECE_HEIGHT
	};
}

static void logic_loop(ChessGame *game)
{
	game->check = is_checkmate_for_player(game->board, game->turn,
					      game->move_count,
					      game->check);
	// Is the game over?
	if (game->move_count < 0 && game->check) {
		if (!is_game_over_for_player(game->board,
					     game->next_board,
					     game->turn,
					     game->move_count,
					     game->check)) {
			INFO_LOG("Checkmate! player %d (%s) wins!\n",
				 ((game->turn + 1) % PLAYER_NUM_COLOURS) + 1,
				 PLAYER_COLOUR_STRINGS[(game->turn +
							1) %
						       PLAYER_NUM_COLOURS]);
			return;
		}
		INFO_LOG(
			"%s king in check!\n",
			PLAYER_COLOUR_STRINGS[(game->turn) %
					      PLAYER_NUM_COLOURS]);
	}
	if (!is_game_stalemate(game->board, game->turn,
			       game->move_count)) {
		INFO_LOG("Stalemate! Game ends in draw!\n");
		return;
	}
}

static void chess_event_loop(Data* data, SDL_Event *event)
{
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
		static int x, y;
		SDL_GetMouseState(&x, &y);
		DEBUG_LOG("Click %d %d\n", x, y);
		int clicked_point = screen_to_piece_loc(x, y);

		if (data->game.mode == OPERATION_SELECT) {
			// Select point.
			DEBUG_LOG("Selected: %c%c (%d)\n",
				  INT_TO_COORD(clicked_point), clicked_point);
			if (select_piece_loc(&data->game, clicked_point)) {
				data->game.mode = OPERATION_MOVE;
				DEBUG_LOG("Select success!\n");
			}
		} else {
			// Move piece.
			DEBUG_LOG("Move: %c%c (%d)\n",
				  INT_TO_COORD(clicked_point), clicked_point);
			if (move_piece_loc(&data->game, clicked_point)) {
				toggle_player_turn(&data->game);
				logic_loop(&data->game);
			}
			data->game.mode = OPERATION_SELECT;
			clear_piece_selection(&data->game);
		}
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
			default:
				break;
			}
		default:
			break;
		}
		chess_event_loop(data, &event);
	}
}

static void render_loop(Data *data)
{
	SDL_RenderClear(data->renderer);
	SDL_Rect tile = { .x = 0, .y = 0, .w = TILE_DIM, .h = TILE_DIM };
	for (size_t place = 0; place < BOARD_SIZE * BOARD_SIZE; place++) {
		size_t x = (place % BOARD_SIZE);
		size_t y = (place / BOARD_SIZE);
		size_t inverted_y = (BOARD_SIZE - 1 - (place / BOARD_SIZE));

		size_t even_x = (x + 1) % 2;
		size_t even_y = (y + 1) % 2;

		tile.x = (x * TILE_DIM) + X_OFFSET;
		tile.y = inverted_y * TILE_DIM;

		// We draw white squares when only one of x and y is even,
		// we draw black squares otherwise.
		if ((even_x && !even_y) || (!even_x && even_y)) {
			SDL_SetRenderDrawColor(data->renderer,
					       DISPLAY_COLOUR_BLACK_TILE);
		} else {
			SDL_SetRenderDrawColor(data->renderer,
					       DISPLAY_COLOUR_WHITE);
		}
		SDL_RenderFillRect(data->renderer, &tile);

		// Draw piece.
		PlayPiece piece = data->game.board[place];
		if (piece.type != PIECE_NONE) {
			SDL_Rect texture_loc = render_piece_loc(piece);
			SDL_RenderCopy(data->renderer, data->tiles,
				       &texture_loc, &tile);
		}

		// Draw piece boarder.
		SDL_SetRenderDrawColor(data->renderer, DISPLAY_COLOUR_BLACK);
		SDL_RenderDrawRect(data->renderer, &tile);

		if (data->game.selected_piece != -1) {
			if (place == data->game.selected_piece) {
				SDL_SetRenderDrawColor(data->renderer,
						       DISPLAY_COLOUR_SELECTED);
				SDL_RenderDrawRect(data->renderer, &tile);
			}
		}
	}

	// Draw all of our valid moves.
	for (size_t move_index = 0; move_index < data->game.num_possible_moves;
	     move_index++) {
		PossibleMove *move = &data->game.possible_moves[move_index];
		size_t x = (move->target % BOARD_SIZE);
		size_t y = BOARD_SIZE - 1 - (move->target / BOARD_SIZE);
		tile.x = (x * TILE_DIM) + X_OFFSET;
		tile.y = y * TILE_DIM;
		switch (move->type) {
		case MOVEMENT_PIECE_CAPTURE:
			SDL_SetRenderDrawColor(data->renderer,
					       DISPLAY_COLOUR_SELECTED);
			draw_circle(data->renderer, tile.x + (TILE_DIM / 2),
				    tile.y + (TILE_DIM / 2), TILE_DIM / 4);
			break;
		default:
			SDL_SetRenderDrawColor(data->renderer,
					       DISPLAY_COLOUR_MOVE);
			draw_circle(data->renderer, tile.x + (TILE_DIM / 2),
				    tile.y + (TILE_DIM / 2), TILE_DIM / 4);
			break;
		}
	}


	SDL_Rect boarder =
	{ .x = X_OFFSET, .y = 0, .w = TILE_DIM * BOARD_SIZE,
	  .h = TILE_DIM * BOARD_SIZE };
	SDL_SetRenderDrawColor(data->renderer, DISPLAY_COLOUR_BLACK);
	SDL_RenderDrawRect(data->renderer, &boarder);


	SDL_SetRenderDrawColor(data->renderer, DEFAULT_BACKGROUND_COLOUR);
	SDL_RenderPresent(data->renderer);
}

static void game_loop(Data *data)
{
	uint32_t this_frame = 0;
	init_chess_game(&data->game);
	SDL_ShowWindow(data->window);

	while (data->state == PROGRAM_STATE_RUNNING) {
		event_loop(data);
		render_loop(data);
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

	Data data = { .state = PROGRAM_STATE_RUNNING, 0 };
	init_rendering(&data);

	game_loop(&data);

	free_rendering(&data);
	quit_modules();
	return 0;
}
