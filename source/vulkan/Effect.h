#pragma once

#include <vulkan\vulkan.h>
#include <map>
#include "vulkan/PipelineInterface.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"

namespace Utopian::Vk
{
	enum EffectType {
		PHONG = 0,
		COLOR
	};

	class Mat
	{
	public:
		Mat() {
			effectType = EffectType::PHONG;
			variation = 0;
		}

		Mat(EffectType effectType, uint32_t variation) {
			this->effectType = effectType;
			this->variation = variation;
		}
	//private:
		uint32_t effectType;
		uint32_t variation;
	};

	/* 
	 * Base class for all effects.
	 *
	 * \note The naming of the descriptors in C++ and GLSL should be the same for readability.
	 *
	*/
	class Effect
	{
	public:
		Effect();

		void Init(Renderer* renderer);

		virtual void CreateDescriptorPool(Device* device) = 0;
		virtual void CreateVertexDescription(Device* device) = 0;
		virtual void CreatePipelineInterface(Device* device) = 0;
		virtual void CreateDescriptorSets(Device* device) = 0;
		virtual void CreatePipeline(Renderer* renderer) = 0;
		virtual void UpdateMemory(Device* device) = 0;

		void SetPipeline(uint32_t pipelineType);
		VkPipelineLayout GetPipelineLayout();
		DescriptorSetLayout* GetDescriptorSetLayout(uint32_t descriptorSet);
		Pipeline2* GetPipeline(uint32_t variation);
		DescriptorPool* GetDescriptorPool();
		VertexDescription GetVertexDescription();

	protected:
		DescriptorPool* mDescriptorPool;
		PipelineInterface mPipelineInterface;
		VertexDescription mVertexDescription;
		std::map<int, Pipeline2*> mPipelines;
		uint32_t mActivePipeline;
	};
}
