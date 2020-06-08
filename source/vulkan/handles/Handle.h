#pragma once

#include <cassert>
#include <functional>
#include "vulkan/handles/Device.h" 
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian::Vk
{
	/**
	 * Base class for all the Vulkan wrappers which contains their
	 * Vulkan object and handles their destruction.
	 */
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

		/** Returns the Vulkan handle. */
		T GetVkHandle() const
		{
			return mHandle;
		}

		/** Returns a pointer to the Vulkan handle. */
		T* GetVkHandlePtr()
		{
			return &mHandle;
		}

		/** Returns the Vulkan device. */
		VkDevice GetVkDevice() const
		{
			return mDevice->GetVkDevice();
		}

		/** Returns device wrapper. */
		Device* GetDevice()
		{
			return mDevice;
		}

		std::string GetDebugName() const
		{
			return mName;
		}

		/** Sets the device. */
		void SetDevice(Device* device)
		{
			mDevice = device;
		}

		void SetDebugName(std::string name)
		{
			mName = name;
		}

	protected:
		T mHandle = VK_NULL_HANDLE;
	private:
		std::function<void(VkDevice, T, VkAllocationCallbacks*)> mDestroyFunc;
		Device* mDevice = nullptr;
		std::string mName = "Vulkan Handle";
	};
}
