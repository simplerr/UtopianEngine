actor_list = 
{
	
	{
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-763776444039001202.obj",
				texturePath = "data/textures/prototype/Dark/texture_01.ktx",
			},
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "Unknown",
				render_flags = 1,
			},
			CRigidBody = 
			{
				collisionShapeType = 2,
				friction = 0,
				kinematic = true,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 13.171571731567,
				pos_y = 0.5,
				pos_z = -18.1155834198,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
	
	{
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-11995561671828378483.obj",
				texturePath = "data/textures/prototype/Orange/texture_02.ktx",
			},
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "Unknown",
				render_flags = 1,
			},
			CRigidBody = 
			{
				collisionShapeType = 2,
				friction = 0.49000000953674,
				kinematic = true,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -13.06858921051,
				pos_y = 0.47020196914673,
				pos_z = 10.772416114807,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
	
	{
		actor_name = "Camera 1",
		components = 
		{
			CCamera = 
			{
				far_plane = 400,
				fov = 60,
				look_at_x = 10.188196182251,
				look_at_y = 6.8503322601318,
				look_at_z = 1.9245694875717,
				near_plane = 0.0077999997884035,
			},
			CCatmullSpline = 
			{
				draw_debug = 0,
				filename = "data/camera_spline.txt",
				time_per_segment = 2084.6149902344,
			},
			CNoClip = 
			{
				speed = 0.03999999910593,
			},
			CPlayerControl = 
			{
				empty = 0,
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
				collisionShapeType = 3,
				friction = 3.9860000610352,
				kinematic = false,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = -4.3711388286738e-08,
				orientation_x = 1,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -9.326810836792,
				pos_y = 1.9244334697723,
				pos_z = 12.1351146698,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
	
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
	
	{
		actor_name = "TestBox2",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateSquare_1.obj",
				render_flags = 1,
			},
			CRigidBody = 
			{
				collisionShapeType = 0,
				friction = 0.5,
				kinematic = false,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = 3.1304659842135e-07,
				orientation_x = 0.99766385555267,
				orientation_y = 1.6609119768418e-07,
				orientation_z = -0.068314589560032,
				pos_x = 2.0574481487274,
				pos_y = 0.87340402603149,
				pos_z = 16.465953826904,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	}, 
	[0] = 
	{
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-1152098208628123170.obj",
				texturePath = "data/textures/prototype/Purple/texture_02.ktx",
			},
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "Unknown",
				render_flags = 1,
			},
			CRigidBody = 
			{
				collisionShapeType = 2,
				friction = 0.5,
				kinematic = true,
				mass = 1,
				restitution = 0,
				rollingFriction = 0,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -3.8527290821075,
				pos_y = 0.50011348724365,
				pos_z = 0.64655685424805,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
}



