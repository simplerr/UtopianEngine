#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace VulkanLib
{
	class Instance 
	{
	public:
		Instance(std::string appName, bool enableValidation);
		~Instance();

		VkInstance GetVkHandle();
	private:
		VkInstance mInstance;
	};
}
