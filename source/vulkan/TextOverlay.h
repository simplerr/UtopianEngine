#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <vector>
#include <vulkan/vulkan.h>

#include "../external/stb/stb_font_consolas_24_latin1.inl"

// Defines for the STB font used
// STB font files can be found at http://nothings.org/stb/font/
#define STB_FONT_NAME stb_font_consolas_24_latin1
#define STB_FONT_WIDTH STB_FONT_consolas_24_latin1_BITMAP_WIDTH
#define STB_FONT_HEIGHT STB_FONT_consolas_24_latin1_BITMAP_HEIGHT 
#define STB_FIRST_CHAR STB_FONT_consolas_24_latin1_FIRST_CHAR
#define STB_NUM_CHARS STB_FONT_consolas_24_latin1_NUM_CHARS

// Max. number of chars the text overlay buffer can hold
#define TEXTOVERLAY_MAX_CHAR_COUNT 2048

namespace VulkanLib
{
	class Device;
	class Renderer;
	class CommandBuffer;

	// Mostly self-contained text overlay class
	class TextOverlay
	{
	public:
		enum TextAlign { alignLeft, alignCenter, alignRight };

		bool visible = true;

		TextOverlay(Renderer* renderer,
			Device *vulkanDevice,
			VkQueue queue,
			std::vector<VkFramebuffer> &framebuffers,
			VkFormat colorformat,
			VkFormat depthformat,
			uint32_t *framebufferwidth,
			uint32_t *framebufferheight,
			std::vector<VkPipelineShaderStageCreateInfo> shaderstages);
		
		~TextOverlay();

		// Prepare all vulkan resources required to render the font
		// The text overlay uses separate resources for descriptors (pool, sets, layouts), pipelines and command buffers
		void prepareResources();
		
		// Prepare a separate pipeline for the font rendering decoupled from the main application
		void preparePipeline();

		// Prepare a separate render pass for rendering the text as an overlay
		void prepareRenderPass();

		// Map buffer 
		void beginTextUpdate();

		// Add text to the current buffer
		// todo : drop shadow? color attribute?
		void addText(std::string text, float x, float y, TextAlign align);
		
		// Unmap buffer and update command buffers
		void endTextUpdate();

		// Needs to be called by the application
		void updateCommandBuffers();

		// Submit the text command buffers to a queue
		// Does a queue wait idle
		void submit(VkQueue queue, uint32_t bufferindex);
	private:
		// My code
		Device* vulkanDevice;
		Renderer* mRenderer;
		CommandBuffer* mCommandBuffer;

		// Saschas code 
		VkQueue queue;
		VkFormat colorFormat;
		VkFormat depthFormat;

		uint32_t *frameBufferWidth;
		uint32_t *frameBufferHeight;

		VkSampler sampler;
		VkImage image;
		VkImageView view;
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		VkDeviceMemory imageMemory;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
		VkRenderPass renderPass;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> cmdBuffers;
		std::vector<VkFramebuffer*> frameBuffers;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		// Pointer to mapped vertex buffer
		glm::vec4 *mapped = nullptr;
		stb_fontchar stbFontData[STB_NUM_CHARS];
		uint32_t numLetters;
	};
}
