// TODO - We'll need to provide a rotation matrix to face the individual triangles away from the camera as well.
void sort_voxels(i32* render_index_map, f32* cam_pos)
{
	i32 grid_length = GRID_LENGTH;
	i32 grid_area = grid_length * grid_length;
	i32 grid_volume = grid_area * grid_length;
	
	// All these variables suffexed "_term" will be 0 or 1, and are used to
	// selectively terms we don't want in the final calculation.
	i32 x_positive_term = 0;
	i32 x_negative_term = 0;
	i32 x_unit_term = 0;
	i32 x_row_term = 0;
	i32 x_slice_term = 0;

	i32 y_positive_term = 0;
	i32 y_negative_term = 0;
	i32 y_unit_term = 0;
	i32 y_row_term = 0;
	i32 y_slice_term = 0;

	i32 z_positive_term = 0;
	i32 z_negative_term = 0;
	i32 z_unit_term = 0;
	i32 z_row_term = 0;
	i32 z_slice_term = 0;

	f32 x_abs = fabs(cam_pos[0]);
	f32 y_abs = fabs(cam_pos[1]);
	f32 z_abs = fabs(cam_pos[2]);

	if(z_abs > y_abs)
	{
		if(z_abs > x_abs)
		{
			z_slice_term = 1;
			if(x_abs > y_abs)
			{
				x_row_term = 1;
				y_unit_term = 1;
			}
			else
			{
				y_row_term = 1;
				x_unit_term = 1;
			}
		}
		else
		{
			x_slice_term = 1;
			z_row_term = 1;
			y_unit_term = 1;
		}
	}
	else // y_abs > z_abs
	{
		printf("z not slicin! z: %f, x: %f\n", z_abs, x_abs);
		if(y_abs > x_abs)
		{
			y_slice_term = 1;
			if(z_abs > x_abs)
			{
				z_row_term = 1;
				x_unit_term = 1;
			}
			else
			{
				x_row_term = 1;
				z_unit_term = 1;
			}
		}
		else
		{
			x_slice_term = 1;
			y_row_term = 1;
			z_unit_term = 1;
		}
	}

	if(cam_pos[0] >= 0)
	{
		x_positive_term = 1;
	}
	else
	{
		x_negative_term = 1;
	}

	if(cam_pos[1] >= 0)
	{
		y_positive_term = 1;
	}
	else
	{
		y_negative_term = 1;
	}

	if(cam_pos[2] >= 0)
	{
		z_positive_term = 1;
	}
	else
	{
		z_negative_term = 1;
	}

	if(1==0) // DBG voxel terms
	{
		printf("\033[2J\033[H");
		printf("x_positive_term: %i\n", x_positive_term);
		printf("x_negative_term: %i\n", x_negative_term);
		printf("x_unit_term:     %i\n", x_unit_term);
		printf("x_row_term:      %i\n", x_row_term);
		printf("x_slice_term:    %i\n", x_slice_term);
		printf("\n");
		printf("y_positive_term: %i\n", y_positive_term);
		printf("y_negative_term: %i\n", y_negative_term);
		printf("y_unit_term:     %i\n", y_unit_term);
		printf("y_row_term:      %i\n", y_row_term);
		printf("y_slice_term:    %i\n", y_slice_term);
		printf("\n");
		printf("z_positive_term: %i\n", z_positive_term);
		printf("z_negative_term: %i\n", z_negative_term);
		printf("z_unit_term:     %i\n", z_unit_term);
		printf("z_row_term:      %i\n", z_row_term);
		printf("z_slice_term:    %i\n", z_slice_term);
	}

	for(i32 i = 0; i < grid_volume; i++)
	{
		i32 unit_index = i % grid_length;
		i32 row_index = (i % grid_area) / grid_length;
		i32 slice_index = i / grid_area;

		i32 x =
			  (unit_index  * x_positive_term + (grid_length - 1 - unit_index)  * x_negative_term) * x_unit_term
			+ (row_index   * x_positive_term + (grid_length - 1 - row_index)   * x_negative_term) * x_row_term
			+ (slice_index * x_positive_term + (grid_length - 1 - slice_index) * x_negative_term) * x_slice_term;
		i32 y =
			  (unit_index  * y_positive_term + (grid_length - 1 - unit_index)  * y_negative_term) * y_unit_term
			+ (row_index   * y_positive_term + (grid_length - 1 - row_index)   * y_negative_term) * y_row_term
			+ (slice_index * y_positive_term + (grid_length - 1 - slice_index) * y_negative_term) * y_slice_term;
		i32 z =
			  (unit_index  * z_positive_term + (grid_length - 1 - unit_index)  * z_negative_term) * z_unit_term
			+ (row_index   * z_positive_term + (grid_length - 1 - row_index)   * z_negative_term) * z_row_term
			+ (slice_index * z_positive_term + (grid_length - 1 - slice_index) * z_negative_term) * z_slice_term;

		render_index_map[i] = z * grid_area + y * grid_length + x;
	}
}
