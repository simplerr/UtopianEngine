actor_list = 
{
	
	{
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-12512079723111979634.obj",
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
				pos_x = 4.2168407440186,
				pos_y = 0.5,
				pos_z = 4.1357355117798,
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
				modelPath = "data/models/polymesh/polymesh-3951816049023924530.obj",
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
				orientation_w = 0.99176394939423,
				orientation_x = -0.12807916104794,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = -0.99975764751434,
				pos_y = 1.7164533138275,
				pos_z = 3.0855898857117,
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
		actor_name = "Polymesh",
		components = 
		{
			CPolyMesh = 
			{
				modelPath = "data/models/polymesh/polymesh-763776444039001202.obj",
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
				pos_x = 10.752745628357,
				pos_y = 12.838685035706,
				pos_z = -15.013296127319,
				scale_x = 1,
				scale_y = 1,
				scale_z = 1,
			},
		},
		scene_layer = 0,
	},
}



