#include "core/renderer/PixelDebugJob.h"
#include "core/renderer/GeometryThicknessJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "ImGuiRenderer.h"
#include "Input.h"

namespace Utopian
{
	PixelDebugJob::PixelDebugJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{

	}

	PixelDebugJob::~PixelDebugJob()
	{
	}

	void PixelDebugJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mRenderTarget->SetClearColor(0, 1, 0, 1);
		mRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/pixel_debug/pixel_debug.frag";

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mEffect->CreatePipeline();

		mMouseInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mOutputBuffer.Create(mDevice, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		mEffect->BindUniformBuffer("UBO_input", &mMouseInputBlock);
		mEffect->BindStorageBuffer("UBO_output", mOutputBuffer.GetDescriptor());

		// Update this to the image to read pixel values from
		GeometryThicknessJob* geometryThicknessJob = static_cast<GeometryThicknessJob*>(jobs[JobGraph::GEOMETRY_THICKNESS_INDEX]);
		mEffect->BindCombinedImage("debugSampler", geometryThicknessJob->geometryThicknessImage.get(), mRenderTarget->GetSampler());
	}

	void PixelDebugJob::Render(const JobInput& jobInput)
	{
		glm::vec2 mousePos = gInput().GetMousePosition();
		mMouseInputBlock.data.mousePosUV = glm::vec2(mousePos.x / mWidth, mousePos.y / mHeight);
		mMouseInputBlock.UpdateMemory();

		mRenderTarget->Begin("Pixel debug pass", glm::vec4(0.5, 1.0, 0.3, 1.0));
		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());

		// Retrieve pixel value
		float* mapped;
		mOutputBuffer.MapMemory(0, sizeof(glm::vec4), 0, (void**)&mapped);
		mOutputBuffer.data.pixelValue = *(glm::vec4*)mapped;
		mOutputBuffer.UnmapMemory();
	}

	void PixelDebugJob::Update()
	{
		glm::vec4 pixelValue = mOutputBuffer.data.pixelValue;
		ImGuiRenderer::BeginWindow("Pixel debug", glm::vec2(500.0f, 10.0f), 400.0f);
		ImGuiRenderer::TextV("R: %f, G: %f, B: %f, A: %f", pixelValue.x, pixelValue.y, pixelValue.z, pixelValue.w);
		ImGuiRenderer::EndWindow();
	}
}
