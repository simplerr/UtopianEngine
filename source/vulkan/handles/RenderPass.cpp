#include <array>
#include "RenderPass.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace VulkanLib
{
	RenderPass::RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat)
		: Handle(device, vkDestroyRenderPass)
	{
		// Descriptors for the attachments used by this renderpass
		std::array<VkAttachmentDescription, 2> attachments = {};

		// Color attachment
		attachments[0].format = colorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;									
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;						
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
																	
		// Depth attachment											
		attachments[1].format = depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;						
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	

		// Setup attachment references																			
		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;											
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;									
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		

		// Setup a single subpass reference																			
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;							
		subpassDescription.pColorAttachments = &colorReference;				
		subpassDescription.pDepthStencilAttachment = &depthReference;	
		subpassDescription.inputAttachmentCount = 0;				
		subpassDescription.pInputAttachments = nullptr;			
		subpassDescription.preserveAttachmentCount = 0;		
		subpassDescription.pPreserveAttachments = nullptr;								
		subpassDescription.pResolveAttachments = nullptr;							

		std::array<VkSubpassDependency, 2> dependencies;

		// First dependency at the start of the renderpass
		// Does the transition from final to initial layout 
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						
		dependencies[0].dstSubpass = 0;										
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Second dependency at the end the renderpass
		// Does the transition from the initial to the final layout
		dependencies[1].srcSubpass = 0;									
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;			
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());	
		renderPassInfo.pAttachments = attachments.data();						
		renderPassInfo.subpassCount = 1;									
		renderPassInfo.pSubpasses = &subpassDescription;				
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());	
		renderPassInfo.pDependencies = dependencies.data();							

		VulkanDebug::ErrorCheck(vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &mHandle));
	}
}