#pragma once

#include <vector>
#include "vulkan/GBufferEffect.h"
#include "vulkan/DeferredEffect.h"

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;
	class BaseRenderer;

	namespace Vk
	{
		class Image;
		class Renderer;
		class RenderTarget;
		class BasicRenderTarget;
	}

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 eyePos;
	};

	struct RendererInput
	{
		RendererInput(const SceneInfo& sceneInfo, const std::vector<BaseRenderer*>& renderers) 
			: sceneInfo(sceneInfo), renderers(renderers) {

		}

		const SceneInfo& sceneInfo;
		const std::vector<BaseRenderer*>& renderers;
	};

	class BaseRenderer
	{
	public:
		virtual ~BaseRenderer() {};
		virtual void Render(Vk::Renderer* renderer, const RendererInput& rendererInput) = 0;
	private:
	};

	class GBufferRenderer : public BaseRenderer
	{
	public:
		GBufferRenderer(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GBufferRenderer();

		void Render(Vk::Renderer* renderer, const RendererInput& rendererInput) override;

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
		DeferredRenderer(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~DeferredRenderer();

		void Render(Vk::Renderer* renderer, const RendererInput& rendererInput) override;

		Vk::BasicRenderTarget* renderTarget;

		Vk::DeferredEffect effect;
	private:
	};
}
