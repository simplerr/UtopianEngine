#pragma once

#include <vector>
#include "vulkan/GBufferEffect.h"

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;

	namespace Vk
	{
		class Image;
		class Renderer;
		class RenderTarget;
	}

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	class BaseRenderer
	{
	public:
		virtual ~BaseRenderer() {};
		virtual void render(Vk::Renderer* renderer, const SceneInfo& sceneInfo) = 0;
	private:
	};

	class GBufferRenderer : public BaseRenderer
	{
	public:
		GBufferRenderer(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GBufferRenderer();

		void render(Vk::Renderer* renderer, const SceneInfo& sceneInfo) override;

		Vk::Image* positionImage;
		Vk::Image* normalImage;
		Vk::Image* albedoImage;
		Vk::Image* depthImage;
		Vk::RenderTarget* renderTarget;

		Vk::GBufferEffect mGBufferEffect;
	private:
	};

	class DeferredRenderer : public BaseRenderer
	{
	public:
		DeferredRenderer(Vk::Renderer* renderer);
		~DeferredRenderer();

		void render(Vk::Renderer* renderer, const SceneInfo& sceneInfo) override;
	private:
	};
}
