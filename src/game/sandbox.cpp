void sandbox_init(Sandbox* sandbox) {
	// TODO: implement
}

void sandbox_update(Game* game, Windowing::Context* window) {
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

	// TODO: implement
}

void sandbox_color_cube(Game* game, i32 cube_index, i32* render_position, f32* color) {
	// TODO: implement
}

void sandbox_render(Game* game, Render::Context* renderer, Windowing::Context* window) {
	// TODO: implement
}
