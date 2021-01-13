#include "Timer.h"
#include <fstream>

namespace Utopian
{
	Timer::Timer()
	{
		mStartTime = gTimer().GetTimestamp();
	}

	Timer& gTimer()
	{
		return Timer::Instance();
	}

	void Timer::CalculateFrameTime()
	{
		// Raw frame time calculation
		static Timestamp previousTimestamp = gTimer().GetTimestamp();
		mFrameTime = gTimer().GetElapsedTime(previousTimestamp);
		previousTimestamp = gTimer().GetTimestamp();

		// Average frame time calculation
		static Timestamp previousSampleTimestamp = gTimer().GetTimestamp();
		if (gTimer().GetElapsedTime(previousSampleTimestamp) > mFrameTimeSamplePeriod)
		{
			mAverageFrameTime = 0.0;
			for (uint32_t i = 0u; i < mFrameTimeList.size(); i++)
			{
				mAverageFrameTime += mFrameTimeList[i];
			}

			mAverageFrameTime /= mFrameTimeList.size();
			mFrameTimeList.clear();

			previousSampleTimestamp = gTimer().GetTimestamp();
		}

		mFrameTimeList.push_back(mFrameTime);
	}

	double Timer::GetFrameTime() const
	{
		return mFrameTime;
	}
	
	double Timer::GetFps() const
	{
		return 1000.0f / GetFrameTime();
	}

	double Timer::GetAverageFrameTime() const
	{
		return mAverageFrameTime;
	}
	
	double Timer::GetAverageFps() const
	{
		return 1000.0f / GetAverageFrameTime();
	}

	double Timer::GetTime() const
	{
		return GetElapsedTime(mStartTime);
	}

	Timestamp Timer::GetTimestamp() const
	{
		return std::chrono::high_resolution_clock::now();
	}

	double Timer::GetElapsedTime(Timestamp timestamp) const
	{
		Timestamp now = std::chrono::high_resolution_clock::now();
		double elapsedTime = std::chrono::duration<double, std::milli>(now - timestamp).count();
		return elapsedTime;
	}

}	// VulkanLib namespace