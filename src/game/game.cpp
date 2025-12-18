/*

Game Design: A turn based shooting game in 4 dimensions, inspired by Battleship.

In the setup phase, you assign a position to your ship in 4 dimensions within a
3x3x3x3 grid. A computer controlled player does the same thing.

On your turn, you can take 1 of 3 actions:
	1. Move your ship in any direction (perhaps the ship continues to move until
	   the direction is changed)
	2. Query a slice of the grid. (maybe different costs for 1, 2, or 3 full 
	   dimensions?)
		- If different costs, maybe you can choose between: 
			2x 1d slice
			1x 2d slice
			1x 3d slice which takes 2 turns. (is this shown before activating? Would be
			quite interesting if it wasn't, to be honest.)
	3. Launch an attack on a single grid space. If the attack hits, you win.

You go back and forth like this with the computer, and each attack/slice is
seen by the other player. The only information which is not shown to both 
players is the position of the ships.
	
*/

enum class GameState {
	Splash,
	Session,
	End
};

struct Game {
	Arena persistent_arena;
	Arena session_arena;
	Arena frame_arena;

	GameState state;
	bool close_requested;
	u32 frames_since_init;

	Windowing::ButtonHandle up_button;
	Windowing::ButtonHandle down_button;
	Windowing::ButtonHandle left_button;
	Windowing::ButtonHandle right_button;
	Windowing::ButtonHandle forward_button;
	Windowing::ButtonHandle back_button;
	Windowing::ButtonHandle ana_button;
	Windowing::ButtonHandle kata_button;
	Windowing::ButtonHandle quit_button;
	Windowing::ButtonHandle action_button;
};

Game* game_init(Windowing::Context* window, Arena* program_arena) 
{
	Game* game = (Game*)arena_alloc(program_arena, sizeof(Game));

	arena_init(&game->persistent_arena, MEGABYTE * 4);
	arena_init(&game->session_arena, MEGABYTE * 4);
	arena_init(&game->frame_arena, MEGABYTE * 4);

	game->state = GameState::Splash;
	game->close_requested = false;
	game->frames_since_init = 0;

	game->up_button = Windowing::register_key(window, Windowing::Keycode::Q);
	game->down_button = Windowing::register_key(window, Windowing::Keycode::E);
	game->left_button = Windowing::register_key(window, Windowing::Keycode::A);
	game->right_button = Windowing::register_key(window, Windowing::Keycode::D);
	game->forward_button = Windowing::register_key(window, Windowing::Keycode::W);
	game->back_button = Windowing::register_key(window, Windowing::Keycode::S);
	game->ana_button = Windowing::register_key(window, Windowing::Keycode::X);
	game->kata_button = Windowing::register_key(window, Windowing::Keycode::Z);
	game->quit_button = Windowing::register_key(window, Windowing::Keycode::Escape);
	game->action_button = Windowing::register_key(window, Windowing::Keycode::Space);

	return game;
}

void splash_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	text_line( renderer, "Splash!", window->window_width / 2.0f, window->window_height / 2.0f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, FONT_FACE_SMALL);

	if(Windowing::button_pressed(window, game->action_button)) {
		game->state = GameState::Session;
	}
}

void session_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	text_line( renderer, "Session?", window->window_width / 2.0f, window->window_height / 2.0f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, FONT_FACE_SMALL);

	if(Windowing::button_pressed(window, game->action_button)) {
		game->state = GameState::End;
	}
}

void end_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	text_line( renderer, "End...", window->window_width / 2.0f, window->window_height / 2.0f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f, FONT_FACE_SMALL);

	if(Windowing::button_pressed(window, game->action_button)) {
		game->state = GameState::Splash;
	}
}

void game_update(Game* game, Windowing::Context* window, Render::Context* renderer)
{
	switch(game->state) {
		case GameState::Splash:
			splash_update(game, window, renderer);
			break;
		case GameState::Session:
			session_update(game, window, renderer);
			break;
		case GameState::End:
			end_update(game, window, renderer);
			break;
		default: break;
	} 
	game->frames_since_init++;
	arena_clear(&game->frame_arena);
}

bool game_close_requested(Game* game)
{
	return game->close_requested;
}
