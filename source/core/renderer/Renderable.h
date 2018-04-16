#pragma once
#include <glm/glm.hpp>
#include "core/SceneNode.h"
#include "core/components/Component.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/PhongEffect.h"

namespace Vk::Test
{
	class A
	{

	};
}

namespace Utopian
{
	class Actor;

	class Renderable : public SceneNode
	{
	public:
		Renderable();
		~Renderable();

		void Initialize();

		static SharedPtr<Renderable> Create();

		void SetModel(Utopian::Vk::StaticModel* model);
		void SetPipeline(Vk::PipelineType2 pipeline);
		void SetColor(glm::vec4 color);

		Utopian::Vk::StaticModel* GetModel();
		const BoundingBox GetBoundingBox() const;
		const Vk::PipelineType2 GetPipeline() const;
		const glm::vec4 GetColor() const;

	private:
		Utopian::Vk::StaticModel* mModel;
		Vk::PipelineType2 mPipeline;
		glm::vec4 mColor;
	};
}
