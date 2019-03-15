actor_list = 
{
	
	{
		actor_name = "Camera",
		components = 
		{
			CCamera = 
			{
				far_plane = 51200,
				fov = 60,
				look_at_x = -400,
				look_at_y = 50,
				look_at_z = 0,
				near_plane = 1,
			},
			CNoClip = 
			{
				speed = 1,
			},
			CPlayerControl = 
			{
				empty = 0,
			},
			CTransform = 
			{
				pos_x = -2719.4907226563,
				pos_y = 193.53289794922,
				pos_z = -3435.5593261719,
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
			CRenderable = 
			{
				path = "data/models/sponza/sponza.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				pos_x = -2885.8703613281,
				pos_y = 0,
				pos_z = -3392.9382324219,
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
		actor_name = "EditorActor",
		components = 
		{
			CLight = 
			{
				att_x = 0.57800000905991,
				att_y = 0,
				att_z = 0,
				color_b = 0.99998998641968,
				color_g = 0.99999910593033,
				color_r = 1,
				dir_x = 0.70710676908493,
				dir_y = 0.70710676908493,
				dir_z = -0,
				intensity_x = 0.096000000834465,
				intensity_y = 1,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 0,
			},
			CRenderable = 
			{
				path = "data/models/teapot.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				pos_x = -1782.9721679688,
				pos_y = 231.34075927734,
				pos_z = -3421.5627441406,
				rotation_x = 182.19999694824,
				rotation_y = 0,
				rotation_z = 0,
				scale_x = 3.6000001430511,
				scale_y = 4.8000001907349,
				scale_z = 3.6000001430511,
			},
		},
	},
}



