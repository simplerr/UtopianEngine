#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Semaphore : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device* device);
	private:
	};
}
