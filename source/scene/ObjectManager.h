#pragma once
#include <map>
#include "scene/Object.h"
#include "utility/Module.h"
#include "utility/Common.h"

namespace Utopian
{
	/* 
		ObjectManager is responsible for the lifetime of every Object.
	*/
	class ObjectManager : public Module<ObjectManager>
	{
	public:
		ObjectManager();
		~ObjectManager();

		void RegisterObject(const SharedPtr<Object>& object);

		Object& GetObjectHandle(uint32_t id);

		void PrintObjects();
	private:
		std::map<uint32_t, SharedPtr<Object>> mObjects;
		uint32_t mNextAvailableId;
	};
}
