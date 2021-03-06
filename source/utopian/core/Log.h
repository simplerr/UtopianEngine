#pragma once
#include <string>
#include <functional>
#include <vector>
#include "utility/Module.h"

#define UTO_LOG(message, ...)                   \
   do                                           \
   {                                            \
      Utopian::gLog().AddMessage(message);      \
   } while (0)

namespace Utopian
{
   class Log : public Module<Log>
   {
   public:
      Log();
      ~Log();

      void SetupConsole(std::string title);
      void AddMessage(std::string message);

      template<class ...Args>
      void RegisterUserLogCallback(Args &&...args)
      {
         mUserLogCallback = std::bind(std::forward<Args>(args)...);

         for (const auto& entry : mStartupEntries)
            mUserLogCallback(entry);
      }

      void UnregisterUserCallback();

   private:
      std::function<void(std::string)> mUserLogCallback;
      std::vector<std::string> mStartupEntries;
   };

   Log& gLog();
}
