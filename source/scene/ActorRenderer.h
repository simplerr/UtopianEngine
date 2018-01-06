#pragma once
#include <vector>
#include "scene/Object.h"
#include "scene/Module.h"
#include "Common.h"
#include "vulkan/PhongEffect.h"

namespace Vulkan
{
	class Camera;
	class Renderer;
	class CommandBuffer;
}

namespace Scene
{
	class Renderable;

	class ActorRenderer : public Module<ActorRenderer>
	{
		struct PushConstantBlock
		{
			glm::mat4 world;
			glm::mat4 worldInvTranspose;
		};

	public:
		ActorRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera);
		~ActorRenderer();

		void RenderAll();

		void AddRenderable(Renderable* renderable);

	private:
		std::vector<Renderable*> mRenderables;
		Vulkan::Renderer* mRenderer;
		Vulkan::Camera* mCamera;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::PhongEffect mPhongEffect;
	};
}
