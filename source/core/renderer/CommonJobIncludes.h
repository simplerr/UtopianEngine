#pragma once

/* 
	Contains includes that are shared between most of the jobs
*/

#include "core/renderer/Renderable.h"
#include "core/renderer/RenderingManager.h"
#include "core/renderer/RendererUtility.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/Effect.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/Vertex.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/EffectManager.h"
#include "vulkan/ScreenQuadUi.h"
