#pragma once
#include <vector>
#include "utility/Module.h"
#include "LegitProfiler/ImGuiProfilerRenderer.h"

namespace Utopian
{
	class Profiler : public Module<Profiler>
	{
	public:
		Profiler();
		~Profiler();

		void Update();
      void AddProfilerTask(const std::string& name, float start, float end, const glm::vec4& color);
	private:
		LegitProfiler::ProfilersWindow mProfilerWindow;
      std::vector<LegitProfiler::ProfilerTask> mProfilerTasks;
      bool mEnabled;
	};

	Profiler& gProfiler();
}
