#pragma once

#include "Handle.h"

namespace Vulkan
{
	class Device;
	class Semaphore : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device* device);
	private:
	};
}
