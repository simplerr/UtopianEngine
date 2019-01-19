#pragma once

#include <vector>
#include "core/renderer/SceneInfo.h"
#include "core/renderer/RenderSettings.h"

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;
	class BaseJob;
	class PerlinTerrain;	
	
	struct JobInput
	{
		JobInput(const SceneInfo& sceneInfo, const std::vector<BaseJob*>& jobs, const RenderingSettings& renderingSettings) 
			: sceneInfo(sceneInfo), jobs(jobs) , renderingSettings(renderingSettings) {

		}

		const SceneInfo& sceneInfo;
		const std::vector<BaseJob*>& jobs;
		const RenderingSettings& renderingSettings;
	};

	class BaseJob
	{
	public:
		BaseJob(Vk::Renderer* renderer, uint32_t width, uint32_t height) {
			mRenderer = renderer;
			mWidth = width;
			mHeight = height;
		}

		virtual ~BaseJob() {};

		/*
		 * If a job needs to query information from another job that's already added
		   it should be done inside of this function.
		*/
		virtual void Init(const std::vector<BaseJob*>& jobs) = 0;

		virtual void Render(Vk::Renderer* renderer, const JobInput& jobInput) = 0;
	protected:
		Vk::Renderer* mRenderer;
		uint32_t mWidth;
		uint32_t mHeight;
	};
}
