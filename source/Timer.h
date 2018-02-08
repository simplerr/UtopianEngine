#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

namespace Utopian
{
	class Timer
	{
	public:
		void		FrameBegin();
		uint32_t	FrameEnd();				// Returns the FPS if 1000.0f milliseconds have passed

		uint32_t	GetFPS();
		float		GetElapsedTime();

		void		PrintLog(std::ofstream& fout);
		void		ResetLifetimeCounter();
	private:
		std::chrono::high_resolution_clock::time_point mFrameBegin;

		float		mFrameTimer = 1.0f;		// Last frame time, measured using a high performance timer (if available)
		uint32_t	mFrameCounter = 0;		// Frame counter to display fps
		float		mTimer = 0.0f;			// Defines a frame rate independent timer value clamped from -1.0...1.0
		float		mTimerSpeed = 0.25f;	// Multiplier for speeding up (or slowing down) the global timer
		float		mFpsTimer = 0.0f;		// FPS timer (one second interval)
		float		mLifetimeTimer = 0.0f;
		uint32_t	mFramesPerSecond;

		std::vector<float> mFpsLog;
	};
}	// VulkanLib namespace
