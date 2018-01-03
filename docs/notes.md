* What happened to the setupCmdBuffer?
* Might be a good idea to take a look at VK_EXT_DEBUG_MARKER_EXTENSION_NAME
* queueFamilyIndex is currenly hardcoded to 0 for the graphics queue, if a compute queue is to be added this needs to be updated
* Device that contains VkPhysicalDevice, VkDevice, VkPhysicalDeviceProperties etc.
* mSetupCmdBuffer seems to be removed and replaced with creating temporary command buffers instead
	- VulkanExampleBase::createCommandBuffer() and VulkanExampleBase::flushCommandBuffer()
* Image barriers seems to be removed as well
	- It seems like the memory barrier transitions are replaced with render pass dependencies
	- Commit: Fold attachment layout transitions into subpass (Refs #155)
	- See Triangle::setupRenderPass() for great comments


‒ Images must be moved to the right layout before usage.
	‒ This can be requested by:
	‒ injecting image barriers into command buffers
	‒ correct renderpass & subpass configuration

### Renderer
* Is able to let application code generate different command buffers each frame
	- This makes it impossible to swap to a non-Vulkan renderer!
	- Should Renderer be responsible for all command buffer generation then?
		- __How should the connection between MeshComponent and Renderer be?__
		- How should the connection between Terrain and Renderer be?
* Should not need to know of any application code 

* The `RenderSystem` is only one of many that needs to use Vulkan functionality
	- Making `RenderSystem` inherit from `renderer` would mean that other classes would depend on `RenderSystem`, `Terrain` for example
	- It's better if RenderSystem _has_ a pointer to `renderer`
	- `renderer` should let higher level classes generate command buffers to run each frame
		- How are they added?

RenderSystem::OnEntityAdded(entity)
{
	mRenderer->AddModelInstance(entity.model);	
}

RenderSystem::OnEntityRemoved(entity)
{
	mRenderer->RemoveModelInstance(entity.model);	
}

Editor/Terrain/World/RenderSystem
- Terrain is not related to the ECS at all
- Water is also not related to the ECS, same for grass
- Could be placed in World
- The responsibility of RenderSystem should ONLY be to handle rendering
of MeshComponents
- Load/Save should be placed in World
	- If world does not contain Entities this is false
- How does World communicate with RenderSystem? 

- PhysicsSystem needs Terrain::Intersect()
- EditorSystem needs Terrain::Intersect()

Ideas:
World has Terrain and RenderSystem

WorldRenderer renders World + RenderSystem (handles reflections, deferred etc.)

Other possible Components:
- BillboardComponent
- ParticleSystemComponent
- Should these be handled by RenderSystem or have separate Systems?