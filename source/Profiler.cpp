#include "core/Profiler.h"
#include "imgui/imgui.h"
#include "ImGuiRenderer.h"
#include "utility/Timer.h"
#include "utility/math/Helpers.h"

namespace Utopian
{
	Profiler& gProfiler()
	{
		return Profiler::Instance();
	}

	Profiler::Profiler()
	{
	}

	Profiler::~Profiler()
	{
	}

	void Profiler::Update()
	{
      #define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))

      std::vector<LegitProfiler::ProfilerTask> testTasks;
      LegitProfiler::ProfilerTask task;
      task.name = "Test task";
      task.color = RGBA_LE(0xf1c40fffu);
      task.startTime = 0.0f;
      task.endTime = Math::GetRandom(0.001f, 0.005f);;
      testTasks.push_back(task);

      task.name = "SSR task";
      task.color = RGBA_LE(0x010000ffu);
      task.startTime = task.endTime;
      task.endTime = task.startTime + Math::GetRandom(0.001f, 0.01f);
      testTasks.push_back(task);

      task.name = "Junk task";
      task.color = RGBA_LE(0x0188ccffu);
      task.startTime = task.endTime;
      task.endTime = task.startTime + Math::GetRandom(0.001f, 0.01f);
      testTasks.push_back(task);

      static uint32_t test;
      if (test % 100 == 0)
         mProfilerWindow.gpuGraph.LoadFrameData(testTasks.data(), testTasks.size());

      test++;
      mProfilerWindow.Render();
	}
}
