actor_list = 
{
	
	{
		actor_name = "Camera",
		components = 
		{
			CCamera = 
			{
				far_plane = 256000,
				fov = 60,
				look_at_x = -400,
				look_at_y = 50,
				look_at_z = 0,
				near_plane = 10,
			},
			CNoClip = 
			{
				speed = 16,
			},
			CPlayerControl = 
			{
				empty = 0,
			},
			CTransform = 
			{
				pos_x = 1376.6258544922,
				pos_y = 140.31463623047,
				pos_z = -44.312461853027,
				rotation_x = 0,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CLight = 
			{
				att_x = 0.57800000905991,
				att_y = 0,
				att_z = 0,
				color_b = 0.7593954205513,
				color_g = 0.90262448787689,
				color_r = 0.91666668653488,
				dir_x = -1,
				dir_y = 1,
				dir_z = 1,
				intensity_x = 0,
				intensity_y = 0.78300005197525,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 0,
			},
			CRenderable = 
			{
				path = "data/models/teapot.obj",
				render_flags = 2,
			},
			CTransform = 
			{
				pos_x = 1459.3508300781,
				pos_y = 1130.4156494141,
				pos_z = 319.82543945313,
				rotation_x = 182.19999694824,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
	}, 
	[0] = 
	{
		actor_name = "Grid",
		components = 
		{
			CTransform = 
			{
				pos_x = 0,
				pos_y = 0,
				pos_z = 0,
				rotation_x = 0,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 25,
				scale_y = 1,
				scale_z = 25,
			},
		},
	},
}



