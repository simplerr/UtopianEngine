#pragma once
#include <string>
#include <glm/glm.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/ShaderBuffer.h"
#include "core/Terrain.h"

namespace Utopian
{
	class Terrain;

	class TerrainTool
	{
	public:
		TerrainTool(const SharedPtr<Terrain>& terrain, Vk::Device* device);
		~TerrainTool();

		void Update();
		void RenderUi();

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
		Terrain::BrushBlock brushBlock;
		BrushSettings brushSettings;
	};
}