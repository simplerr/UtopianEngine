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
				dir_x = 0.70710676908493,
				dir_y = 0.70710676908493,
				dir_z = -0,
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
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateLong_reflective.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 750.86083984375,
				pos_y = 59.161010742188,
				pos_z = -219.58293151855,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "Sponza",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/sponza/sponza.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = -0.0042035197839141,
				orientation_x = 0.99999117851257,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 0,
				pos_y = 0,
				pos_z = -112.08429718018,
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
			CRenderable = 
			{
				color_a = 1,
				color_b = 0,
				color_g = 1,
				color_r = 0,
				path = "data/models/sphere_lowres.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = -0.25387027859688,
				orientation_x = 0.052028991281986,
				orientation_y = -0.034035313874483,
				orientation_z = 0.96523803472519,
				pos_x = 809.93402099609,
				pos_y = 87.239349365234,
				pos_z = -157.49926757813,
				scale_x = 19.999998092651,
				scale_y = 19.999988555908,
				scale_z = 20,
			},
		},
	},
	
	{
		actor_name = "Camera 1",
		components = 
		{
			CCamera = 
			{
				far_plane = 51200,
				fov = 60,
				look_at_x = 121.77127075195,
				look_at_y = -768.64642333984,
				look_at_z = -756.98364257813,
				near_plane = 1,
			},
			CCatmullSpline = 
			{
				draw_debug = 0,
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
				pos_x = 236.3876953125,
				pos_y = 421.77536010742,
				pos_z = -122.07766723633,
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
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/HouseBricksThin_1.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 0.097430571913719,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0.99524229764938,
				pos_x = 117.19053649902,
				pos_y = -6.1823120117188,
				pos_z = -130.04898071289,
				scale_x = 14.499996185303,
				scale_y = 8.4000005722046,
				scale_z = 15.5,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/PlantA.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 1045.8708496094,
				pos_y = -1544.6199951172,
				pos_z = -52.53687286377,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/PlantA.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 997.83081054688,
				pos_y = -1520.8354492188,
				pos_z = -101.49221038818,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/PlantA.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 988.75799560547,
				pos_y = -1527.4547119141,
				pos_z = -169.39596557617,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/PlantA_1.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 994.04364013672,
				pos_y = -1519.5942382813,
				pos_z = -204.61752319336,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateLong_reflective.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 1944.0385742188,
				pos_y = -1832.6387939453,
				pos_z = -75.396484375,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateLong_reflective.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 20672.2578125,
				pos_y = -1410.9807128906,
				pos_z = -259.05453491211,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
	
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/ShopSign_1.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 1012.2759399414,
				pos_y = 174.23913574219,
				pos_z = 4.5795555114746,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	}, 
	[0] = 
	{
		actor_name = "EditorActor",
		components = 
		{
			CRenderable = 
			{
				color_a = 1,
				color_b = 1,
				color_g = 1,
				color_r = 1,
				path = "data/models/adventure_village/CrateLong_reflective.obj",
				render_flags = 1,
			},
			CTransform = 
			{
				orientation_w = 1,
				orientation_x = 0,
				orientation_y = 0,
				orientation_z = 0,
				pos_x = 844.65533447266,
				pos_y = 14.064889907837,
				pos_z = -95.484016418457,
				scale_x = 50,
				scale_y = 50,
				scale_z = 50,
			},
		},
	},
}



