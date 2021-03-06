#include <string>
#include <fstream>
#include "core/renderer/InstancingManager.h"
#include "core/renderer/SceneInfo.h"
#include "core/renderer/Renderer.h"
#include "core/AssetLoader.h"
#include "vulkan/handles/Device.h"
#include "utility/math/Helpers.h"

namespace Utopian
{
   InstancingManager::InstancingManager(Renderer* renderer)
   {
      mSceneInfo = renderer->GetSceneInfo();
      mDevice = renderer->GetDevice();
   }

   InstancingManager::~InstancingManager()
   {

   }

   void InstancingManager::UpdateInstanceAltitudes()
   {
      for (uint32_t i = 0; i < mSceneInfo->instanceGroups.size(); i++)
      {
         mSceneInfo->instanceGroups[i]->UpdateAltitudes(mSceneInfo->terrain);
         mSceneInfo->instanceGroups[i]->BuildBuffer(mDevice);
      }
   }

   void InstancingManager::AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated, bool castShadow)
   {
      // Instance group already exists?
      SharedPtr<InstanceGroup> instanceGroup = nullptr;
      for (uint32_t i = 0; i < mSceneInfo->instanceGroups.size(); i++)
      {
         if (mSceneInfo->instanceGroups[i]->GetAssetId() == assetId)
         {
            instanceGroup = mSceneInfo->instanceGroups[i];
            break;
         }
      }

      if (instanceGroup == nullptr)
      {
         // Todo: Check if assetId is valid
         instanceGroup = std::make_shared<InstanceGroup>(assetId, animated, castShadow);
         mSceneInfo->instanceGroups.push_back(instanceGroup);
      }

      instanceGroup->AddInstance(position, rotation, scale);
   }

   void InstancingManager::RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius)
   {
      for (auto iter = mSceneInfo->instanceGroups.begin(); iter != mSceneInfo->instanceGroups.end();)
      {
         if (assetId == DELETE_ALL_ASSETS_ID || (*iter)->GetAssetId() == assetId)
         {
            (*iter)->RemoveInstancesWithinRadius(position, radius);
            (*iter)->BuildBuffer(mDevice);
         }

         // Remove instance group if empty
         if ((*iter)->GetNumInstances() == 0)
            iter = mSceneInfo->instanceGroups.erase(iter);
         else
            iter++;
      }
   }

   void InstancingManager::ClearInstanceGroups()
   {
      for (uint32_t i = 0; i < mSceneInfo->instanceGroups.size(); i++)
      {
         mSceneInfo->instanceGroups[i]->RemoveInstances();
      }

      mSceneInfo->instanceGroups.clear();
   }

   void InstancingManager::SaveInstancesToFile(const std::string& filename)
   {
      std::ofstream fout(filename);

      fout << "INSTANCES:";

      if (mSceneInfo->instanceGroups.size() > 0)
         fout << std::endl;

      for (auto& instanceGroup : mSceneInfo->instanceGroups)
      {
         instanceGroup->SaveToFile(fout);
      }

      fout.close();
   }

   void InstancingManager::LoadInstancesFromFile(const std::string& filename)
   {
      std::ifstream fin(filename);

      std::string header;
      fin >> header;

      while (!fin.eof())
      {
         uint32_t assetId;
         bool animated, castShadows;
         glm::vec3 position, rotation, scale;
         fin >> assetId >> animated >> castShadows;
         fin >> position.x >> position.y >> position.z;
         fin >> rotation.x >> rotation.y >> rotation.z;
         fin >> scale.x >> scale.y >> scale.z;

         AddInstancedAsset(assetId, position, rotation, scale, animated, castShadows);
      }

      fin.close();
   }

   void InstancingManager::BuildAllInstances()
   {
      for (uint32_t i = 0; i < mSceneInfo->instanceGroups.size(); i++)
      {
         mSceneInfo->instanceGroups[i]->BuildBuffer(mDevice);
      }
   }

   InstanceGroup::InstanceGroup(uint32_t assetId, bool animated, bool castShadows)
   {
      mAssetId = assetId;
      mInstanceBuffer = nullptr;
      mAnimated = animated;
      mCastShadows = castShadows;

      mModel = gAssetLoader().LoadAsset(assetId);

      assert(mModel);
   }

   InstanceGroup::~InstanceGroup()
   {
      RemoveInstances();
   }

   void InstanceGroup::AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
   {
      glm::mat4 world = glm::translate(glm::mat4(), position);
      world = glm::rotate(world, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      world = glm::rotate(world, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      world = glm::rotate(world, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
      world = glm::scale(world, scale);

      InstanceDataGPU instanceData;
      instanceData.world = world;

      mInstances.push_back(instanceData);
      mInstanceData.push_back(InstanceData(position, rotation, scale));
   }

   void InstanceGroup::RemoveInstances()
   {
      mInstances.clear();
      mInstanceData.clear();

      gRenderer().GetDevice()->QueueDestroy(mInstanceBuffer);
   }

   void InstanceGroup::RemoveInstancesWithinRadius(glm::vec3 position, float radius)
   {
      for (auto iter = mInstances.begin(); iter != mInstances.end();)
      {
         if (glm::distance(position, Math::GetTranslation((*iter).world)) < radius)
            iter = mInstances.erase(iter);
         else
            iter++;
      }

      for (auto iter = mInstanceData.begin(); iter != mInstanceData.end();)
      {
         if (glm::distance(position, (*iter).position) < radius)
            iter = mInstanceData.erase(iter);
         else
            iter++;
      }
   }

   void InstanceGroup::SaveToFile(std::ofstream& fout)
   {
      for (auto& instance : mInstanceData)
      {
         fout << mAssetId << " " << mAnimated << " " << mCastShadows << " ";
         fout << instance.position.x << " " << instance.position.y << " " << instance.position.z << " ";
         fout << instance.rotation.x << " " << instance.rotation.y << " " << instance.rotation.z << " ";
         fout << instance.scale.x << " " << instance.scale.y << " " << instance.scale.z;
         fout << std::endl;
      }
   }

   void InstanceGroup::BuildBuffer(Vk::Device* device)
   {
      // Todo: use device local buffer for better performance
      // Note: Recreating the buffer every time since if the size has increased just
      // mapping and updating the memory is not enough.
      if (GetNumInstances() != 0)
      {
         gRenderer().GetDevice()->QueueDestroy(mInstanceBuffer);

         Vk::BUFFER_CREATE_INFO createInfo;
         createInfo.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
         createInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
         createInfo.data = mInstances.data();
         createInfo.size = mInstances.size() * sizeof(InstanceDataGPU);
         createInfo.name = "ScreenQuad vertex buffer";
         mInstanceBuffer = std::make_shared<Vk::Buffer>(createInfo, device);
      }
   }

   void InstanceGroup::UpdateAltitudes(const SharedPtr<Terrain>& terrain)
   {
      for (uint32_t i = 0; i < mInstances.size(); i++)
      {
         glm::vec3 translation = mInstanceData[i].position;
         translation.y = -terrain->GetHeight(-translation.x, -translation.z);
         mInstances[i].world = Math::SetTranslation(mInstances[i].world, translation);
         mInstanceData[i].position = translation;
      }
   }

   void InstanceGroup::SetAnimated(bool animated)
   {
      mAnimated = animated;
   }

   void InstanceGroup::SetCastShadows(bool castShadows)
   {
      mCastShadows = castShadows;
   }

   uint32_t InstanceGroup::GetAssetId()
   {
      return mAssetId;
   }

   uint32_t InstanceGroup::GetNumInstances()
   {
      return (uint32_t)mInstances.size();
   }

   Vk::Buffer* InstanceGroup::GetBuffer()
   {
      return mInstanceBuffer.get();
   }

   Vk::StaticModel* InstanceGroup::GetModel()
   {
      return mModel.get();
   }

   bool InstanceGroup::IsAnimated()
   {
      return mAnimated;
   }

   bool InstanceGroup::IsCastingShadows()
   {
      return mCastShadows;
   }
}
