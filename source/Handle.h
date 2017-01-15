#pragma once
#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>
#include "Device.h"

namespace VulkanLib
{
	class Device;

	template<typename T>
	class Handle
	{
	public:
		Handle()
		{
			mHandle = VK_NULL_HANDLE;
			mDestroyFunc = nullptr;
		}

		Handle(std::function<void(VkDevice, T, VkAllocationCallbacks*)> destroyFunction)
		{
			mDestroyFunc = destroyFunction;
		}
		
		virtual void Cleanup(VkDevice device)
		{
			assert(mDestroyFunc);
			assert(mHandle);

			mDestroyFunc(device, mHandle, nullptr);

			mHandle = VK_NULL_HANDLE;
		}

		T GetVkHandle()
		{
			return mHandle;
		}

		T mHandle = VK_NULL_HANDLE;
	protected:
		std::function<void(VkDevice, T, VkAllocationCallbacks*)> mDestroyFunc;
	};
}
