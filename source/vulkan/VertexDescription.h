#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <map>

namespace VulkanLib
{
	/*
	Interface for all the different vertex attribute
	Extend it with new vertex attributes whenever needed
	*/
	class VertexAttribute
	{
	public:
		virtual VkFormat GetFormat() = 0;
		virtual uint32_t GetSize() = 0;
	};

	class Vec2Attribute : public VertexAttribute
	{
	public:
		virtual VkFormat GetFormat() { return VK_FORMAT_R32G32_SFLOAT; }
		virtual uint32_t GetSize() { return sizeof(glm::vec2); }
	};

	class Vec3Attribute : public VertexAttribute
	{
	public:
		virtual VkFormat GetFormat() { return VK_FORMAT_R32G32B32_SFLOAT; }
		virtual uint32_t GetSize() { return sizeof(glm::vec3); }
	};

	class Vec4Attribute : public VertexAttribute
	{
	public:
		virtual VkFormat GetFormat() { return VK_FORMAT_R32G32B32A32_SFLOAT; }
		virtual uint32_t GetSize() { return sizeof(glm::vec4); }
	};

	/*
	Contains the binding information used to bind the vertex data in C++ to GLSL
	Make sure you add attributes with a format that corresponds to  your vertex format
	VertexDescription format = C++ vertex format = GLSL vertex format, otherwise undefined behaviour!
	*/
	class VertexDescription
	{
	public:
		VertexDescription()
		{
			inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			inputState.pNext = NULL;
			inputState.flags = 0;

			// Neither the bindingDescriptions or the attributeDescriptions is used directly
			// When creating a graphics pipeline a VkPipelineVertexInputStateCreateInfo structure is sent as an argument and this structure
			// contains the VkVertexInputBindingDescription and VkVertexInputAttributeDescription
			// The last thing to do is to assign the binding and attribute descriptions
			// This is done in AddBinding() and AddAttribute()
		}

		void AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
		{
			VkVertexInputBindingDescription bindingDescription;

			bindingDescription.binding = binding;
			bindingDescription.stride = stride;
			bindingDescription.inputRate = inputRate;

			bindingDescriptions.push_back(bindingDescription);

			// Update the binding count
			inputState.vertexBindingDescriptionCount = bindingDescriptions.size();
			inputState.pVertexBindingDescriptions = bindingDescriptions.data();
		}

		// Adds a attribute
		// Note: The ordering determines the location
		void AddAttribute(uint32_t binding, VertexAttribute& attribute)
		{
			VkVertexInputAttributeDescription attributeDescription;

			attributeDescription.binding = binding;
			attributeDescription.location = attributeDescriptions.size();
			attributeDescription.format = attribute.GetFormat();
			attributeDescription.offset = offsets[binding];

			attributeDescriptions.push_back(attributeDescription);

			// Update the attribute count
			inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
			inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

			offsets[binding] += attribute.GetSize();
		}

		VkPipelineVertexInputStateCreateInfo GetInputState() const
		{
			return inputState;
		}

	private:
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		std::map<int, int> offsets;		// Each binding have their own offset
	};
}