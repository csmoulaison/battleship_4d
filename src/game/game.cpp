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

#include "game/grid.cpp"

#define MENU_ITEMS_LEN 5
const char* menu_strings[MENU_ITEMS_LEN] = {
	"Resume",
	"New Game",
	"Controls",
	"Settings",
	"Quit"
};

enum class GameState {
	Menu,
	Session
};

struct Game {
	Arena persistent_arena;
	Arena session_arena;
	Arena frame_arena;

	GameState state;
	bool close_requested;
	u32 frames_since_init;
	float menu_transition_t;
	float menu_transition_speed;

	// Session
	i32 selection_index;

	Windowing::ButtonHandle up_button;
	Windowing::ButtonHandle down_button;
	Windowing::ButtonHandle left_button;
	Windowing::ButtonHandle right_button;
	Windowing::ButtonHandle forward_button;
	Windowing::ButtonHandle back_button;
	Windowing::ButtonHandle ana_button;
	Windowing::ButtonHandle kata_button;

	Windowing::ButtonHandle pitch_up_button;
	Windowing::ButtonHandle pitch_down_button;
	Windowing::ButtonHandle yaw_up_button;
	Windowing::ButtonHandle yaw_down_button;

	Windowing::ButtonHandle quit_button;
	Windowing::ButtonHandle action_button;

	// Menu data
	i32 menu_selection;
	float menu_activations[MENU_ITEMS_LEN];
	float menu_flashes[MENU_ITEMS_LEN];

	// Camera
	f32 camera_phi;
	f32 camera_theta;
	f32 camera_distance;
	f32 camera_target_distance;

	// Cubes
	f32 cube_idle_positions[27][3];
	f32 cube_idle_orientations[27][3];
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
	game->menu_transition_t = 1;
	game->menu_transition_speed = STATE_TRANSITION_SPEED;

	game->selection_index = 0;

	game->up_button = Windowing::register_key(window, Windowing::Keycode::Q);
	game->down_button = Windowing::register_key(window, Windowing::Keycode::E);
	game->left_button = Windowing::register_key(window, Windowing::Keycode::A);
	game->right_button = Windowing::register_key(window, Windowing::Keycode::D);
	game->forward_button = Windowing::register_key(window, Windowing::Keycode::W);
	game->back_button = Windowing::register_key(window, Windowing::Keycode::S);
	game->ana_button = Windowing::register_key(window, Windowing::Keycode::X);
	game->kata_button = Windowing::register_key(window, Windowing::Keycode::Z);

	game->pitch_up_button = Windowing::register_key(window, Windowing::Keycode::Up);
	game->pitch_down_button = Windowing::register_key(window, Windowing::Keycode::Down);
	game->yaw_up_button = Windowing::register_key(window, Windowing::Keycode::Left);
	game->yaw_down_button = Windowing::register_key(window, Windowing::Keycode::Right);

	game->quit_button = Windowing::register_key(window, Windowing::Keycode::Escape);
	game->action_button = Windowing::register_key(window, Windowing::Keycode::Space);

	game->menu_selection = 0;
	for(i32 i = 0; i > MENU_ITEMS_LEN; i++) {
		game->menu_activations[i] = 0;
		game->menu_flashes[i] = 0;
	}

	game->camera_phi = 1.1f;
	game->camera_theta = 1.2f;
	game->camera_distance = 10.0f;
	game->camera_target_distance = 1.0f;

	for(i32 i = 0; i < 27; i++) {
		game->cube_idle_positions[i][0] = random_f32() * 20.0f - 10.0f;
		game->cube_idle_positions[i][1] = random_f32() * 20.0f - 10.0f;
		game->cube_idle_positions[i][2] = random_f32() * 20.0f - 10.0f;

		game->cube_idle_orientations[i][0] = random_f32() * 1.0f;
		game->cube_idle_orientations[i][1] = random_f32() * 1.0f;
		game->cube_idle_orientations[i][2] = random_f32() * 1.0f;
	}

	return game;
}

void menu_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	game->menu_transition_t += game->menu_transition_speed * BASE_FRAME_LENGTH;
	if(game->menu_transition_t > 1.0f) game->menu_transition_t = 1.0f;

	bool flash = false;
	if(Windowing::button_pressed(window, game->back_button)) {
		game->menu_selection++;
		flash = true;
	}
	if(Windowing::button_pressed(window, game->forward_button)) {
		game->menu_selection--;
		flash = true;
	}
	if(game->menu_selection >= MENU_ITEMS_LEN) game->menu_selection = 0;
	if(game->menu_selection < 0) game->menu_selection = MENU_ITEMS_LEN - 1;

	if(flash) game->menu_flashes[game->menu_selection] = 1.0f;
	
	if(Windowing::button_pressed(window, game->action_button)) {
		switch(game->menu_selection) {
			case 0:
				game->state = GameState::Session;
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				game->close_requested = true;
				break;
			default: 
				panic();
		}
		game->menu_flashes[game->menu_selection] = 2.0f;
	}

	if(Windowing::button_pressed(window, game->quit_button)) {
		game->state = GameState::Session;
	}
}

void session_update(Game* game, Windowing::Context* window, Render::Context* renderer) {
	if(Windowing::button_down(window, game->yaw_up_button))
		game->camera_theta += 5.0f * BASE_FRAME_LENGTH;
	if(Windowing::button_down(window, game->yaw_down_button))
		game->camera_theta -= 5.0f * BASE_FRAME_LENGTH;
	if(Windowing::button_down(window, game->pitch_up_button))
		game->camera_phi -= 5.0f * BASE_FRAME_LENGTH;
	if(Windowing::button_down(window, game->pitch_down_button))
		game->camera_phi += 5.0f * BASE_FRAME_LENGTH;
	if(game->camera_phi < 0.01f)
		game->camera_phi = 0.01f;
	if(game->camera_phi > 3.14f)
		game->camera_phi = 3.14f;
	if(game->camera_theta < 0.0f)
		game->camera_theta += 3.14159f * 2.0f;
	if(game->camera_theta > 3.14159f * 2.0f)
		game->camera_theta -= 3.14159f * 2.0f;
	if(game->camera_theta > 10.0f)
		game->camera_theta = 0.0f;

	i8 xmove = 0;
	i8 ymove = 0;
	i8 zmove = 0;
	if(Windowing::button_pressed(window, game->right_button))
		xmove = 1;
	if(Windowing::button_pressed(window, game->left_button))
		xmove = -1;
	if(Windowing::button_pressed(window, game->up_button))
		ymove = 1;
	if(Windowing::button_pressed(window, game->down_button))
		ymove = -1;
	if(Windowing::button_pressed(window, game->forward_button))
		zmove = -1;
	if(Windowing::button_pressed(window, game->back_button))
		zmove = 1;

	i32 selection_xyz[3];
	grid_xyz_from_index(game->selection_index, selection_xyz);

	selection_xyz[0] += xmove;
	if(selection_xyz[0] < 0) selection_xyz[0] = GRID_LENGTH - 1;
	if(selection_xyz[0] >= GRID_LENGTH) selection_xyz[0] = 0;

	selection_xyz[1] += ymove;
	if(selection_xyz[1] < 0) selection_xyz[1] = GRID_LENGTH - 1;
	if(selection_xyz[1] >= GRID_LENGTH) selection_xyz[1] = 0;

	selection_xyz[2] += zmove;
	if(selection_xyz[2] < 0) selection_xyz[2] = GRID_LENGTH - 1;
	if(selection_xyz[2] >= GRID_LENGTH) selection_xyz[2] = 0;
	game->selection_index = grid_index_from_xyz(selection_xyz);

	game->menu_transition_t -= game->menu_transition_speed * BASE_FRAME_LENGTH;
	if(game->menu_transition_t < 0.0f) game->menu_transition_t = 0.0f;

	if(Windowing::button_pressed(window, game->quit_button)) {
		game->state = GameState::Menu;
	}
}

void game_draw(Game* game, Windowing::Context* window, Render::Context* renderer) {
	for(i8 i = 0; i < MENU_ITEMS_LEN; i++) {
		float lower = i * 0.1f;
		float upper = lower + 1.0f - MENU_ITEMS_LEN * 0.1f;
		float transition_t = smoothstep(lower, lower + 0.5f, game->menu_transition_t);

		if(game->menu_selection == i) {
			game->menu_activations[i] += MENU_ACTIVATION_SPEED * BASE_FRAME_LENGTH;
			if(game->menu_activations[i] > 1.0f) game->menu_activations[i] = 1.0f;
		} else {
			game->menu_activations[i] -= MENU_ACTIVATION_SPEED * BASE_FRAME_LENGTH;
			if(game->menu_activations[i] < 0.0f) game->menu_activations[i] = 0.0f;
		}
		float activation_t = smoothstep(0.0f, 1.0f, game->menu_activations[i]);

		game->menu_flashes[i] -= MENU_FLASH_SPEED * BASE_FRAME_LENGTH;
		if(game->menu_flashes[i] < 0.0f) game->menu_flashes[i] = 0.0f;

		text_line(renderer, menu_strings[i], 
			200.0f - 200.0f * (1.0f - transition_t) + activation_t * 50.0f,
			window->window_height - 200.0f - i * 100.0f, 
			0.0f, 1.0f, 
			0.4f + game->menu_flashes[i] * 0.3f, 0.4f + game->menu_flashes[i] * 0.3f, 0.4f + activation_t * 0.4f + game->menu_flashes[i] * 0.3f, transition_t, 
			FONT_FACE_LARGE);
	}
}

void game_update(Game* game, Windowing::Context* window, Render::Context* renderer)
{
	switch(game->state) {
		case GameState::Menu:
			menu_update(game, window, renderer);
			break;
		case GameState::Session:
			session_update(game, window, renderer);
			break;
		default: break;
	} 
	game_draw(game, window, renderer);
	
	game->frames_since_init++;
	arena_clear(&game->frame_arena);

	renderer->current_state.camera_position[0] = 
		game->camera_distance * sin(game->camera_phi) * cos(game->camera_theta);
	renderer->current_state.camera_position[1] = 
		game->camera_distance * cos(game->camera_phi);
	renderer->current_state.camera_position[2] = 
		game->camera_distance * sin(game->camera_phi) * sin(game->camera_theta);
	renderer->current_state.camera_target[0] = 0.0f;
	renderer->current_state.camera_target[1] = 0.0f;
	renderer->current_state.camera_target[2] = 0.0f;

	f32 smooth_t = smoothstep(0.0f, 1.0f, game->menu_transition_t);
	float dist = 1.5f;
	for(i32 i = 0; i < 27; i++) {
		Render::Cube* cube = &renderer->current_state.cubes[i];

		i32 render_pos[3];
		grid_xyz_from_index(i, render_pos);

		cube->position[0] = -dist + (f32)render_pos[0] * dist;
		cube->position[1] = -dist + (f32)render_pos[1] * dist;
		cube->position[2] = -dist + (f32)render_pos[2] * dist;

		cube->position[0] = lerp(cube->position[0], game->cube_idle_positions[i][0], smooth_t);
		cube->position[1] = lerp(cube->position[1], game->cube_idle_positions[i][1], smooth_t);
		cube->position[2] = lerp(cube->position[2], game->cube_idle_positions[i][2], smooth_t);

		cube->orientation[0] = 0.0f;
		cube->orientation[1] = 0.0f;
		cube->orientation[2] = 0.0f;

		cube->orientation[0] = lerp(cube->orientation[0], game->cube_idle_orientations[i][0], smooth_t);
		cube->orientation[1] = lerp(cube->orientation[1], game->cube_idle_orientations[i][1], smooth_t);
		cube->orientation[2] = lerp(cube->orientation[2], game->cube_idle_orientations[i][2], smooth_t);


		float selected_mod = 0.0f;
		if(i == game->selection_index) selected_mod = 1.0f;

		cube->color[0] = 0.8f + selected_mod * 0.2f;
		cube->color[1] = 0.8f - selected_mod * 0.2f;
		cube->color[2] = 0.8f - selected_mod * 0.2f;
		cube->color[3] = 0.6f - smooth_t * 0.4f + selected_mod * 0.25f;
	}

	renderer->current_state.cubes_len = 27;

	renderer->current_state.clear_color[0] = 0.9f;
	renderer->current_state.clear_color[1] = 0.9f;
	renderer->current_state.clear_color[2] = 0.9f;

}

bool game_close_requested(Game* game)
{
	return game->close_requested;
}
