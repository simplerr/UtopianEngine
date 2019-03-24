#include "editor/VegetationTool.h"
#include "editor/TerrainTool.h"
#include "vulkan/UIOverlay.h"
#include "core/renderer/Renderer.h"
#include "utility/Timer.h"
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

				// Todo: Add Random helper class
				std::random_device rd;
				std::mt19937 mt(rd());
				float range = mTerrain->GetTerrainSize();
				std::uniform_real_distribution<float> random(0.0f, 1.0f);

				float r = radius * glm::sqrt(random(mt));
				float theta = random(mt) * 2 * glm::pi<float>();

				glm::vec3 instancePosition = intersection + glm::vec3(r * glm::cos(theta), 0.0f, r * sin(theta));

				if (gInput().KeyPressed(VK_LBUTTON))
					AddVegetation(mVegetationSettings.assetId, instancePosition, glm::vec3(180.0f, 0.0f, 0.0f), 1.0f, true, true);

				if (gInput().KeyDown(VK_LBUTTON) && mVegetationSettings.continuous)
				{
					auto now = std::chrono::high_resolution_clock::now();
					double elapsedTime = std::chrono::duration<double, std::milli>(now - mLastAddTimestamp).count();
					if (elapsedTime >= (1000.0f / mVegetationSettings.frequency))
						AddVegetation(mVegetationSettings.assetId, instancePosition, glm::vec3(180.0f, 0.0f, 0.0f), 1.0f, true, true);
				}
			}

			if (gInput().KeyDown(VK_RBUTTON))
			{
				float radius = mBrushSettings->radius;

				Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
				glm::vec3 intersection = mTerrain->GetIntersectPoint(ray);
				intersection *= -1; // Todo: Note: negative sign

				gRenderer().RemoveInstancesWithinRadius(mVegetationSettings.assetId, intersection, radius);
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

		if (ImGui::Button("Vegetation #1"))
		{
			mBrushSettings->mode = BrushSettings::Mode::VEGETATION;
			mVegetationSettings.assetId = 147;
		}
		if (ImGui::Button("Vegetation #2"))
		{
			mBrushSettings->mode = BrushSettings::Mode::VEGETATION;
			mVegetationSettings.assetId = 67;
		}

		Vk::UIOverlay::EndWindow();
	}

	void VegetationTool::AddVegetation(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, float scale, bool animated, bool castShadows)
	{
		gRenderer().AddInstancedAsset(mVegetationSettings.assetId, position, rotation, glm::vec3(scale), animated, castShadows);
		gRenderer().BuildAllInstances();

		mLastAddTimestamp = std::chrono::high_resolution_clock::now();
	}

	void VegetationTool::SetBrushSettings(BrushSettings* brushSettings)
	{
		mBrushSettings = brushSettings;
	}
}