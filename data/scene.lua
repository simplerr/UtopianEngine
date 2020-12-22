actor_list = 
{
	
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
			CTransform = 
			{
				orientation_w = 0.9553365111351,
				orientation_x = -0.29552018642426,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -11.083020210266,
				pos_y = 3.1401624679565,
				pos_z = -3.3290386199951,
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
				modelPath = "data/models/polymesh/polymesh-14638855888101003123.obj",
				texturePath = "data/textures/prototype/Orange/texture_01.ktx",
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
				orientation_w = 0.92105847597122,
				orientation_x = -0.38942432403564,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -4.0782871246338,
				pos_y = -0.13117778301239,
				pos_z = 13.356126785278,
				scale_x = 1,
				scale_y = 0.99999970197678,
				scale_z = 0.99999970197678,
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
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-6871773098909665999.obj",
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
				pos_x = -1.0689210891724,
				pos_y = 0.48603534698486,
				pos_z = -3.7996497154236,
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
				modelPath = "data/models/polymesh/polymesh-5228511347705790491.obj",
				texturePath = "data/textures/prototype/Orange/texture_01.ktx",
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
				pos_x = -6.3563356399536,
				pos_y = 0.51372909545898,
				pos_z = -6.2250289916992,
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
				modelPath = "data/models/polymesh/polymesh-8127227261953711779.obj",
				texturePath = "data/textures/prototype/Orange/texture_01.ktx",
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
				orientation_w = 0.98021537065506,
				orientation_x = 0,
				orientation_y = -0.19793370366096,
				orientation_z = 0,
				pos_x = 10.000697135925,
				pos_y = 0.53354644775391,
				pos_z = 6.9957842826843,
				scale_x = 0.99999958276749,
				scale_y = 1,
				scale_z = 0.99999958276749,
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
				modelPath = "data/models/polymesh/polymesh-1152098208628123170.obj",
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
				pos_x = 13.171571731567,
				pos_y = 0.53593099117279,
				pos_z = -18.1155834198,
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
				modelPath = "data/models/polymesh/polymesh-6036189481427907133.obj",
				texturePath = "data/textures/prototype/Orange/texture_01.ktx",
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
				pos_x = 3.9103910923004,
				pos_y = 0.47050952911377,
				pos_z = 1.416729927063,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
}



