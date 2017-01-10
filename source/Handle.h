#pragma once
#include <functional>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
	template<typename T>
	class Handle
	{
	public:
		Handle(VkDevice device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> destroyFunction)
		{
			mDevice = device;
			mDestroyFunc = destroyFunction;
		}

		~Handle()
		{
			mDestroyFunc(mDevice, mHandle, nullptr);
		}

		T GetVkHandle()
		{
			return mHandle;
		}

	protected:
		std::function<void(VkDevice, T, VkAllocationCallbacks*)> mDestroyFunc;
		T mHandle = VK_NULL_HANDLE;
		VkDevice mDevice;
	};
}
