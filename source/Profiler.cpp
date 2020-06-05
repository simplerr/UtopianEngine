#include "core/Profiler.h"
#include "imgui/imgui.h"
#include "ImGuiRenderer.h"

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
      mProfilerWindow.Render();
	}
}
