#pragma once

#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>
#include "vulkan/Device.h" 

namespace Vulkan
{
	class Device;

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

		virtual ~Handle()
		{
			// REMOVE THIS
			if (mDevice == VK_NULL_HANDLE)
				return;

			// Some handles will need a custom destroy function, like CommandBuffer
			// If there's no mDestroyFunc the derived handle class is trusted to handle destruction
			if (mDestroyFunc == nullptr)
				return;

			assert(mDevice);
			assert(mDestroyFunc);
			assert(mHandle);

			mDestroyFunc(mDevice, mHandle, nullptr);

			mHandle = VK_NULL_HANDLE;

		}

		Handle(Device* device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> destroyFunction)
		{
			mDevice = device->GetVkDevice();
			mDestroyFunc = destroyFunction;
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

		VkDevice GetDevice()
		{
			return mDevice;
		}

		T mHandle = VK_NULL_HANDLE;
	private:
		std::function<void(VkDevice, T, VkAllocationCallbacks*)> mDestroyFunc;
		VkDevice mDevice = VK_NULL_HANDLE;
	};
}
