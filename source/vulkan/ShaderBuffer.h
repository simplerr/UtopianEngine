#pragma once

#include "vulkan/VulkanInclude.h"

/**
 * Macros that can be used to ease the creation of the C++ representation
 * of the GLSL uniform blocks.
 *  
 * @note Will not be sufficient in complicated uniform blocks (eg. light data)
 * but in those cases ShaderBuffer can be inherited and used manually instead.
 */
#define UNIFORM_BLOCK_BEGIN(Name) 											\
		class Name : public Utopian::Vk::ShaderBuffer 						\
		{ 																	\
		public: 															\
			virtual void UpdateMemory() { 									\
				uint8_t *mapped; 											\
				mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped); 	\
				memcpy(mapped, &data, sizeof(data));	 					\
				mBuffer->UnmapMemory(); 									\
			}																\
																			\
			virtual int GetSize() { 										\
				return sizeof(data); 										\
			} 																\
 																			\
			struct { 														\

#define UNIFORM_PARAM(Type, Name) Type Name;

#define UNIFORM_BLOCK_END() 	\
			} data; 			\
		}; 						\

namespace Utopian::Vk
{
	/** Base class for uniform and storage buffer. */
	class ShaderBuffer
	{
	public:
		virtual ~ShaderBuffer();

		// [NOTE] This has to be called after elements have been added to vectors, since GetSize() needs to return the correct size
		// Creates a VkBuffer and maps it to a VkMemory (VulkanBase::CreateBuffer())
		void Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		void MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data);
		void UnmapMemory();

		// This is where the data gets transfered to device memory w/ vkMapMemory,vkUnmapMemory and memcpy
		virtual void UpdateMemory() = 0;

		virtual int GetSize() = 0;

		VkDescriptorBufferInfo* GetDescriptor() { return &mDescriptor; }
		Buffer* GetBuffer() { return mBuffer; }

	protected:
		Buffer* mBuffer;
		VkDescriptorBufferInfo mDescriptor;
	};
}
