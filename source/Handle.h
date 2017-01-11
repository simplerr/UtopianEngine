#pragma once
#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
	template<typename T>
	class Handle
	{
	public:
		Handle()
		{
			mDevice = VK_NULL_HANDLE;
			mHandle = VK_NULL_HANDLE;
			mDestroyFunc = nullptr;
		}

		Handle(VkDevice device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> destroyFunction)
		{
			mDevice = device;
			mDestroyFunc = destroyFunction;
		}

		virtual ~Handle()
		{
			Cleanup();
		}
		
		virtual void Cleanup()
		{
			assert(mDevice);
			assert(mHandle);
			assert(mDestroyFunc);

			mDestroyFunc(mDevice, mHandle, nullptr);
		}

		void SetDevice(VkDevice device)
		{
			mDevice = device;
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
