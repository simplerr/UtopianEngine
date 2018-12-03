#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/DescriptorSet.h"
#include "Mesh.h"

namespace Utopian::Vk
{
	Mesh::Mesh(Device* device)
	{
		mDevice = device;
	}
	
	Mesh::~Mesh()
	{
		
	}

	void Mesh::AddVertex(Vertex vertex)
	{
		vertexVector.push_back(vertex);
	}

	void Mesh::AddVertex(float x, float y, float z)
	{
		AddVertex(Vertex(x, y, z));
	}

	void Mesh::AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
	{
		indexVector.push_back(v1);
		indexVector.push_back(v2);
		indexVector.push_back(v3);
	}

	void Mesh::BuildBuffers(Device* device)
	{
		mVerticesCount = vertexVector.size();
		mIndicesCount = indexVector.size();

		uint32_t vertexBufferSize = mVerticesCount * sizeof(Vertex);
		uint32_t indexBufferSize = mIndicesCount * sizeof(uint32_t);

		mVertexBuffer = std::make_shared<Buffer>(device,
												 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
												 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
												 vertexBufferSize,
												 vertexVector.data());

		mIndexBuffer = std::make_shared<Buffer>(device,
												VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
												indexBufferSize,
												indexVector.data());
	}

	void Mesh::BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>)
	{

	}

	void Mesh::SetTexture(Texture* texture)
	{
		mTexture = texture;
	}

	VkDescriptorSet Mesh::GetTextureDescriptor()
	{
		return mTexture->GetDescriptorSet()->descriptorSet;
	}

	BoundingBox Mesh::GetBoundingBox()
	{
		return mBoundingBox;
	}

	uint32_t Mesh::GetNumIndices()
	{
		return mIndicesCount;
	}

	void Mesh::SetTexturePath(std::string texturePath)
	{
		mTexturePath = texturePath;
	}

	Buffer* Mesh::GetVertxBuffer()
	{
		return mVertexBuffer.get();
	}

	Buffer* Mesh::GetIndexBuffer()
	{
		return mIndexBuffer.get();
	}
}
