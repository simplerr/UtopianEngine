#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vulkan/RenderTarget.h>
#include <vulkan/handles/Semaphore.h>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"
#include "core/renderer/Model.h"

using namespace Utopian;

namespace Utopian
{
   class glTFLoader;
   class AssimpLoader;
}

class MiniCamera;

class PhysicallyBasedRendering
{
public:
   UNIFORM_BLOCK_BEGIN(VertexInputParameters)
      UNIFORM_PARAM(glm::mat4, projection)
      UNIFORM_PARAM(glm::mat4, view)
      UNIFORM_PARAM(glm::vec3, eyePos)
      UNIFORM_PARAM(float, pad)
   UNIFORM_BLOCK_END()

   struct SceneNode
   {
      SharedPtr<Utopian::Model> model;
      glm::mat4 worldMatrix;
   };

   PhysicallyBasedRendering(Utopian::Window* window);
   ~PhysicallyBasedRendering();

   void Run();

   void DestroyCallback();
   void UpdateCallback();
   void DrawCallback();

   void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

   Utopian::Model* AddModel(std::string filename, glm::vec3 pos, glm::vec3 scale, glm::quat rotation = glm::quat());
private:
   void InitResources();

   Vk::VulkanApp* mVulkanApp;
   Utopian::Window* mWindow;

   SharedPtr<Vk::RenderTarget> mRenderTarget;
   SharedPtr<Vk::Effect> mEffect;
   SharedPtr<Vk::Effect> mSkinningEffect;
   SharedPtr<Vk::Semaphore> mPhysicallyBasedRenderingComplete;
   SharedPtr<Vk::Image> mOutputImage;
   SharedPtr<Vk::Image> mDepthImage;
   SharedPtr<Vk::Sampler> mSampler;

   SharedPtr<MiniCamera> mCamera;
   VertexInputParameters mVertexInputParameters;

   std::vector<SceneNode> mSceneNodes;
};
