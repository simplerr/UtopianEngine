#include "editor/FoliageTool.h"
#include "editor/TerrainTool.h"
#include "core/renderer/ImGuiRenderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "core/renderer/Renderer.h"
#include "core/AssetLoader.h"
#include "utility/Timer.h"
#include "utility/math/Helpers.h"
#include "utility/Utility.h"
#include "core/Input.h"
#include "core/Camera.h"
#include <random>

namespace Utopian
{
	FoliageTool::FoliageTool(Terrain* terrain, Vk::Device* device)
	{
		mTerrain = terrain;
		mDevice = device;
		mLastAddTimestamp = gTimer().GetTimestamp();
		mVegetationSettings.continuous = true;
		mVegetationSettings.restrictedDeletion = true;
		mVegetationSettings.frequency = 150.0f;
		mVegetationSettings.assetId = 147;
		mVegetationSettings.randomRotation = true;
		mVegetationSettings.randomScale = true;
		mVegetationSettings.minScale = 1.0f;
		mVegetationSettings.maxScale = 2.0f;

		// Create data for ImGui listbox
		const uint32_t numAssets = gAssetLoader().GetNumAssets();
		for (uint32_t i = 0; i < numAssets; i++)
		{
			std::string name = gAssetLoader().GetAssetByIndex(i).model;
			name = ExtractFilename(name);

			mAssetNames.push_back(_strdup(name.c_str()));
		}

		AddAssetToUi(7, "data/textures/thumbnails/7.png");
		AddAssetToUi(12, "data/textures/thumbnails/12.png");
		AddAssetToUi(17, "data/textures/thumbnails/17.png");
		AddAssetToUi(23, "data/textures/thumbnails/23.png");
		AddAssetToUi(27, "data/textures/thumbnails/27.png");
		AddAssetToUi(67, "data/textures/thumbnails/67.png");
		AddAssetToUi(104, "data/textures/thumbnails/104.png", 0.04f / 128.0f, false);
		AddAssetToUi(136, "data/textures/thumbnails/136.png");
		AddAssetToUi(145, "data/textures/thumbnails/145.png");
		AddAssetToUi(149, "data/textures/thumbnails/149.png");
	}

	FoliageTool::~FoliageTool()
	{

	}

	void FoliageTool::Update()
	{
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
					float r = radius * glm::sqrt(Math::GetRandom(0.0f, 1.0f));
					float theta = Math::GetRandom(0.0f, 1.0f) * 2 * glm::pi<float>();
					position += glm::vec3(r * glm::cos(theta), 0.0f, r * sin(theta));
					position.y = -mTerrain->GetHeight(-position.x, -position.z);
				}

				if (gInput().KeyPressed(VK_LBUTTON))
					AddVegetation(mSelectedUiAsset.assetId, position, mSelectedUiAsset.animated, true, mSelectedUiAsset.scaleFactor);

				if (gInput().KeyDown(VK_LBUTTON) && mVegetationSettings.continuous)
				{
					if (gTimer().GetElapsedTime(mLastAddTimestamp) >= (1000.0f / mVegetationSettings.frequency))
						AddVegetation(mSelectedUiAsset.assetId, position, mSelectedUiAsset.animated, true, mSelectedUiAsset.scaleFactor);
				}
			}

			if (gInput().KeyPressed(VK_RBUTTON) || gInput().KeyDown(VK_RBUTTON))
			{
				float radius = mBrushSettings->radius;

				Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
				glm::vec3 intersection = mTerrain->GetIntersectPoint(ray);
				intersection *= -1; // Todo: Note: negative sign

				if (mVegetationSettings.restrictedDeletion)
					gRenderer().RemoveInstancesWithinRadius(mSelectedUiAsset.assetId, intersection, radius);
				else
					gRenderer().RemoveInstancesWithinRadius(DELETE_ALL_ASSETS_ID, intersection, radius);

				gRenderer().BuildAllInstances();
			}
		}
	}

	void FoliageTool::RenderUi()
	{
		if (ImGui::CollapsingHeader("Foliage tool", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Frequency", &mVegetationSettings.frequency, 1.0f, 1000.0f);
			ImGui::Checkbox("Continuous", &mVegetationSettings.continuous);
			ImGui::Checkbox("Random rotation", &mVegetationSettings.randomRotation);
			ImGui::Checkbox("Random scale", &mVegetationSettings.randomScale);
			ImGui::Checkbox("Restricted deletion", &mVegetationSettings.restrictedDeletion);

			if (mVegetationSettings.randomScale)
			{
				ImGui::SliderFloat("Min scale", &mVegetationSettings.minScale, 0.0f, 20.0f);
				ImGui::SliderFloat("Max scale", &mVegetationSettings.maxScale, 0.0f, 20.0f);
			}
			else
			{
				ImGui::SliderFloat("Scale", &mVegetationSettings.minScale, 0.0f, 20.0f);
			}

			// Display preview buttons for a select couple of assets
			for (uint32_t i = 0; i < mUiAssets.size(); i++)
			{
				if (ImGui::ImageButton(mUiAssets[i].previewTextureId, ImVec2(64, 64)))
				{
					mSelectedUiAsset = mUiAssets[i];
					mBrushSettings->mode = BrushSettings::Mode::VEGETATION;
				}

				if ((i % 3 != 0 || i == 0) && i != mUiAssets.size() - 1)
					ImGui::SameLine();
			}

			// Listbox containing all assets that can be placed
			// Todo: Some of them are broken
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			if (ImGui::CollapsingHeader("All available assets"))
			{
				int selectedAsset = mSelectedUiAsset.assetId;
				if (ImGui::ListBox("", &selectedAsset, mAssetNames.data(), (int)mAssetNames.size(), 20))
				{
					UiAsset uiAsset;
					uiAsset.assetId = selectedAsset;
					mSelectedUiAsset = uiAsset;
					mBrushSettings->mode = BrushSettings::Mode::VEGETATION;
				}
			}
		}
	}

	void FoliageTool::AddAssetToUi(uint32_t assetId, std::string previewPath, float scaleFactor, bool animated)
	{
		SharedPtr<Vk::Texture> texture = Vk::gTextureLoader().LoadTexture(previewPath);
		ImTextureID previewTextureId = gRenderer().GetUiOverlay()->AddImage(texture->GetImage());

		UiAsset uiAsset;
		uiAsset.assetId = assetId;
		uiAsset.previewTextureId = previewTextureId;
		uiAsset.scaleFactor = scaleFactor;
		uiAsset.animated = animated;

		mUiAssets.push_back(uiAsset);
	}

	void FoliageTool::AddVegetation(uint32_t assetId, glm::vec3 position, bool animated, bool castShadows, float scaleFactor)
	{
		float scale = 1.0f;
		float rotationY = 0.0f;

		if (mVegetationSettings.randomScale)
			scale = Math::GetRandom(mVegetationSettings.minScale, mVegetationSettings.maxScale);
		else
			scale = mVegetationSettings.minScale;

			scale *= scaleFactor;

		if (mVegetationSettings.randomRotation)
			rotationY = Math::GetRandom(0.0f, 360.0f);

		gRenderer().AddInstancedAsset(assetId, position, glm::vec3(180.0f, rotationY, 0.0f), glm::vec3(scale), animated, castShadows);
		gRenderer().BuildAllInstances();

		mLastAddTimestamp = gTimer().GetTimestamp();
	}

	void FoliageTool::SetBrushSettings(BrushSettings* brushSettings)
	{
		mBrushSettings = brushSettings;
	}
}