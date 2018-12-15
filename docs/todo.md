# Todo

### Technical debt
* Unify namespace usage
* Unify Vulkan handle classes
* Port remaining legacy effects
* Refactor Renderer/VulkanBase classses
* Correct folder structure
* Investigate performance issues
* Vulkan synchronization

### New features
* Instancing
* Vegetation rendering
* Finish shadow mapping
* Glow effect
* Separate effect for terrain (multitexturing)
* Particle effects
* Marching cubes terrain
* Integrate physics

# Old todo

18/2
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

30/9
* Properly handle ComputePipeline in the Effect base class
* Unify interfaces in CommandBuffer

2/10
* Effect::mVertexDescription should not be a pointer

3/10
* Remove DescriptorPool and mTextureDescriptorSetLayout from Renderer

6/12
* colorImageViews in FrameBuffers.h might depend a bit too much on the SwapChain class
	Perhaps pass an Image* instead similar to depthStencil
* TextureLoader and Texture should reuse the code in Image

9/12
* BindCombinedImage should also supporting updating the bound descriptor like BindStorageBuffer does.

29/12
* Create debug normal effect


