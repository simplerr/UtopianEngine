* What happened to the setupCmdBuffer?
* Might be a good idea to take a look at VK_EXT_DEBUG_MARKER_EXTENSION_NAME
* queueFamilyIndex is currenly hardcoded to 0 for the graphics queue, if a compute queue is to be added this needs to be updated
* VulkanDevice that contains VkPhysicalDevice, VkDevice, VkPhysicalDeviceProperties etc.
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