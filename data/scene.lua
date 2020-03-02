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
				look_at_x = 1279.7807617188,
				look_at_y = -1103.6174316406,
				look_at_z = 820.81854248047,
				near_plane = 1,
			},
			CCatmullSpline = 
			{
				filename = "data/camera_spline.txt",
				time_per_segment = 2187.412109375,
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
				orientation_w = 0.9553365111351,
				orientation_x = -0.29552018642426,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 1262.3218994141,
				pos_y = -1097.8609619141,
				pos_z = 812.06567382813,
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
			CBloomLight = 
			{
				empty = 0,
			},
			CLight = 
			{
				att_x = 0.20000000298023,
				att_y = 0.0013999999500811,
				att_z = 7.0000000960135e-06,
				color_b = 0.99998998641968,
				color_g = 0.99999910593033,
				color_r = 1,
				dir_x = 0.70710676908493,
				dir_y = 0.70710676908493,
				dir_z = 0,
				intensity_x = 0.34900000691414,
				intensity_y = 0.84300005435944,
				intensity_z = 0,
				range = 100000,
				spot = 4,
				type = 0,
			},
			CRenderable = 
			{
				color_a = 8.6999998092651,
				color_b = 0.99998998641968,
				color_g = 0.99999910593033,
				color_r = 1,
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
	},
}



