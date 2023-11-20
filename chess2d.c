#include "core/board.h"
#include "core/game.h"
#include "core/logic.h"
#include "core/display.h"
#include "core/log.h"

#include "2d/config2d.h"
#include "2d/coordinates2d.h"
#include "2d/render2d.h"

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

typedef enum {
	PROGRAM_STATE_RUNNING,
	PROGRAM_STATE_QUIT
} EProgramState;

typedef struct Data {
	EProgramState state;
	bool debug;

	// Renderer
	SDL_Window *window;
	SDL_Renderer *renderer;
	bool full_screen;

	// Resources
	TTF_Font *font;
	SDL_Surface *surface;
	SDL_Texture *tiles;

	uint32_t last_frame;

	ChessGame game;
} Data;

static void render_loop(Data *data)
{
	SDL_RenderClear(data->renderer);

	render_board(data->renderer, data->tiles, data->game.board,
		     data->game.selected_piece, data->game.num_possible_moves,
		     data->game.possible_moves);

	SDL_SetRenderDrawColor(data->renderer, DEFAULT_BACKGROUND_COLOUR);
	SDL_RenderPresent(data->renderer);
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

static void chess_event_loop(Data *data, SDL_Event *event)
{
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
	{
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
		}else{
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
			case SDL_SCANCODE_F10:
			{
				data->full_screen ? SDL_SetWindowFullscreen(
					data->window,
					0) : SDL_SetWindowFullscreen(
					data->window,
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

static void game_loop(Data *data)
{
	init_chess_game(&data->game);
	SDL_ShowWindow(data->window);

	while (data->state == PROGRAM_STATE_RUNNING) {
		event_loop(data);
		render_loop(data);
	}
	SDL_HideWindow(data->window);
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

int main(int argc, char *argv[])
{
	if (!init_modules())
		return 1;
	srand(time(NULL));

	Data data =
	{ .state = PROGRAM_STATE_RUNNING, 0 };
	init_rendering(&data);

	game_loop(&data);

	free_rendering(&data);
	quit_modules();
	return 0;
}
