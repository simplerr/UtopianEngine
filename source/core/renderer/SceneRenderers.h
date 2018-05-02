#pragma once

#include <vector>
#include "vulkan/GBufferEffect.h"
#include "vulkan/DeferredEffect.h"
#include "utility/Common.h"

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
		class ScreenQuad;
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

	struct RenderingSettings
	{
		bool deferredPipeline;
	};

	struct RendererInput
	{
		RendererInput(const SceneInfo& sceneInfo, const std::vector<BaseRenderer*>& renderers, const RenderingSettings& renderingSettings) 
			: sceneInfo(sceneInfo), renderers(renderers) , renderingSettings(renderingSettings) {

		}

		const SceneInfo& sceneInfo;
		const std::vector<BaseRenderer*>& renderers;
		const RenderingSettings& renderingSettings;
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

		SharedPtr<Vk::Image> positionImage;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::Image> albedoImage;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		Vk::GBufferEffect mGBufferEffect;
	private:
	};

	class DeferredRenderer : public BaseRenderer
	{
	public:
		DeferredRenderer(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~DeferredRenderer();

		void Render(Vk::Renderer* renderer, const RendererInput& rendererInput) override;

		SharedPtr<Vk::BasicRenderTarget> renderTarget;

		Vk::DeferredEffect effect;
	private:
		SharedPtr<Vk::ScreenQuad> mScreenQuad;
	};
}
