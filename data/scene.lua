actor_list = 
{
	
	{
		actor_name = "Directional light",
		components = 
		{
			CBloomLight = 
			{
				empty = 0,
			},
			CLight = 
			{
				att_x = 0.20000000298023,
				att_y = 0.0013999999500811,
				att_z = 0,
				color_b = 0.88532823324203,
				color_g = 0.93212097883224,
				color_r = 0.9367088675499,
				dir_x = -0.5735764503479,
				dir_y = 0.81915205717087,
				dir_z = 0,
				intensity_x = 0.34900000691414,
				intensity_y = 0.84300005435944,
				intensity_z = 0.58300000429153,
				range = 100000,
				spot = 4,
				type = 0,
			},
			CRenderable = 
			{
				color_a = 8.6999998092651,
				color_b = 0.88532823324203,
				color_g = 0.93212097883224,
				color_r = 0.9367088675499,
				path = "data/models/teapot.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = -0.019197400659323,
				orientation_x = 0.99981570243835,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -348.98825073242,
				pos_y = -1462.2564697266,
				pos_z = 687.95874023438,
				scale_x = 1.2000000476837,
				scale_y = 1.2000000476837,
				scale_z = 1.2000000476837,
			},
		},
		scene_layer = 0,
	}, 
	[0] = 
	{
		actor_name = "Camera 1",
		components = 
		{
			CCamera = 
			{
				far_plane = 400,
				fov = 60,
				look_at_x = 0,
				look_at_y = 0,
				look_at_z = 0,
				near_plane = 0.0077999997884035,
			},
			CCatmullSpline = 
			{
				draw_debug = 0,
				filename = "data/camera_spline.txt",
				time_per_segment = 2187.412109375,
			},
			CNoClip = 
			{
				speed = 0.03999999910593,
			},
			CPlayerControl = 
			{
				empty = 0,
			},
			CTransform = 
			{
				orientation_w = 0.9553365111351,
				orientation_x = -0.29552018642426,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -20.002490997314,
				pos_y = 8.3596115112305,
				pos_z = 28.593864440918,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
}



