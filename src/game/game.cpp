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

#define MENU_ITEMS_LEN 5

enum class GameState {
	Menu,
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
	float state_transition_t;
	float state_transition_speed;

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

	// Menu data
	i32 menu_selection;
	float menu_activations[MENU_ITEMS_LEN];
};

Game* game_init(Windowing::Context* window, Arena* program_arena) 
{
	Game* game = (Game*)arena_alloc(program_arena, sizeof(Game));

	arena_init(&game->persistent_arena, MEGABYTE * 4);
	arena_init(&game->session_arena, MEGABYTE * 4);
	arena_init(&game->frame_arena, MEGABYTE * 4);

	game->state = GameState::Menu;
	game->close_requested = false;
	game->frames_since_init = 0;
	game->state_transition_t = 0;
	game->state_transition_speed = STATE_TRANSITION_SPEED;

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

	game->menu_selection = 0;

	return game;
}

void game_transition_state(Game* game, GameState new_state) {
	game->state = new_state;
	game->state_transition_t = 0.0f;
	game->state_transition_speed = STATE_TRANSITION_SPEED;
}

void menu_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	if(Windowing::button_pressed(window, game->back_button)) game->menu_selection++;
	if(Windowing::button_pressed(window, game->forward_button)) game->menu_selection--;
	if(game->menu_selection >= MENU_ITEMS_LEN) game->menu_selection = 0;
	if(game->menu_selection < 0) game->menu_selection = MENU_ITEMS_LEN - 1;
	
	const char* menu_strings[MENU_ITEMS_LEN] = {
		"Resume",
		"New Game",
		"Controls",
		"Settings",
		"Quit"
	};

	for(i8 i = 0; i < MENU_ITEMS_LEN; i++) {
		float lower = i * 0.1f;
		float transition_t = smoothstep(lower, lower + 0.75f, game->state_transition_t);

		if(game->menu_selection == i) {
			game->menu_activations[i] += MENU_ACTIVATION_SPEED * BASE_FRAME_LENGTH;
			if(game->menu_activations[i] > 1.0f) game->menu_activations[i] = 1.0f;
		} else {
			game->menu_activations[i] -= MENU_ACTIVATION_SPEED * BASE_FRAME_LENGTH;
			if(game->menu_activations[i] < 0.0f) game->menu_activations[i] = 0.0f;
		}
		float activation_t = smoothstep(0.0f, 1.0f, game->menu_activations[i]);

		text_line(renderer, menu_strings[i], 
			96.0f - 64.0f * (1.0f - transition_t) + activation_t * 32.0f,
			window->window_height - 128.0f - i * 64.0f, 
			0.0f, 1.0f, 
			1.0f - activation_t * 0.5f, 1.0f - activation_t * 0.5f, 1.0f, transition_t, 
			FONT_FACE_SMALL);
	}

	if(Windowing::button_pressed(window, game->action_button)) {
		game_transition_state(game, GameState::Session);
	}
}

void session_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	text_line(renderer, "Session?",
		window->window_width / 2.0f, 
		window->window_height / 2.0f, 
		0.5f, 0.5f, 
		1.0f, 1.0f, 1.0f, game->state_transition_t, 
		FONT_FACE_SMALL);

	if(Windowing::button_pressed(window, game->action_button)) {
		game_transition_state(game, GameState::End);
	}
}

void end_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	text_line(renderer, "End...",
		window->window_width / 2.0f, 
		window->window_height / 2.0f, 
		0.5f, 0.5f, 
		1.0f, 1.0f, 1.0f, game->state_transition_t, 
		FONT_FACE_SMALL);

	if(Windowing::button_pressed(window, game->action_button)) {
		game_transition_state(game, GameState::Menu);
	}
}

void game_update(Game* game, Windowing::Context* window, Render::Context* renderer)
{
	game->state_transition_t += game->state_transition_speed * BASE_FRAME_LENGTH;
	
	switch(game->state) {
		case GameState::Menu:
			menu_update(game, window, renderer);
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

	if(Windowing::button_pressed(window, game->quit_button)) {
		game->close_requested = true;
	}
}

bool game_close_requested(Game* game)
{
	return game->close_requested;
}
