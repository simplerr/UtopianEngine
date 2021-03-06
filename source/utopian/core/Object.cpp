#include "core/Object.h"

namespace Utopian
{
   Object::Object()
   {

   }

   Object::Object(std::string name)
   {
      SetName(name);
   }
   
   void Object::Initialize(uint32_t id)
   {
      SetId(id);
   }

   void Object::SetName(std::string name)
   {
      mName = name;
   }

   void Object::SetId(uint32_t id)
   {
      mId = id;
   }

   std::string Object::GetName()
   {
      return mName;
   }

   uint32_t Object::GetId()
   {
      return mId;
   }
}