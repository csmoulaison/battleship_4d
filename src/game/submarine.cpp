i32 submarine_opponent_turn(Submarine* submarine) {
	if(submarine->turn == 0)
		return 1;
	return 0;
}

void submarine_advance_turn(Game* game) {
	Submarine* sub = &game->submarine;
	sub->turn = submarine_opponent_turn(sub);
	sub->interstitial = true;
	sub->action_type = SUBMARINE_ACTION_MOVE;
	game->selection_index = 0;
}

i32* submarine_player_ship_index(Submarine* submarine) {
	return &submarine->ship_indices[submarine->turn];
}

i32* submarine_opponent_ship_index(Submarine* submarine) {
	return &submarine->ship_indices[submarine_opponent_turn(submarine)];
}

void submarine_init(Submarine* submarine) {
	submarine->game_won = false;
	submarine->turn = 0;
	submarine->interstitial = 0;

	submarine->action_type = SUBMARINE_ACTION_MOVE;
	submarine->ship_indices[0] = (random_f32() * GRID_VOLUME) / GRID_VOLUME;
	submarine->ship_indices[1] = (random_f32() * GRID_VOLUME) / GRID_VOLUME;
	submarine->query_axis = 0;

	submarine->previous_action_type = -1;
	submarine->previous_action_index = 0;
	submarine->previous_query_axis = 0;
}

void submarine_update(Game* game, Windowing::Context* window) {
	Submarine* sub = &game->submarine;

	if(sub->interstitial) {
		if(Windowing::button_pressed(window, game->action_button)) {
			sub->interstitial = false;
		}
		return;
	}
	
	i32* player_ship_index = submarine_player_ship_index(sub);
	i32* opponent_ship_index = submarine_opponent_ship_index(sub);

	// Selection control
	i32 selection_pos[3];
	grid_position_from_index(game->selection_index, selection_pos);

	i8 xmove = 0;
	i8 ymove = 0;
	i8 zmove = 0;

	if(Windowing::button_pressed(window, game->left_button) && selection_pos[0] > 0)
		xmove = -1;
	if(Windowing::button_pressed(window, game->right_button) && selection_pos[0] < GRID_LENGTH - 1)
		xmove = 1;
	if(Windowing::button_pressed(window, game->down_button) && selection_pos[1] > 0)
		ymove = -1;
	if(Windowing::button_pressed(window, game->up_button) && selection_pos[1] < GRID_LENGTH - 1)
		ymove = 1;
	if(Windowing::button_pressed(window, game->forward_button) && selection_pos[2] > 0)
		zmove = -1;
	if(Windowing::button_pressed(window, game->back_button) && selection_pos[2] < GRID_LENGTH - 1)
		zmove = 1;

	selection_pos[0] += xmove;
	selection_pos[1] += ymove;
	selection_pos[2] += zmove;
	game->selection_index = grid_index_from_position(selection_pos);

	if(Windowing::button_pressed(window, game->cycle_button)) {
		sub->action_type++;
		if(sub->action_type >= NUM_SUBMARINE_ACTIONS) {
			sub->action_type = 0;
		}
	}

	if(sub->action_type == SUBMARINE_ACTION_QUERY) {
		if(Windowing::button_pressed(window, game->modify_button)) {
			sub->query_axis++;
			if(sub->query_axis > 2) {
				sub->query_axis = 0;
			}
		}
		if(Windowing::button_pressed(window, game->action_button)) {
			sub->previous_action_type = sub->action_type;
			sub->previous_action_index = *player_ship_index;
			sub->previous_query_axis = sub->query_axis;
			submarine_advance_turn(game);
		}
	} else if(sub->action_type == SUBMARINE_ACTION_MOVE) {
		i32 ship_position[3];
		grid_position_from_index(*player_ship_index, ship_position);
		if(Windowing::button_pressed(window, game->action_button) && grid_eligible_move_position(ship_position, game->selection_index)) {
			*player_ship_index = game->selection_index;
			sub->previous_action_type = sub->action_type;
			sub->previous_action_index = game->selection_index;
			submarine_advance_turn(game);
		}
	} else if(sub->action_type == SUBMARINE_ACTION_FIRE) {
		if(Windowing::button_pressed(window, game->action_button)) {
			if(game->selection_index == *opponent_ship_index) {
				sub->game_won = true;
			} else {
				sub->previous_action_type = sub->action_type;
				sub->previous_action_index = game->selection_index;
				submarine_advance_turn(game);
			}
		}
	}
}

void submarine_color_cube(Game* game, i32 cube_index, i32* render_position, f32* color) {
	Submarine* sub = &game->submarine;

	switch(sub->previous_action_type) {
		case SUBMARINE_ACTION_MOVE:
			break;
		case SUBMARINE_ACTION_QUERY: {
			i32 query_pos[3];
			grid_position_from_index(sub->previous_action_index, query_pos);

			i32 cube_pos[3];
			grid_position_from_index(cube_index, cube_pos);

			i32* player_ship_index = submarine_player_ship_index(sub);
			i32 player_ship_pos[3];
			grid_position_from_index(*player_ship_index, player_ship_pos);

			if(query_pos[sub->previous_query_axis] == cube_pos[sub->previous_query_axis]) {
				if(query_pos[sub->previous_query_axis] == player_ship_pos[sub->previous_query_axis]) {
					color_cube(color, 0.0f, 0.5f, 0.0f, 0.15f);
				} else {
					color_cube(color, 0.5f, 0.2f, 0.0f, 0.15f);
				}
			}
			break;
		}
		case SUBMARINE_ACTION_FIRE:
			if(cube_index == sub->previous_action_index) {
				color_cube(color, 0.5f, 0.0f, 0.0f, 0.15f);
			}
			break;
		default: break;
	}

	if(sub->interstitial) {
		return;
	}
	
	i32* player_ship_index = submarine_player_ship_index(sub);
	i32* opponent_ship_index = submarine_opponent_ship_index(sub);
	
	if(sub->action_type == SUBMARINE_ACTION_MOVE) {
		i32 player_ship_pos[3];
		grid_position_from_index(*player_ship_index, player_ship_pos);

		if(grid_eligible_move_position(player_ship_pos, cube_index))
			color_cube(color, 0.5f, 0.5f, 0.5f, 0.5f);

		if(cube_index == game->selection_index)
			color_cube(color, 0.2f, 0.2f, 0.7f, 0.6f);

		if(cube_index == *player_ship_index)
			color_cube(color, color[0], color[1] - 0.6f, color[2] - 0.6f, color[3] + 0.3f);
	} else if(sub->action_type == SUBMARINE_ACTION_QUERY) {
		i32 player_ship_pos[3];
		grid_position_from_index(*player_ship_index, player_ship_pos);

		if(player_ship_pos[sub->query_axis] == render_position[sub->query_axis]) {
			i32 opponent_ship_pos[3];
			grid_position_from_index(*opponent_ship_index, opponent_ship_pos);
			color_cube(color, 0.8f, 0.8f, 0.2f, 0.5f);
		}

		if(cube_index == *player_ship_index)
			color_cube(color, color[0], color[1] - 0.6f, color[2] - 0.6f, color[3] + 0.3f);
	} else if(sub->action_type == SUBMARINE_ACTION_FIRE) {
		if(cube_index == *player_ship_index)
			color_cube(color, 0.4f, 0.1f, 0.4f, 0.2f);

		if(cube_index == game->selection_index) {
			color_cube(color, 0.7f, 0.2f, 0.2f, 0.6f);
		}
	}
}

void submarine_render(Game* game, Render::Context* renderer, Windowing::Context* window) {
	Submarine* sub = &game->submarine;

	// NOW: refactor from game_draw. copied logic.
	float transition_t = smoothstep(1.0f, 0.0f, game->menu_transition_t);

	if(sub->game_won) {
		char win_string[256];
		sprintf(win_string, "Player %i wins!", sub->turn + 1);

		text_line(renderer, win_string, 
			window->window_width / 2.0f,
			96.0f,
			0.5f, 0.0f, 
			0.4f, 0.4f, 0.4f, 0.8f + sin((f32)game->frames_since_init * 0.1f) * 0.2f,
			FONT_FACE_LARGE);
		return;
	}

	char previous_action_string[256];
	switch(sub->previous_action_type) {
		case SUBMARINE_ACTION_MOVE:
			sprintf(previous_action_string, "Player %i moved. Player %is turn.", submarine_opponent_turn(sub) + 1, sub->turn + 1);
			break;
		case SUBMARINE_ACTION_QUERY:
			sprintf(previous_action_string, "Player %i queried. Player %is turn.", submarine_opponent_turn(sub) + 1, sub->turn + 1);
			break;
		case SUBMARINE_ACTION_FIRE:
			sprintf(previous_action_string, "Player %i fired and missed. Player %is turn.", submarine_opponent_turn(sub) + 1, sub->turn + 1);
			break;
		default:
			sprintf(previous_action_string, "Player %i's turn.", sub->turn + 1);
			break;
	};

	// NOW: refactor from below controls text
	text_line(renderer, previous_action_string, 
		window->window_width + 64.0f - transition_t * 128.0f,
		window->window_height - 64.0f,
		1.0f, 0.0f, 
		0.4f, 0.4f, 0.4f, transition_t,
		FONT_FACE_SMALL);

	if(sub->interstitial) {
		text_line(renderer, "Press [ENTER] to continue...", 
			window->window_width / 2.0f,
			96.0f,
			0.5f, 0.0f, 
			0.4f, 0.4f, 0.4f, 0.8f + sin((f32)game->frames_since_init * 0.1f) * 0.2f,
			FONT_FACE_LARGE);
		return;
	}

	text_line(renderer, "[TAB] Cycle  [SPACE] Modify  [ENTER] Activate", 
		window->window_width + 64.0f - transition_t * 128.0f,
		64.0f,
		1.0f, 0.0f, 
		0.4f, 0.4f, 0.4f, transition_t,
		FONT_FACE_SMALL);
}
