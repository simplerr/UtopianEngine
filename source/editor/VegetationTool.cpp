#include "editor/VegetationTool.h"
#include "editor/TerrainTool.h"
#include "vulkan/UIOverlay.h"
#include "core/renderer/Renderer.h"
#include "core/AssetLoader.h"
#include "utility/Timer.h"
#include "utility/Random.h"
#include "utility/Utility.h"
#include "Input.h"
#include "Camera.h"
#include <random>

namespace Utopian
{
	VegetationTool::VegetationTool(const SharedPtr<Terrain>& terrain, Vk::Device* device)
	{
		mTerrain = terrain;
		mDevice = device;
		mLastAddTimestamp = std::chrono::high_resolution_clock::now();
		mVegetationSettings.continuous = true;
		mVegetationSettings.frequency = 10.0f;
		mVegetationSettings.assetId = 147;
		mVegetationSettings.randomRotation = true;
		mVegetationSettings.randomScale = true;
		mVegetationSettings.minScale = 1.0f;
		mVegetationSettings.maxScale = 2.0f;

		mSelectedAsset = 0;

		// Create data for ImGui listbox
		const uint32_t numAssets = gAssetLoader().GetNumAssets();
		for (uint32_t i = 0; i < numAssets; i++)
		{
			std::string name = gAssetLoader().GetAssetByIndex(i).model;
			name = ExtractFilename(name);

			mAssetNames.push_back(strdup(name.c_str()));
		}
	}

	VegetationTool::~VegetationTool()
	{

	}

	void VegetationTool::Update()
	{
		RenderUi();

		if (mBrushSettings->mode == BrushSettings::Mode::VEGETATION)
		{
			if (gInput().KeyPressed(VK_LBUTTON) || gInput().KeyDown(VK_LBUTTON))
			{
				float radius = mBrushSettings->radius;

				Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
				glm::vec3 intersection = mTerrain->GetIntersectPoint(ray);
				intersection *= -1; // Todo: Note: negative sign

				glm::vec3 position = intersection;
				
				// If continuous mode randomize the position within brush area
				if (mVegetationSettings.continuous)
				{
					float r = radius * glm::sqrt(GetRandomFloat(0.0f, 1.0f));
					float theta = GetRandomFloat(0.0f, 1.0f) * 2 * glm::pi<float>();
					position += glm::vec3(r * glm::cos(theta), 0.0f, r * sin(theta));
				}

				if (gInput().KeyPressed(VK_LBUTTON))
					AddVegetation(mSelectedAsset, position, true, true);

				if (gInput().KeyDown(VK_LBUTTON) && mVegetationSettings.continuous)
				{
					auto now = std::chrono::high_resolution_clock::now();
					double elapsedTime = std::chrono::duration<double, std::milli>(now - mLastAddTimestamp).count();
					if (elapsedTime >= (1000.0f / mVegetationSettings.frequency))
						AddVegetation(mSelectedAsset, position, true, true);
				}
			}

			if (gInput().KeyDown(VK_RBUTTON))
			{
				float radius = mBrushSettings->radius;

				Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
				glm::vec3 intersection = mTerrain->GetIntersectPoint(ray);
				intersection *= -1; // Todo: Note: negative sign

				gRenderer().RemoveInstancesWithinRadius(mSelectedAsset, intersection, radius);
				gRenderer().BuildAllInstances();
			}
		}
	}

	void VegetationTool::RenderUi()
	{
		// Display Actor creation list
		Vk::UIOverlay::BeginWindow("Vegetation tool", glm::vec2(1500.0f, 1350.0f), 200.0f);

		ImGui::SliderFloat("Frequency", &mVegetationSettings.frequency, 1.0f, 1000.0f);
		ImGui::Checkbox("Continuous", &mVegetationSettings.continuous);
		ImGui::Checkbox("Random rotation", &mVegetationSettings.randomRotation);
		ImGui::Checkbox("Random scale", &mVegetationSettings.randomScale);

		if (mVegetationSettings.randomScale)
		{
			ImGui::SliderFloat("Min scale", &mVegetationSettings.minScale, 0.0f, 20.0f);
			ImGui::SliderFloat("Max scale", &mVegetationSettings.maxScale, 0.0f, 20.0f);
		}
		else
		{
			ImGui::SliderFloat("Scale", &mVegetationSettings.minScale, 0.0f, 20.0f);
		}

		// Listbox containing all assets that can be placed
		if (ImGui::ListBox("", &mSelectedAsset, mAssetNames.data(), mAssetNames.size(), 20))
			mBrushSettings->mode = BrushSettings::Mode::VEGETATION;

		Vk::UIOverlay::EndWindow();
	}

	void VegetationTool::AddVegetation(uint32_t assetId, glm::vec3 position, bool animated, bool castShadows)
	{
		float scale = 1.0f;
		float rotationY = 0.0f;

		if (mVegetationSettings.randomScale)
			scale = GetRandomFloat(mVegetationSettings.minScale, mVegetationSettings.maxScale);
		else
			scale = mVegetationSettings.minScale;

		if (mVegetationSettings.randomRotation)
			rotationY = GetRandomFloat(0.0f, 360.0f);

		gRenderer().AddInstancedAsset(assetId, position, glm::vec3(180.0f, rotationY, 0.0f), glm::vec3(scale), animated, castShadows);
		gRenderer().BuildAllInstances();

		mLastAddTimestamp = std::chrono::high_resolution_clock::now();
	}

	void VegetationTool::SetBrushSettings(BrushSettings* brushSettings)
	{
		mBrushSettings = brushSettings;
	}
}