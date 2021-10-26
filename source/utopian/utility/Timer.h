#pragma once

#include <chrono>
#include <cstdint>
#include <vector>
#include "utility\Module.h"

namespace Utopian
{
   typedef std::chrono::high_resolution_clock::time_point Timestamp;

   class Timer : public Module<Timer>
   {
   public:
      Timer();

      /** Should be called after swapchain presentation. */
      void CalculateFrameTime();

      /** Returns the raw frame time for the current frame. */
      double GetFrameTime() const;

      /** Returns the raw FPS for the current frame. */
      double GetFps() const;

      /** Returns the average frame time over 1s. */
      double GetAverageFrameTime() const;

      /** Returns the average FPS over 1s. */
      double GetAverageFps() const;

      /** Returns the time since application start. */
      double GetTime() const;

      /** Returns the current timestamp in milliseconds. */
      Timestamp GetTimestamp() const;

      /** Returns the elapsed time between now and timestamp in milliseconds. */
      double GetElapsedTime(Timestamp timestamp) const;

   private:
      Timestamp mStartTime;
      std::vector<double> mFrameTimeList;
      double mFrameTime = 0.0;
      double mAverageFrameTime = 0.0;
      float mFrameTimeSamplePeriod = 1000.0f;
      double mDeltaTime = 0.0;
   };

   Timer& gTimer();
}
