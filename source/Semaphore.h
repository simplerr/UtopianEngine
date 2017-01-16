#pragma once
#include "Handle.h"

namespace VulkanLib
{
	class Device;
	class Semaphore : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device* device);
	private:
	};
}
