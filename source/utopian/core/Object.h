#pragma once
#include <string>

namespace Utopian
{
   class Object
   {
   public:
      Object();
      Object(std::string name);

      void Initialize(uint32_t id);

      void SetName(std::string name);
      void SetId(uint32_t id);

      std::string GetName();
      uint32_t GetId();

   private:
      std::string mName;
      uint32_t mId;
   };
}
