#include "core/ObjectManager.h"
#include "vulkan/VulkanDebug.h"

namespace Utopian
{
	ObjectManager::ObjectManager()
		: mNextAvailableId(0)
	{

	}

	ObjectManager::~ObjectManager()
	{

	}

	void ObjectManager::RegisterObject(const SharedPtr<Object>& object)
	{
		object->Initialize(mNextAvailableId);

		mObjects[mNextAvailableId] = object;
		mNextAvailableId++;
	}

	Object& ObjectManager::GetObjectHandle(uint32_t id)
	{
		auto iterFind = mObjects.find(id);

		if (iterFind != mObjects.end())
			return *iterFind->second;

		//return nullptr;
	}

	void ObjectManager::PrintObjects()
	{
		for (auto& entry : mObjects)
		{
			Utopian::Vk::VulkanDebug::ConsolePrint("Name: " + entry.second->GetName() + ", Id: " + std::to_string(entry.second->GetId()) + ", Use count: " + std::to_string(entry.second.use_count()));
		}
	}
}
