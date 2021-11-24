actor_list = 
{
	
	{
		actor_name = "Player",
		components = 
		{
			CCamera = 
			{
				far_plane = 400,
				fov = 60,
				look_at_x = -2.1985194683075,
				look_at_y = 0.98862808942795,
				look_at_z = 7.6998615264893,
				near_plane = 0.0077999997884035,
			},
			CCatmullSpline = 
			{
				draw_debug = 0,
				filename = "data/camera_spline.txt",
				time_per_segment = 1673.4270019531,
			},
			CNoClip = 
			{
				speed = 10,
			},
			CPlayerControl = 
			{
				jumpStrength = 5,
				maxSpeed = 3,
			},
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateSquare_1.obj",
				render_flags = 0,
			},
			CRigidBody = 
			{
				anisotropic_friciton_x = 1,
				anisotropic_friciton_y = 0,
				anisotropic_friciton_z = 1,
				collisionShapeType = 3,
				friction = 2.9210000038147,
				kinematic = true,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = -4.5765158773747e-08,
				orientation_x = 1,
				orientation_y = 0.00010023208596976,
				orientation_z = 1.1216277906456e-11,
				pos_x = 20.629434585571,
				pos_y = 3.2499997615814,
				pos_z = 6.018753528595,
				scale_x = 2.4999997615814,
				scale_y = 2.4999997615814,
				scale_z = 2.4999997615814,
			},
		},
		scene_layer = 0,
	}, 
	[0] = 
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
				pos_x = 7.698205947876,
				pos_y = 3.7239489555359,
				pos_z = 31.354467391968,
				scale_x = 1.2000000476837,
				scale_y = 1.2000000476837,
				scale_z = 1.2000000476837,
			},
		},
		scene_layer = 0,
	},
}



