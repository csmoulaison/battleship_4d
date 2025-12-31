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

// Christian didn't ever do anything but move. He wins every time.

#include "game/grid.cpp"
#include "game/helpers.cpp"
#include "game/voxel_sort.cpp"

#define MENU_ITEMS_LEN 5
const char* menu_strings[MENU_ITEMS_LEN] = {
	"Resume",
	"Submarine",
	"Bombergramps",
	"Sandbox",
	"Quit"
};

enum class GameState {
	Menu,
	Session
};

enum class GameType {
	Submarine,
	Bomber,
	Sandbox
};

enum SubmarineActionType {
	SUBMARINE_ACTION_MOVE = 0,
	SUBMARINE_ACTION_QUERY,
	SUBMARINE_ACTION_FIRE,
	NUM_SUBMARINE_ACTIONS
};

struct Submarine {
	bool game_won;
	i32 turn;
	bool interstitial;

	i32 action_type;
	i32 ship_indices[2];
	i32 query_axis;

	i32 previous_action_type;
	i32 previous_action_index;
	i32 previous_query_axis;
};

struct Bomber {
};

struct Sandbox {
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

	Windowing::ButtonHandle up_button;
	Windowing::ButtonHandle down_button;
	Windowing::ButtonHandle left_button;
	Windowing::ButtonHandle right_button;
	Windowing::ButtonHandle forward_button;
	Windowing::ButtonHandle back_button;

	Windowing::ButtonHandle pitch_up_button;
	Windowing::ButtonHandle pitch_down_button;
	Windowing::ButtonHandle yaw_up_button;
	Windowing::ButtonHandle yaw_down_button;

	Windowing::ButtonHandle quit_button;
	Windowing::ButtonHandle cycle_button;
	Windowing::ButtonHandle modify_button;
	Windowing::ButtonHandle action_button;

	// Menu
	i32 menu_selection;
	float menu_activations[MENU_ITEMS_LEN];
	float menu_flashes[MENU_ITEMS_LEN];

	// Game control
	i32 selection_index;

	// Game state
	GameType game_type;
	// NOW: union switch to either static allocated array or arena allocation (need game scope arena)
	union {
		Submarine submarine;
		Bomber bomber;
		Sandbox sandbox;
	};

	// Camera
	f32 camera_phi;
	f32 camera_theta;
	f32 camera_distance;
	f32 camera_target_distance;

	// Cubes
	f32 cube_color_targets[GRID_VOLUME][4];
	f32 cube_colors[GRID_VOLUME][4];
	f32 cube_idle_positions[GRID_VOLUME][3];
	f32 cube_idle_orientations[GRID_VOLUME][3];
};

#include "game/submarine.cpp"
#include "game/bomber.cpp"
#include "game/sandbox.cpp"

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

	game->down_button = Windowing::register_key(window, Windowing::Keycode::Q);
	game->up_button = Windowing::register_key(window, Windowing::Keycode::E);
	game->left_button = Windowing::register_key(window, Windowing::Keycode::A);
	game->right_button = Windowing::register_key(window, Windowing::Keycode::D);
	game->forward_button = Windowing::register_key(window, Windowing::Keycode::W);
	game->back_button = Windowing::register_key(window, Windowing::Keycode::S);

	game->pitch_up_button = Windowing::register_key(window, Windowing::Keycode::Up);
	game->pitch_down_button = Windowing::register_key(window, Windowing::Keycode::Down);
	game->yaw_up_button = Windowing::register_key(window, Windowing::Keycode::Left);
	game->yaw_down_button = Windowing::register_key(window, Windowing::Keycode::Right);

	game->quit_button = Windowing::register_key(window, Windowing::Keycode::Escape);
	game->cycle_button = Windowing::register_key(window, Windowing::Keycode::Tab);
	game->modify_button = Windowing::register_key(window, Windowing::Keycode::Space);
	game->action_button = Windowing::register_key(window, Windowing::Keycode::Enter);

	game->menu_selection = 0;
	for(i32 i = 0; i > MENU_ITEMS_LEN; i++) {
		game->menu_activations[i] = 0;
		game->menu_flashes[i] = 0;
	}

	game->selection_index = 0;

	game->camera_phi = 1.1f;
	game->camera_theta = 1.2f;
	game->camera_distance = 2.5f * GRID_LENGTH;
	game->camera_target_distance = 1.0f;

	for(i32 i = 0; i < GRID_VOLUME; i++) {
		game->cube_color_targets[i][0] = 0.9f;
		game->cube_color_targets[i][1] = 0.9f;
		game->cube_color_targets[i][2] = 0.9f;
		game->cube_color_targets[i][3] = 0.0f;
		game->cube_colors[i][0] = 0.9f;
		game->cube_colors[i][1] = 0.9f;
		game->cube_colors[i][2] = 0.9f;
		game->cube_colors[i][3] = 0.0f;

		game->cube_idle_positions[i][0] = random_f32() * 20.0f - 10.0f;
		game->cube_idle_positions[i][1] = random_f32() * 20.0f - 10.0f;
		game->cube_idle_positions[i][2] = random_f32() * 20.0f - 10.0f;

		game->cube_idle_orientations[i][0] = random_f32() * 1.0f;
		game->cube_idle_orientations[i][1] = random_f32() * 1.0f;
		game->cube_idle_orientations[i][2] = random_f32() * 1.0f;
	}

	switch(game->game_type) {
		case GameType::Submarine:
			submarine_init(&game->submarine);
			break;
		case GameType::Bomber:
			bomber_init(&game->bomber);
			break;
		case GameType::Sandbox:
			sandbox_init(&game->sandbox);
			break;
		default: break;
	};

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
				submarine_init(&game->submarine);
				game->state = GameState::Session;
				break;
			case 2:
				bomber_init(&game->bomber);
				game->state = GameState::Session;
				break;
			case 3:
				sandbox_init(&game->sandbox);
				game->state = GameState::Session;
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
	// Camera control
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

	switch(game->game_type) {
		case GameType::Submarine:
			submarine_update(game, window);
			break;
		case GameType::Bomber:
			bomber_update(game, window);
			break;
		case GameType::Sandbox:
			sandbox_update(game, window);
			break;
		default: break;
	};

	// Menu transition and control
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

	i32 render_index_map[GRID_VOLUME];
	sort_voxels(render_index_map, renderer->current_state.camera_position);
	
	for(i32 i = 0; i < GRID_VOLUME; i++) {
		Render::Cube* cube = &renderer->current_state.cubes[i];

		i32 render_pos[3];
		i32 cube_index = render_index_map[i];
		grid_position_from_index(cube_index, render_pos);

		cube->position[0] = (-0.5f + 0.5f * GRID_LENGTH) * -dist + (f32)render_pos[0] * dist;
		cube->position[1] = (-0.5f + 0.5f * GRID_LENGTH) * -dist + (f32)render_pos[1] * dist;
		cube->position[2] = (-0.5f + 0.5f * GRID_LENGTH) * -dist + (f32)render_pos[2] * dist;

		cube->position[0] = lerp(cube->position[0], game->cube_idle_positions[cube_index][0], smooth_t);
		cube->position[1] = lerp(cube->position[1], game->cube_idle_positions[cube_index][1], smooth_t);
		cube->position[2] = lerp(cube->position[2], game->cube_idle_positions[cube_index][2], smooth_t);

		cube->orientation[0] = 0.0f;
		cube->orientation[1] = 0.0f;
		cube->orientation[2] = 0.0f;

		cube->orientation[0] = lerp(cube->orientation[0], game->cube_idle_orientations[cube_index][0], smooth_t);
		cube->orientation[1] = lerp(cube->orientation[1], game->cube_idle_orientations[cube_index][1], smooth_t);
		cube->orientation[2] = lerp(cube->orientation[2], game->cube_idle_orientations[cube_index][2], smooth_t);

		f32* color_target = game->cube_color_targets[cube_index];
		color_target[0] = 0.6f;
		color_target[1] = 0.6f;
		color_target[2] = 0.8f;
		color_target[3] = 0.0f + (v3_distance(cube->position, renderer->current_state.camera_position) - game->camera_distance / 2.0f) * 0.008f - smooth_t * 0.1f;

		switch(game->game_type) {
			case GameType::Submarine:
				submarine_color_cube(game, cube_index, render_pos, color_target);
				break;
			case GameType::Bomber:
				bomber_color_cube(game, cube_index, render_pos, color_target);
				break;
			case GameType::Sandbox:
				sandbox_color_cube(game, cube_index, render_pos, color_target);
				break;
			default: break;
		};

		f32* color = game->cube_colors[cube_index];
		color[0] = lerp(color[0], color_target[0], BASE_FRAME_LENGTH * VOXEL_COLOR_SPEED);
		color[1] = lerp(color[1], color_target[1], BASE_FRAME_LENGTH * VOXEL_COLOR_SPEED);
		color[2] = lerp(color[2], color_target[2], BASE_FRAME_LENGTH * VOXEL_COLOR_SPEED);
		color[3] = lerp(color[3], color_target[3], BASE_FRAME_LENGTH * VOXEL_COLOR_SPEED);

		cube->color[0] = color[0];
		cube->color[1] = color[1];
		cube->color[2] = color[2];
		cube->color[3] = color[3];
	}

	Render::Cube* c = &renderer->current_state.cubes[GRID_VOLUME];
	c->orientation[0] = 0.0f;
	c->orientation[1] = 0.0f;
	c->orientation[2] = 0.0f;
	c->position[0] = dist * 1.5f;
	c->position[1] = -dist * 1.5f;
	c->position[2] = dist * 1.5f;
	c->color[0] = 0.0f;
	c->color[1] = 0.0f;
	c->color[2] = 0.0f;
	c->color[3] = 0.5f;

	renderer->current_state.cubes_len = GRID_VOLUME + 1;

	renderer->current_state.clear_color[0] = 0.9f;
	renderer->current_state.clear_color[1] = 0.9f;
	renderer->current_state.clear_color[2] = 0.9f;

	switch(game->game_type) {
		case GameType::Submarine:
			submarine_render(game, renderer, window);
			break;
		case GameType::Bomber:
			bomber_render(game, renderer, window);
			break;
		case GameType::Sandbox:
			sandbox_render(game, renderer, window);
			break;
		default: break;
	};
}

bool game_close_requested(Game* game)
{
	return game->close_requested;
}
