#pragma once
#include <string>
#include <glm/glm.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/ShaderBuffer.h"

namespace Utopian
{
	class Terrain;

	class TerrainTool
	{
	public:
		UNIFORM_BLOCK_BEGIN(BrushBlock)
			UNIFORM_PARAM(glm::vec2, brushPos)
		UNIFORM_BLOCK_END()

		TerrainTool(const SharedPtr<Terrain>& terrain, Vk::Device* device);
		~TerrainTool();

		void Update();

		void EffectRecompiledCallback(std::string name);

		void SetupBlendmapBrushEffect();
		void SetupHeightmapBrushEffect();

		void RenderBlendmapBrush();
		void RenderHeightmapBrush();
	private:
		Vk::Device* mDevice;
		SharedPtr<Terrain> mTerrain;
		SharedPtr<Vk::Effect> mBlendmapBrushEffect;
		SharedPtr<Vk::Effect> mHeightmapBrushEffect;
		SharedPtr<Vk::RenderTarget> heightmapBrushRenderTarget;
		SharedPtr<Vk::RenderTarget> blendmapBrushRenderTarget;
		BrushBlock brushBlock;
		glm::vec2 brushPos;
	};
}