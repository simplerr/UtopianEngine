#include "StaticModel.h"
#include "Debug.h"
#include "vulkan/handles/Device.h"

namespace Utopian::Vk
{
   StaticModel::StaticModel()
   {

   }

   StaticModel::~StaticModel()
   {
      for (auto mesh : mMeshes)
         delete mesh;
   }

   void StaticModel::AddMesh(Primitive* mesh)
   {
      mMeshes.push_back(mesh);
   }

   void StaticModel::Init(Device* device)
   {
      std::vector<Vertex> vertexVector;
      std::vector<uint32_t> indexVector;

      for (int meshId = 0; meshId < mMeshes.size(); meshId++)
      {
         for (int i = 0; i < mMeshes[meshId]->vertexVector.size(); i++)
         {
            Vertex vertex = mMeshes[meshId]->vertexVector[i];
            vertexVector.push_back(vertex);
         }

         for (int i = 0; i < mMeshes[meshId]->indexVector.size(); i++)
         {
            indexVector.push_back(mMeshes[meshId]->indexVector[i]);
         }
      }

      mVerticesCount = (uint32_t)vertexVector.size();
      mIndicesCount = (uint32_t)indexVector.size();

      mBoundingBox.Init(vertexVector);
   }

   int StaticModel::GetNumIndices()
   {
      return mIndicesCount;
   }

   int StaticModel::GetNumVertics()
   {
      return mVerticesCount;
   }

   BoundingBox StaticModel::GetBoundingBox()
   {
      return mBoundingBox;
   }
}  // VulkanLib namespace