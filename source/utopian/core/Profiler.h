#pragma once
#include <vector>
#include "utility/Module.h"
#include "LegitProfiler/ImGuiProfilerRenderer.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian
{
   class MiniPlot
   {
   public:
      MiniPlot();
      ~MiniPlot();

      void Render(const std::string& label, float minScale, float maxScale);
      void Configure(uint32_t maxEntries, float updateIntervalMs);
      void AddData(float data);
      void SetUpdateInterval(float intervalMs);
   private:
      std::vector<float> mEntries;
      uint32_t mNumMaxEntries = 20;
      float mUpdateIntervalMs = 300.0f;
      float mLastAddTimestamp = 0.0f;
   };

	class Profiler : public Module<Profiler>
	{
	public:
		Profiler(Vk::VulkanApp* vulkanApp);
		~Profiler();

		void Update();
		void AddProfilerTask(const std::string& name, float start, float end, const glm::vec4& color);
		void SetEnabled(bool enabled);
	private:
		LegitProfiler::ProfilersWindow mProfilerWindow;
		std::vector<LegitProfiler::ProfilerTask> mProfilerTasks;
		bool mEnabled;
		MiniPlot mFrametimePlot;
		MiniPlot mMemoryUsagePlot;
		MiniPlot mFpsPlot;
		std::vector<float> mFrameTimes;
		Vk::VulkanApp* mVulkanApp;
		bool mVisible;
	};

	Profiler& gProfiler();
}
