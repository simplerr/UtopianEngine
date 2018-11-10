actor_list = 
{
	
	{
		actor_name = "Castle",
		components = 
		{
			CLight = 
			{
				att_x = 0,
				att_y = 0,
				att_z = 1.9999999878451e-08,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				dir_x = 1,
				dir_y = 0,
				dir_z = 0,
				intensity_x = 0.20000000298023,
				intensity_y = 0,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 2,
			},
			CStaticMesh = 
			{
				path = "data/models/sponza_lowres/sponza.obj",
			},
			CTransform = 
			{
				pos_x = 0,
				pos_y = 0,
				pos_z = 0,
				rotation_x = 180,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
	},
	
	{
		actor_name = "Teapot",
		components = 
		{
			CLight = 
			{
				att_x = 1,
				att_y = 0,
				att_z = 1.9999999878451e-08,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				dir_x = 1,
				dir_y = 0,
				dir_z = 0,
				intensity_x = 0,
				intensity_y = 1,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 2,
			},
			CStaticMesh = 
			{
				path = "data/models/teapot.obj",
			},
			CTransform = 
			{
				pos_x = -400,
				pos_y = 50,
				pos_z = 0,
				rotation_x = 0,
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
		actor_name = "Camera",
		components = 
		{
			CCamera = 
			{
				far_Plane = 256000,
				fov = 60,
				look_at_x = -400,
				look_at_y = 50,
				look_at_z = 0,
				near_Plane = 10,
			},
			CNoClip = 
			{
				speed = 6,
			},
			CPlayerControl = 
			{
				empty = 0,
			},
			CTransform = 
			{
				pos_x = 400,
				pos_y = 400,
				pos_z = 0,
				rotation_x = 0,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
	},
}



