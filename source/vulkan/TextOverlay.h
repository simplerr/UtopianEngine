#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <vector>
#include "vulkan/VulkanInclude.h"

#include "../../external/stb/stb_font_consolas_24_latin1.inl"

// Defines for the STB font used
// STB font files can be found at http://nothings.org/stb/font/
#define STB_FONT_NAME stb_font_consolas_24_latin1
#define STB_FONT_WIDTH STB_FONT_consolas_24_latin1_BITMAP_WIDTH
#define STB_FONT_HEIGHT STB_FONT_consolas_24_latin1_BITMAP_HEIGHT 
#define STB_FIRST_CHAR STB_FONT_consolas_24_latin1_FIRST_CHAR
#define STB_NUM_CHARS STB_FONT_consolas_24_latin1_NUM_CHARS

// Max. number of chars the text overlay buffer can hold
#define TEXTOVERLAY_MAX_CHAR_COUNT 2048

namespace Utopian::Vk
{
	// Mostly self-contained text overlay class
	class TextOverlay
	{
	public:
		enum TextAlign {
			ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT 
		};

		TextOverlay(Renderer* renderer);
		~TextOverlay();

		// Map buffer 
		void BeginTextUpdate();

		// Add text to the current buffer
		// todo : drop shadow? color attribute?
		void AddText(std::string text, float x, float y, TextAlign align);
		void AddText(std::string text, glm::vec3 vec, float x, float y, TextAlign align);
		void AddText(std::string text, glm::vec4 vec, float x, float y, TextAlign align);
		void AddText(std::string text, glm::mat4 mat, float x, float y, TextAlign align);
		
		// Unmap buffer and update command buffers
		void EndTextUpdate();

		// Needs to be called by the application
		void UpdateCommandBuffers();

		void ToggleVisible();
		bool IsVisible();
	private:
		// My code
		Device* vulkanDevice;
		Renderer* mRenderer;
		CommandBuffer* mCommandBuffer;
		RenderPass* mRenderPass;
		Pipeline* mPipeline;
		Texture* mTexture;
		PipelineLayout* mPipelineLayout;
		VertexDescription* mVertexDescription;
		Buffer* mVertexBuffer;

		// Pointer to mapped vertex buffer
		glm::vec4 *mapped = nullptr;
		stb_fontchar stbFontData[STB_NUM_CHARS];
		uint32_t numLetters;

		bool mVisible = true;

		// How much to offset debug print from header print
		const float y_offset = 15.0f;
	};
}
