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
#include "vulkan/StaticModel.h"
#include "vulkan/TextureLoader.h"

// TODO: Note that the format should be #include <assimp/Importer.hpp> but something in the project settings is wrong
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/cimport.h"
#include "../external/assimp/assimp/material.h"
#include "../external/assimp/assimp/ai_assert.h"
#include "../external/assimp/assimp/postprocess.h"
#include "../external/assimp/assimp/scene.h"

namespace Utopian::Vk
{
   ModelLoader::ModelLoader(Device* device)
      : mDevice(device)
   {
      mAssimpLoader = std::make_shared<AssimpLoader>(device);
      mglTFLoader = std::make_shared<glTFLoader>(device);

      mMeshTexturesDescriptorSetLayout = std::make_shared<DescriptorSetLayout>(device);
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(0, VK_SHADER_STAGE_ALL, 1); // diffuseSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(1, VK_SHADER_STAGE_ALL, 1); // normalSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(2, VK_SHADER_STAGE_ALL, 1); // specularSampler
      mMeshTexturesDescriptorSetLayout->Create();

      mMeshTexturesDescriptorPool = std::make_shared<DescriptorPool>(device);
      mMeshTexturesDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512);
      mMeshTexturesDescriptorPool->Create();
   }

   ModelLoader::~ModelLoader()
   {
      CleanupModels(mDevice->GetVkDevice());
   }

   void ModelLoader::CleanupModels(VkDevice device)
   {
   }

   ModelLoader& gModelLoader()
   {
      return ModelLoader::Instance();
   }

   SharedPtr<DescriptorSetLayout> ModelLoader::GetMeshTextureDescriptorSetLayout()
   {
      return mMeshTexturesDescriptorSetLayout;
   }

   SharedPtr<DescriptorPool> ModelLoader::GetMeshTextureDescriptorPool()
   {
      return mMeshTexturesDescriptorPool;
   }

   SharedPtr<Model> ModelLoader::LoadModel2(std::string filename)
   {
      // Check if the model already is loaded
      if (mModelMap2.find(filename) != mModelMap2.end())
         return mModelMap2[filename];

      SharedPtr<Model> model = nullptr;

      // Todo: Check file extension
      model = mAssimpLoader->LoadModel(filename);

      if (model == nullptr)
      {
         if (mPlaceholderModel2 == nullptr)
            mPlaceholderModel2 = LoadModel2(PLACEHOLDER_MODEL_PATH);

         model = mPlaceholderModel2;
      }

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadBox2(std::string texture)
   {
      Primitive* primitive = new Primitive(mDevice);

      // Front
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));

      // Back
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)));

      // Top
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Bottom
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Left
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Right
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));

      // Front
      primitive->AddTriangle(2, 0, 1);
      primitive->AddTriangle(0, 2, 3);

      // Back
      primitive->AddTriangle(4, 6, 5);
      primitive->AddTriangle(6, 4, 7);

      // Top
      primitive->AddTriangle(10, 8, 9);
      primitive->AddTriangle(8, 10, 11);

      // Bottom
      primitive->AddTriangle(12, 14, 13);
      primitive->AddTriangle(14, 12, 15);

      // Left
      primitive->AddTriangle(16, 18, 17);
      primitive->AddTriangle(18, 16, 19);

      // Right
      primitive->AddTriangle(22, 20, 21);
      primitive->AddTriangle(20, 22, 23);

      primitive->BuildBuffers(mDevice);

      Mesh mesh;
      mesh.AddPrimitive(primitive, mglTFLoader->GetDefaultMaterial());

      Node* node = new Node();
      node->mesh = mesh;

      SharedPtr<Model> model = std::make_shared<Model>();
      model->AddNode(node);

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadGrid2(float cellSize, int numCells)
   {
      Primitive* primitive = new Primitive(mDevice);

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
            primitive->AddVertex(vertex);
         }
      }

      for (int x = 0; x < numCells - 1; x++)
      {
         for (int z = 0; z < numCells - 1; z++)
         {
            primitive->AddTriangle(x * numCells + z, x * numCells + z + 1, (x + 1) * numCells + z);
            primitive->AddTriangle((x + 1) * numCells + z, x * numCells + z + 1, (x + 1) * numCells + (z + 1));
         }
      }

      primitive->BuildBuffers(mDevice);

      Mesh mesh;
      mesh.AddPrimitive(primitive, mglTFLoader->GetDefaultMaterial());

      Node* node = new Node();
      node->mesh = mesh;

      SharedPtr<Model> model = std::make_shared<Model>();
      model->AddNode(node);

      return model;
   }

   SharedPtr<StaticModel> ModelLoader::LoadModel(std::string filename)
   {
      // Check if the model already is loaded
      if (mModelMap.find(filename) != mModelMap.end())
         return mModelMap[filename];

      SharedPtr<StaticModel> model = std::make_shared<StaticModel>();

      Assimp::Importer importer;

      // Load scene from the file.
      const aiScene* scene = importer.ReadFile(filename, aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

      if (scene != nullptr)
      {
         // Loop over all meshes
         for (unsigned int meshId = 0u; meshId < scene->mNumMeshes; meshId++)
         {
            Primitive* primitive = new Primitive(mDevice);
            aiMesh* assimpMesh = scene->mMeshes[meshId];

            // Get the diffuse color
            aiColor3D color(0.f, 0.f, 0.f);
            scene->mMaterials[assimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

            // Load vertices
            for (unsigned int vertexId = 0u; vertexId < assimpMesh->mNumVertices; vertexId++)
            {
               aiVector3D pos = assimpMesh->mVertices[vertexId];
               aiVector3D normal = assimpMesh->mNormals[vertexId];
               aiVector3D uv = aiVector3D(0, 0, 0);
               aiVector3D tangent = aiVector3D(0, 0, 0);

               if (assimpMesh->HasTextureCoords(0))
                  uv = assimpMesh->mTextureCoords[0][vertexId];

               if (assimpMesh->HasTangentsAndBitangents())
                  tangent = assimpMesh->mTangents[vertexId];

               normal = normal.Normalize();
               Vertex vertex = {};
               vertex.pos = glm::vec3(pos.x, pos.y, pos.z);
               vertex.normal = glm::vec3(normal.x, normal.y, normal.z);
               vertex.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 1.0f);
               vertex.uv = glm::vec2(uv.x, uv.y);
               vertex.color = glm::vec3(color.r, color.g, color.b);
               primitive->AddVertex(vertex);
            }

            // Load indices
            for (unsigned int faceId = 0u; faceId < assimpMesh->mNumFaces; faceId++)
            {
               for (unsigned int indexId = 0u; indexId < assimpMesh->mFaces[faceId].mNumIndices; indexId+=3)
               {
                  primitive->AddTriangle(assimpMesh->mFaces[faceId].mIndices[indexId], assimpMesh->mFaces[faceId].mIndices[indexId+1], assimpMesh->mFaces[faceId].mIndices[indexId+2]);
               }
            }

            // Get texture path
            aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
            int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
            int numNormalMaps = material->GetTextureCount(aiTextureType_NORMALS);
            int numHeightMaps = material->GetTextureCount(aiTextureType_HEIGHT);
            int numSpecularMaps = material->GetTextureCount(aiTextureType_SPECULAR);

            std::string diffuseTexturePath = PLACEHOLDER_TEXTURE_PATH;
            std::string normalTexturePath = DEFAULT_NORMAL_MAP_TEXTURE;
            std::string specularTexturePath = DEFAULT_SPECULAR_MAP_TEXTURE;

            /* Diffuse texture */
            if (numTextures > 0)
            {
               aiString texPath;
               material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
               FindValidPath(&texPath, filename);
               diffuseTexturePath = texPath.C_Str();
               SharedPtr<Texture> texture = gTextureLoader().LoadTexture(diffuseTexturePath);

               // Workaround for Unity assets
               // Note: Instead of calling LoadTexture() something similar to FindFile(diffuseTexturePath) could be used
               if (texture == nullptr)
               {
                  uint32_t idx = (uint32_t)diffuseTexturePath.rfind("\\");
                  std::string textureName = diffuseTexturePath.substr(idx+1);
                  idx = (uint32_t)filename.rfind("/");
                  diffuseTexturePath = filename.substr(0, idx) + "/Textures/" + textureName;

                  texture = gTextureLoader().LoadTexture(diffuseTexturePath);

                  // Try removing _H from the filename
                  if (texture == nullptr)
                  {
                     idx = (uint32_t)diffuseTexturePath.rfind("_H.tga");
                     if (idx != std::string::npos)
                     {
                        diffuseTexturePath = diffuseTexturePath.substr(0, idx) + ".tga";
                        texture = gTextureLoader().LoadTexture(diffuseTexturePath);
                     }

                     if (texture == nullptr)
                     {
                        idx = (uint32_t)diffuseTexturePath.rfind("_H.png");
                        if (idx != std::string::npos)
                        {
                           diffuseTexturePath = diffuseTexturePath.substr(0, idx) + ".png";
                           texture = gTextureLoader().LoadTexture(diffuseTexturePath);
                        }
                     }
                  }
               }

               if (texture == nullptr)
               {
                  texPath = PLACEHOLDER_TEXTURE_PATH;
               }
            }

            /* Normal texture */
            if (numNormalMaps > 0)
            {
               normalTexturePath = GetPath(material, aiTextureType_NORMALS, filename);
            }

            /* Heightmap texture, in some formats this is the same as the normal map */
            if (numHeightMaps > 0)
            {
               assert(numNormalMaps == 0);
               normalTexturePath = GetPath(material, aiTextureType_HEIGHT, filename);
            }

            /* Specular texture */
            if (numSpecularMaps > 0)
            {
               specularTexturePath = GetPath(material, aiTextureType_SPECULAR, filename);
            }

            primitive->LoadTextures(diffuseTexturePath, normalTexturePath, specularTexturePath);
            primitive->SetDebugName(filename);
            primitive->BuildBuffers(mDevice);
            model->AddMesh(primitive);
         }

         // Add the model to the model map
         model->Init(mDevice);
         mModelMap[filename] = model;
      }
      else {
         // Loading of model failed
         UTO_LOG("Failed to load model: " + filename);

         if (mPlaceholderModel == nullptr)
            mPlaceholderModel = LoadModel(PLACEHOLDER_MODEL_PATH);

         return mPlaceholderModel;
      }

      return model;
   }

   SharedPtr<Model> ModelLoader::LoadQuad()
   {
      Utopian::Primitive* primitive = new Utopian::Primitive(nullptr);

      Vk::Vertex vertex = {};
      vertex.pos = glm::vec3(-0.5f, -0.5f, 0.5f);
      vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
      vertex.uv = glm::vec2(0.0f, 1.0f);
      primitive->AddVertex(vertex);

      vertex.pos = glm::vec3(0.5f, -0.5f, 0.5f);
      vertex.uv = glm::vec2(1.0f, 1.0f);
      primitive->AddVertex(vertex);

      vertex.pos = glm::vec3(0.5f, 0.5f, 0.5f);
      vertex.uv = glm::vec2(1.0f, 0.0f);
      primitive->AddVertex(vertex);

      vertex.pos = glm::vec3(-0.5f, 0.5f, 0.5f);
      vertex.uv = glm::vec2(0.0f, 0.0f);
      primitive->AddVertex(vertex);

      primitive->AddTriangle(2, 0, 1);
      primitive->AddTriangle(0, 2, 3);
      primitive->BuildBuffers(mDevice);
      
      Mesh mesh;
      mesh.AddPrimitive(primitive, mglTFLoader->GetDefaultMaterial());
      
      Node* node = new Node();
      node->mesh = mesh;
      
      SharedPtr<Model> model = std::make_shared<Model>();
      model->AddNode(node);

      return model;
   }

   SharedPtr<StaticModel> ModelLoader::LoadGrid(float cellSize, int numCells)
   {
      std::string name = "grid: " + std::to_string(cellSize) + ", " + std::to_string(numCells);

      // Check if the model already is loaded
      if (mModelMap.find(name) != mModelMap.end())
         return mModelMap[name];

      SharedPtr<StaticModel> model = std::make_shared<StaticModel>();
      Primitive* primitive = new Primitive(mDevice);

      for (int x = 0; x < numCells; x++)
      {
         for (int z = 0; z < numCells; z++)
         {
            Vk::Vertex vertex;
            const float originOffset = (cellSize * numCells) / 2.0f - cellSize / 2;
            vertex.pos = glm::vec3(x * cellSize - originOffset, 0.0f, z * cellSize - originOffset);
            vertex.normal = glm::vec3(0.0f, -1.0f, 0.0f);
            vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.uv = glm::vec2((float)x / (numCells - 1), (float)z / (numCells - 1));
            primitive->AddVertex(vertex);
         }
      }

      for (int x = 0; x < numCells - 1; x++)
      {
         for (int z = 0; z < numCells - 1; z++)
         {
            primitive->AddTriangle(x * numCells + z, x * numCells + z + 1, (x + 1) * numCells + z);
            primitive->AddTriangle((x + 1) * numCells + z, x * numCells + z + 1, (x + 1) * numCells + (z + 1));
         }
      }

      primitive->LoadTextures(PLACEHOLDER_TEXTURE_PATH);
      primitive->BuildBuffers(mDevice);
      model->AddMesh(primitive);

      model->Init(mDevice);
      mModelMap[name] = model;
      return model;
   }

   SharedPtr<StaticModel> ModelLoader::LoadBox(std::string texture)
   {
      SharedPtr<StaticModel> model = std::make_shared<StaticModel>();
      Primitive* primitive = new Primitive(mDevice);

      // Front
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)));

      // Back
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)));

      // Top
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));

      // Bottom
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));

      // Left
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));

      // Right
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
      primitive->AddVertex(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));

      // Front
      primitive->AddTriangle(1, 0, 2);
      primitive->AddTriangle(3, 2, 0);

      // Back
      primitive->AddTriangle(5, 6, 4);
      primitive->AddTriangle(7, 4, 6);

      // Top
      primitive->AddTriangle(9, 8, 10);
      primitive->AddTriangle(11, 10, 8);

      // Bottom
      primitive->AddTriangle(13, 14, 12);
      primitive->AddTriangle(15, 12, 14);

      // Left
      primitive->AddTriangle(17, 18, 16);
      primitive->AddTriangle(19, 16, 18);

      // Right
      primitive->AddTriangle(21, 20, 22);
      primitive->AddTriangle(23, 22, 20);

      primitive->LoadTextures(texture);
      primitive->BuildBuffers(mDevice);
      model->AddMesh(primitive);

      model->Init(mDevice);

      return model;
   }

   std::string ModelLoader::GetPath(aiMaterial* material, aiTextureType textureType, std::string filename)
   {
      std::string path;

      aiString texPath;
      material->GetTexture(textureType, 0, &texPath);
      FindValidPath(&texPath, filename);
      path = texPath.C_Str();

      return path;
   }

   // From /assimp/assimp/tools/assimp_view/Material.cpp
   int ModelLoader::FindValidPath(aiString* texturePath, std::string modelPath)
   {
      ai_assert(NULL != texturePath);
      aiString pcpy = *texturePath;
      if ('*' == texturePath->data[0]) {
         // '*' as first character indicates an embedded file
         return 5;
      }

      // first check whether we can directly load the file
      FILE* pFile = fopen(texturePath->data, "rb");
      if (pFile)fclose(pFile);
      else
      {
         // check whether we can use the directory of  the asset as relative base
         char szTemp[MAX_PATH * 2], tmp2[MAX_PATH * 2];
         strcpy(szTemp, modelPath.c_str());
         strcpy(tmp2, szTemp);

         char* szData = texturePath->data;
         if (*szData == '\\' || *szData == '/')++szData;

         char* szEnd = strrchr(szTemp, '\\');
         if (!szEnd)
         {
            szEnd = strrchr(szTemp, '/');
            if (!szEnd)szEnd = szTemp;
         }
         szEnd++;
         *szEnd = 0;
         strcat(szEnd, szData);


         pFile = fopen(szTemp, "rb");
         if (!pFile)
         {
            // convert the string to lower case
            for (unsigned int i = 0;; ++i)
            {
               if ('\0' == szTemp[i])break;
               szTemp[i] = (char)tolower(szTemp[i]);
            }

            if (TryLongerPath(szTemp, texturePath))return 1;
            *szEnd = 0;

            // search common sub directories
            strcat(szEnd, "tex\\");
            strcat(szEnd, szData);

            pFile = fopen(szTemp, "rb");
            if (!pFile)
            {
               if (TryLongerPath(szTemp, texturePath))return 1;

               *szEnd = 0;

               strcat(szEnd, "textures\\");
               strcat(szEnd, szData);

               pFile = fopen(szTemp, "rb");
               if (!pFile)
               {
                  if (TryLongerPath(szTemp, texturePath))return 1;
               }

               // patch by mark sibly to look for textures files in the asset's base directory.
               const char *path = pcpy.data;
               const char *p = strrchr(path, '/');
               if (!p) p = strrchr(path, '\\');
               if (p) {
                  char *q = strrchr(tmp2, '/');
                  if (!q) q = strrchr(tmp2, '\\');
                  if (q) {
                     strcpy(q + 1, p + 1);
                     if (pFile = fopen(tmp2, "r")) {
                        fclose(pFile);
                        strcpy(texturePath->data, tmp2);
                        texturePath->length = strlen(tmp2);
                        return 1;
                     }
                  }
               }
               return 0;
            }
         }
         fclose(pFile);

         // copy the result string back to the aiString
         const size_t iLen = strlen(szTemp);
         size_t iLen2 = iLen + 1;
         iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
         memcpy(texturePath->data, szTemp, iLen2);
         texturePath->length = iLen;

      }
      return 1;
   }

   // From /assimp/assimp/tools/assimp_view/Material.cpp
   bool ModelLoader::TryLongerPath(char* szTemp, aiString* p_szString)
   {
      char szTempB[MAX_PATH];
      strcpy(szTempB, szTemp);

      // go to the beginning of the file name
      char* szFile = strrchr(szTempB, '\\');
      if (!szFile)szFile = strrchr(szTempB, '/');

      char* szFile2 = szTemp + (szFile - szTempB) + 1;
      szFile++;
      char* szExt = strrchr(szFile, '.');
      if (!szExt)return false;
      szExt++;
      *szFile = 0;

      strcat(szTempB, "*.*");
      const unsigned int iSize = (const unsigned int)(szExt - 1 - szFile);

      HANDLE          h;
      WIN32_FIND_DATA info;

      // build a list of files
      h = FindFirstFile(szTempB, &info);
      if (h != INVALID_HANDLE_VALUE)
      {
         do
         {
            if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
            {
               char* szExtFound = strrchr(info.cFileName, '.');
               if (szExtFound)
               {
                  ++szExtFound;
                  if (0 == _stricmp(szExtFound, szExt))
                  {
                     const unsigned int iSizeFound = (const unsigned int)(
                        szExtFound - 1 - info.cFileName);

                     for (unsigned int i = 0; i < iSizeFound; ++i)
                        info.cFileName[i] = (CHAR)tolower(info.cFileName[i]);

                     if (0 == memcmp(info.cFileName, szFile2, glm::min(iSizeFound, iSize)))
                     {
                        // we have it. Build the full path ...
                        char* sz = strrchr(szTempB, '*');
                        *(sz - 2) = 0x0;

                        strcat(szTempB, info.cFileName);

                        // copy the result string back to the aiString
                        const size_t iLen = strlen(szTempB);
                        size_t iLen2 = iLen + 1;
                        iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                        memcpy(p_szString->data, szTempB, iLen2);
                        p_szString->length = iLen;
                        return true;
                     }
                  }
                  // check whether the 8.3 DOS name is matching
                  if (0 == _stricmp(info.cAlternateFileName, p_szString->data))
                  {
                     strcat(szTempB, info.cAlternateFileName);

                     // copy the result string back to the aiString
                     const size_t iLen = strlen(szTempB);
                     size_t iLen2 = iLen + 1;
                     iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                     memcpy(p_szString->data, szTempB, iLen2);
                     p_szString->length = iLen;
                     return true;
                  }
               }
            }
         } while (FindNextFile(h, &info));

         FindClose(h);
      }
      return false;
   }
}  // VulkanLib namespace