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
				speed = 6,
			},
			CPlayerControl = 
			{
				empty = 0,
			},
			CTransform = 
			{
				pos_x = 281.11361694336,
				pos_y = 580.35577392578,
				pos_z = -58.375545501709,
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
				dir_x = -0.88000005483627,
				dir_y = 1,
				dir_z = -1,
				intensity_x = 0.060000002384186,
				intensity_y = 0.10800000280142,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 0,
			},
			CRenderable = 
			{
				path = "data/models/sponza/sponza.obj",
			},
			CTransform = 
			{
				pos_x = -256.17031860352,
				pos_y = 0,
				pos_z = 40.218482971191,
				rotation_x = 180,
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
		actor_name = "Teapot",
		components = 
		{
			CLight = 
			{
				att_x = 0.25300002098083,
				att_y = 0,
				att_z = 1.9999999878451e-08,
				color_b = 0.54164737462997,
				color_g = 0.048058465123177,
				color_r = 0.98039215803146,
				dir_x = 1,
				dir_y = -0.10800000280142,
				dir_z = 0,
				intensity_x = 0.10800000280142,
				intensity_y = 0.38600000739098,
				intensity_z = 0.15500000119209,
				range = 100000,
				spot = 4,
				type = 2,
			},
			CRenderable = 
			{
				path = "data/models/teapot.obj",
			},
			CTransform = 
			{
				pos_x = -577.69158935547,
				pos_y = 50,
				pos_z = 14.482666015625,
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



