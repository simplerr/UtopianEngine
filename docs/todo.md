### TODO

* MeshComponents should be able to change Pipeline at runtime
* Do not use the same DescriptorSet for every Pipeline
* Create PickingSystem
* Optimize TransformComponent::RebuildWorldMatrix()
* Remove use of mHandle outside the Handle classes
* Create ColorImage and DepthStencilImage classes
* Remove VulkanBase::CreateBuffer
* Use proper way to setup the rendering queue, instead of hardcoding it to 0
* Add device information to Device, using vkGetPhysicalDeviceProperties() etc
* Device::FlushCommandBuffer should use fences instead of VkQueueWaitIdle()
* Use RenderDoc

