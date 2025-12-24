// NOW: we need to stop hardcoding these numbers everywhere. Do it NOWWWWW
i32 grid_index_from_xyz(i32* xyz) {
	return xyz[2] * GRID_LENGTH * GRID_LENGTH + xyz[1] * GRID_LENGTH + xyz[0];
}

void grid_xyz_from_index(i32 i, i32* xyz) {
	xyz[0] = i % GRID_LENGTH;
	xyz[1] = i / GRID_LENGTH % GRID_LENGTH;
	xyz[2] = i / (GRID_LENGTH * GRID_LENGTH);
}
