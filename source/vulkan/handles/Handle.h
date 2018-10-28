#pragma once

#include <cassert>
#include <functional>
#include "vulkan/Device.h" 
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
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

			mDestroyFunc(mDevice->GetVkDevice(), mHandle, nullptr);

			mHandle = VK_NULL_HANDLE;

		}

		Handle(Device* device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> destroyFunction)
		{
			mDevice = device;
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

		VkDevice GetVkDevice()
		{
			return mDevice->GetVkDevice();
		}

		Device* GetDevice()
		{
			return mDevice;
		}

		void SetDevice(Device* device)
		{
			mDevice = device;
		}

		T mHandle = VK_NULL_HANDLE;
	private:
		std::function<void(VkDevice, T, VkAllocationCallbacks*)> mDestroyFunc;
		Device* mDevice = nullptr;
	};
}
