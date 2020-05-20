#pragma once

#include <vulkan/vulkan.h>

#define SET_0 0
#define SET_1 1
#define SET_2 2
#define SET_3 3
#define SET_4 4

#define BINDING_0 0
#define BINDING_1 1
#define BINDING_2 2
#define BINDING_3 3
#define BINDING_4 4

// Forward declarations
namespace Utopian::Vk
{
	// Handles
	class Buffer;
	class CommandBuffer;
	class CommandPool;
	class ComputePipeline;
	class DescriptorSet;
	class DescriptorPool;
	class DescriptorSetLayout;
	class Fence;
	class FrameBuffers;
	class Image;
	class Instance;
	class Pipeline2; // Todo: rename
	class Effect;
	class PipelineLegacy;
	class PipelineLayout;
	class Queue;
	class RenderPass;
	class Sampler;
	class Semaphore;
	class Texture;
	class Texture;
	class Pipeline;
	class QueryPool;

	// Other
	class Device;
	class Mesh;
	class ModelLoader;
	class PipelineInterface;
	class VulkanApp;
	class RenderTarget;
	class ScreenQuadRenderer;
	class ShaderBuffer;
	class ShaderFactory;
	class StaticModel;
	class TextOverlay;
	class TextureLoader;
	struct Vertex;
	class VertexAttribute;
	class VertexDescription;
	class VulkanBase;
	class EffectLegacy;
	class Shader;
	class ScreenQuad;
	class BasicRenderTarget;
	class CubeMapTexture;
	class TextureArray;
}

namespace Utopian
{
	// Should be moved
	class Window;
	class Camera;
}
