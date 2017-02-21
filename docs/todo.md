### TODO

18/2
* Properly used the correct texture for loaded models
* Do not use the same DescriptorSet for every Pipeline
* Add debug prints for entity systems
* Add text overlay rendering
* Experiment with terrain rendering
* Use RenderDoc

* MeshComponents should be able to change Pipeline at runtime
* Optimize TransformComponent::RebuildWorldMatrix()
* Remove use of mHandle outside the Handle classes
* Create ColorImage and DepthStencilImage classes
* Remove VulkanBase::CreateBuffer
* Use proper way to setup the rendering queue, instead of hardcoding it to 0
* Add device information to Device, using vkGetPhysicalDeviceProperties() etc
* Device::FlushCommandBuffer should use fences instead of VkQueueWaitIdle()
