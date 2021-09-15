#include <vector>

#include "core/renderer/Primitive.h"
#include "core/Log.h"
#include "core/renderer/Model.h"
#include "core/glTFLoader.h"
#include "core/ModelLoader.h"
#include "core/renderer/Model.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/TextureLoader.h"
#include "utility/Utility.h"

namespace Utopian
{
   using Vk::Vertex;

   ModelLoader::ModelLoader(Vk::Device* device)
      : mDevice(device)
   {
      mAssimpLoader = std::make_shared<AssimpLoader>(device);
      mglTFLoader = std::make_shared<glTFLoader>(device);

      mMeshTexturesDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(device);
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(0, VK_SHADER_STAGE_ALL, 1); // diffuseSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(1, VK_SHADER_STAGE_ALL, 1); // normalSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(2, VK_SHADER_STAGE_ALL, 1); // specularSampler
      mMeshTexturesDescriptorSetLayout->Create();

      mMeshTexturesDescriptorPool = std::make_shared<Vk::DescriptorPool>(device);
      mMeshTexturesDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512);
      mMeshTexturesDescriptorPool->Create();
   }

   ModelLoader::~ModelLoader()
   {
   }

   ModelLoader& gModelLoader()
   {
      return ModelLoader::Instance();
   }

   Vk::DescriptorSetLayout* ModelLoader::GetMeshTextureDescriptorSetLayout()
   {
      return mMeshTexturesDescriptorSetLayout.get();
   }

   Vk::DescriptorPool* ModelLoader::GetMeshTextureDescriptorPool()
   {
      return mMeshTexturesDescriptorPool.get();
   }

   SharedPtr<Model> ModelLoader::LoadModel(std::string filename)
   {
      // Check if the model already is loaded
      if (mModelMap.find(filename) != mModelMap.end())
         return mModelMap[filename];

      SharedPtr<Model> model = nullptr;

      std::string extension = GetFileExtension(filename);

      if (extension == ".gltf")
         model = mglTFLoader->LoadModel(filename, mDevice);
      else
         model = mAssimpLoader->LoadModel(filename);

      if (model == nullptr)
      {
         if (mPlaceholderModel == nullptr)
            mPlaceholderModel = LoadModel(PLACEHOLDER_MODEL_PATH);

         model = mPlaceholderModel;
      }
      else
         mModelMap[filename] = model;

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadGrid(float cellSize, int numCells)
   {
      Primitive primitive;

      for (int x = 0; x < numCells; x++)
      {
         for (int z = 0; z < numCells; z++)
         {
            Vk::Vertex vertex;
            const float originOffset = (cellSize * numCells) / 2.0f - cellSize / 2;
            vertex.pos = glm::vec3(x * cellSize - originOffset, 0.0f, z * cellSize - originOffset);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.uv = glm::vec2((float)x / (numCells - 1), (float)z / (numCells - 1));
            primitive.AddVertex(vertex);
         }
      }

      for (int x = 0; x < numCells - 1; x++)
      {
         for (int z = 0; z < numCells - 1; z++)
         {
            primitive.AddTriangle(x * numCells + z, x * numCells + z + 1, (x + 1) * numCells + z);
            primitive.AddTriangle((x + 1) * numCells + z, x * numCells + z + 1, (x + 1) * numCells + (z + 1));
         }
      }

      primitive.BuildBuffers(mDevice);

      SharedPtr<Model> model = std::make_shared<Model>();

      Primitive* prim = model->AddPrimitive(primitive);
      Material* mat = model->AddMaterial(mglTFLoader->GetDefaultMaterial());

      Node* node = model->CreateNode();
      node->mesh = Mesh(prim, mat);

      model->AddRootNode(node);

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadBox()
   {
      Primitive primitive;

      // Front
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));

      // Back
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)));

      // Top
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Bottom
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Left
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Right
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive.AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Front
      primitive.AddTriangle(2, 0, 1);
      primitive.AddTriangle(0, 2, 3);

      // Back
      primitive.AddTriangle(4, 6, 5);
      primitive.AddTriangle(6, 4, 7);

      // Top
      primitive.AddTriangle(10, 8, 9);
      primitive.AddTriangle(8, 10, 11);

      // Bottom
      primitive.AddTriangle(12, 14, 13);
      primitive.AddTriangle(14, 12, 15);

      // Left
      primitive.AddTriangle(16, 18, 17);
      primitive.AddTriangle(18, 16, 19);

      // Right
      primitive.AddTriangle(22, 20, 21);
      primitive.AddTriangle(20, 22, 23);

      primitive.BuildBuffers(mDevice);

      SharedPtr<Model> model = std::make_shared<Model>();

      Primitive* prim = model->AddPrimitive(primitive);
      Material* material = model->AddMaterial(mglTFLoader->GetDefaultMaterial());

      Node* node = model->CreateNode();
      node->mesh = Mesh(prim, material);

      model->AddRootNode(node);
      model->Init();

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadQuad()
   {
      Primitive primitive;

      Vk::Vertex vertex = {};
      vertex.pos = glm::vec3(-0.5f, -0.5f, 0.5f);
      vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
      vertex.uv = glm::vec2(0.0f, 1.0f);
      primitive.AddVertex(vertex);

      vertex.pos = glm::vec3(0.5f, -0.5f, 0.5f);
      vertex.uv = glm::vec2(1.0f, 1.0f);
      primitive.AddVertex(vertex);

      vertex.pos = glm::vec3(0.5f, 0.5f, 0.5f);
      vertex.uv = glm::vec2(1.0f, 0.0f);
      primitive.AddVertex(vertex);

      vertex.pos = glm::vec3(-0.5f, 0.5f, 0.5f);
      vertex.uv = glm::vec2(0.0f, 0.0f);
      primitive.AddVertex(vertex);

      primitive.AddTriangle(2, 0, 1);
      primitive.AddTriangle(0, 2, 3);
      primitive.BuildBuffers(mDevice);
      
      SharedPtr<Model> model = std::make_shared<Model>();

      Primitive* prim = model->AddPrimitive(primitive);
      Material* mat = model->AddMaterial(mglTFLoader->GetDefaultMaterial());

      Node* node = model->CreateNode();
      node->mesh = Mesh(prim, mat);
      
      model->AddRootNode(node);

      return model;
   }

   void ModelLoader::SetInverseTranslation(bool inverse)
   {
      mglTFLoader->SetInverseTranslation(inverse);
   }
}
