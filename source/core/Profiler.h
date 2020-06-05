#pragma once
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
	private:
		LegitProfiler::ProfilersWindow mProfilerWindow;
	};

	Profiler& gProfiler();
}
