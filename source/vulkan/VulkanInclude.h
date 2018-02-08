#pragma once

#include <vulkan/vulkan.h>

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
	class Pipeline;
	class PipelineLayout;
	class Queue;
	class RenderPass;
	class Sampler;
	class Semaphore;
	class Texture;

	// Other
	class Device;
	class Mesh;
	class ModelLoader;
	class PipelineInterface;
	class Renderer;
	class RenderTarget;
	class ScreenGui;
	class ShaderBuffer;
	class ShaderManager;
	class StaticModel;
	class TextOverlay;
	class TextureLoader;
	struct Vertex;
	class VertexAttribute;
	class VertexDescription;
	class VulkanBase;
	class Effect;
	class Shader;
	class UIOverlay;

	// Should be moved
	class Camera;
}

namespace Utopian
{
	// Should be moved
	class Window;
}
