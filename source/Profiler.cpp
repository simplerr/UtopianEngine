#include "core/Profiler.h"
#include "imgui/imgui.h"
#include "ImGuiRenderer.h"
#include "utility/Timer.h"
#include "utility/math/Helpers.h"
#include "Input.h"

namespace Utopian
{
	Profiler& gProfiler()
	{
		return Profiler::Instance();
	}

	Profiler::Profiler()
	{
      mEnabled = true;
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
}
