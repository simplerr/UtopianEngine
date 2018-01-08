#include <glm/gtc/matrix_transform.hpp>
#include "scene/ActorRenderer.h"
#include "scene/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "Camera.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"

namespace Scene
{
	ActorRenderer::ActorRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
	{
		mRenderer = renderer;
		mCamera = camera;
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// NOTE: This must run before PhongEffect::Init()
		// Create the fragment shader uniform buffer
		Vulkan::Light light;
		light.SetMaterials(vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(1, -1, 1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.8f, 1.0f);
		light.SetType(Vulkan::LightType::DIRECTIONAL_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mPhongEffect.per_frame_ps.lights.push_back(light);

		light.SetMaterials(vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(-1, -1, -1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.5f, 1.0f);
		light.SetType(Vulkan::LightType::SPOT_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mPhongEffect.per_frame_ps.lights.push_back(light);

		// Important to call this before Create() since # lights affects the total size
		mPhongEffect.per_frame_ps.constants.numLights = mPhongEffect.per_frame_ps.lights.size();

		mPhongEffect.Init(mRenderer);
	}

	ActorRenderer::~ActorRenderer()
	{

	}

	void ActorRenderer::RenderAll()
	{
		// From Renderer.cpp
		if (mCamera != nullptr)
		{
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.viewMatrix = mCamera->GetView();
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.eyePos = mCamera->GetPosition();
		}

		mPhongEffect.UpdateMemory(mRenderer->GetDevice());

		mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		for (auto& renderable : mRenderables)
		{
			Vulkan::StaticModel* model = renderable->GetModel();
			mPhongEffect.SetPipeline(0);

			for (Vulkan::Mesh* mesh : model->mMeshes)
			{
				mCommandBuffer->CmdBindPipeline(mPhongEffect.GetPipeline());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();

				VkDescriptorSet descriptorSets[3] = { mPhongEffect.mCameraDescriptorSet->descriptorSet, mPhongEffect.mLightDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(mCommandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPhongEffect.GetPipelineLayout(), 0, 3, descriptorSets, 0, NULL);

				// Push the world matrix constant
				PushConstantBlock pushConstantBlock;
				Transform transform = renderable->GetTransform();
				mat4 world;
				world = glm::translate(world, transform.GetPosition());
				world = glm::scale(world, transform.GetScale());
				pushConstantBlock.world = world;

				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				mCommandBuffer->CmdPushConstants(&mPhongEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				mCommandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
				mCommandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		mCommandBuffer->End();
	}

	void ActorRenderer::AddRenderable(Renderable* renderable)
	{
		mRenderables.push_back(renderable);
	}
}