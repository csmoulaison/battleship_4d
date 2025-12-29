i32 grid_index_from_position(i32* xyz) {
	return xyz[2] * GRID_AREA + xyz[1] * GRID_LENGTH + xyz[0];
}

i32 grid_index_from_xyz(i32 x, i32 y, i32 z) {
	return z * GRID_AREA + y * GRID_LENGTH + x;
}

void grid_position_from_index(i32 i, i32* xyz) {
	xyz[0] = i % GRID_LENGTH;
	xyz[1] = i / GRID_LENGTH % GRID_LENGTH;
	xyz[2] = i / (GRID_AREA);
}

bool grid_eligible_move_position(i32* ship, i32 move_index) {
	return ship[0] > 0               && (move_index == grid_index_from_xyz(ship[0] - 1, ship[1], ship[2])) 
		|| ship[0] < GRID_LENGTH - 1 && (move_index == grid_index_from_xyz(ship[0] + 1, ship[1], ship[2])) 
		|| ship[1] > 0               && (move_index == grid_index_from_xyz(ship[0], ship[1] - 1, ship[2])) 
		|| ship[1] < GRID_LENGTH - 1 && (move_index == grid_index_from_xyz(ship[0], ship[1] + 1, ship[2])) 
		|| ship[2] > 0               && (move_index == grid_index_from_xyz(ship[0], ship[1], ship[2] - 1)) 
		|| ship[2] < GRID_LENGTH - 1 && (move_index == grid_index_from_xyz(ship[0], ship[1], ship[2] + 1));
}
