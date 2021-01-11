#include "core/Profiler.h"
#include "core/renderer/Renderer.h"
#include "imgui/imgui.h"
#include "core/renderer/ImGuiRenderer.h"
#include "utility/Timer.h"
#include "utility/math/Helpers.h"
#include "core/Input.h"
#include "vulkan/handles/Device.h"

namespace Utopian
{
	Profiler& gProfiler()
	{
		return Profiler::Instance();
	}

	Profiler::Profiler(Vk::VulkanApp* vulkanApp)
	{
      mEnabled = true;
      mProfilerWindow.gpuGraph.SetMaxFrameTime(2.0f);
      mFrametimePlot.Configure(20, 30.0f);
      mFpsPlot.Configure(20, 30.0f);
      mMemoryUsagePlot.Configure(20, 300.0f);
      mVulkanApp = vulkanApp;
	}

	Profiler::~Profiler()
	{
	}

	void Profiler::Update()
	{
      if (gInput().KeyPressed('P'))
         mEnabled = !mEnabled;
     
      if (mEnabled)
      {
         static uint32_t period;
         if (period % 50 == 0)
            mProfilerWindow.gpuGraph.LoadFrameData(mProfilerTasks.data(), mProfilerTasks.size());

         period++;

         /* GPU frame time */
         float frameTime = 0.0f;
         for (auto& iter : mProfilerTasks)
            frameTime += (float)iter.GetLength() * 1000.0f;

         mFrametimePlot.AddData(frameTime);

         /* GPU memory usage */
         VmaBudget budget = mVulkanApp->GetDevice()->GetMemoryBudget(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
         mMemoryUsagePlot.AddData(budget.allocationBytes / 1000000.0f);

         /* FPS */
         mFpsPlot.AddData((float)gTimer().GetFPS());

         ImGui::Begin("Profiling data");
         mFrametimePlot.Render("GPU frame time", 0.0f, 2.0f);
         mFpsPlot.Render("FPS", 0.0f, 100.0f);
         mMemoryUsagePlot.Render("GPU memory usage (MB)", 900.0f, 1100.0f);
         ImGui::End();

         mProfilerWindow.Render();
      }
	}

   void Profiler::AddProfilerTask(const std::string& name, float start, float end, const glm::vec4& color)
   {
      bool found = false;
      for (auto& task : mProfilerTasks)
      {
         if (task.name == name)
         {
            task.startTime = 0;
            task.endTime = (end - start) / 1000.0f; // To seconds
            found = true;
            break;
         }
      }

      if (!found)
      {
         LegitProfiler::ProfilerTask task;
         task.name = name;
         task.startTime = 0;
         task.endTime = (end - start) / 1000.0f; // To seconds
         task.color = ((uint32_t(color.a * 255) << 24) | (uint32_t(color.b * 255) << 16) | (uint32_t(color.g * 255) << 8) | (uint32_t(color.r * 255)));
            
         mProfilerTasks.push_back(task);
      }
   }

   void Profiler::SetEnabled(bool enabled)
   {
      mEnabled = enabled;
   }

   MiniPlot::MiniPlot()
   {
      mNumMaxEntries = 20;
      mUpdateIntervalMs = 30.0f;
   }

   MiniPlot::~MiniPlot()
   {
   }

   void MiniPlot::Configure(uint32_t maxEntries, float updateIntervalMs)
   {
      mNumMaxEntries = maxEntries;
      mUpdateIntervalMs = updateIntervalMs;
   }

   void MiniPlot::Render(const std::string& label, float minScale, float maxScale)
   {
      if (mEntries.size() != 0)
      {
         ImGui::Text("%s: %.2f", label.c_str(), mEntries.at(mEntries.size() - 1));
         ImGui::PlotLines("", mEntries.data(), (int)mEntries.size(), 0, "", minScale, maxScale, ImVec2(0.0f, 40.0f));
      }
   }

   void MiniPlot::AddData(float data)
   {
      float currentTimeMs = gTimer().GetTime();
      if (currentTimeMs - mLastAddTimestamp > mUpdateIntervalMs)
      {
         mEntries.push_back(data);
         if (mEntries.size() > mNumMaxEntries)
            mEntries.erase(mEntries.begin());

         mLastAddTimestamp = currentTimeMs;
      }
   }

   void MiniPlot::SetUpdateInterval(float intervalMs)
   {
      mUpdateIntervalMs = intervalMs;
   }
}
