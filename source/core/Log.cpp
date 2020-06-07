#include <iostream>
#include <windows.h>
#include "core/Log.h"
#include "utility/Timer.h"

namespace Utopian
{
   Log& gLog()
   {
      return Log::Instance();
   }

   Log::Log()
   {
      SetupConsole("Utopian Engine (v0.1)");
   }

   Log::~Log()
   {
   }

   void Log::SetupConsole(std::string title)
   {
#if defined(_WIN32)
      AllocConsole();
      AttachConsole(GetCurrentProcessId());
      freopen("CON", "w", stdout);
      SetConsoleTitle(TEXT(title.c_str()));
#endif
   }

   void Log::AddMessage(std::string message)
   {
      std::cout << message << std::endl;

      if (mUserLogCallback != nullptr)
         mUserLogCallback(message);
   }
}
