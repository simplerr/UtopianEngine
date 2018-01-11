#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class Semaphore : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device* device);
	private:
	};
}
