#pragma once

/* 
   Contains includes that are shared between most of the jobs
*/

#include "core/renderer/Renderable.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RendererUtility.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/Effect.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/Vertex.h"
#include "vulkan/TextureLoader.h"
#include "core/ModelLoader.h"
#include "vulkan/EffectManager.h"
#include "core/renderer/ScreenQuadRenderer.h"
