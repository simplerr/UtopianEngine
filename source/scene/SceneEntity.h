#pragma once
#include <string>
#include <type_traits>
#include <vector>
#include "scene/SceneComponent.h"
#include "Common.h"
#include "scene/Object.h"
#include "scene/ObjectManager.h"
#include "scene/World.h"

using namespace std;

namespace Scene
{
	class CTransform;

	class SceneEntity : public Object
	{
	public:
		SceneEntity(string name);

		static SharedPtr<SceneEntity> Create(string name);

		template<class T, class... Args>
		T* AddComponent(Args &&... args)
		{
			static_assert((std::is_base_of<SceneComponent, T>::value), "Specified type is not a valid Component.");

			SharedPtr<T> newComponent(new T(this, std::forward<Args>(args)...));

			ObjectManager::Instance().RegisterObject(newComponent);
			World::Instance().NotifyComponentCreated(newComponent.get());

			mComponents.push_back(newComponent.get());

			return newComponent.get();
		}

		template <typename T>
		T* GetComponent() const
		{
			static_assert((std::is_base_of<SceneComponent, T>::value), "Specified type is not a valid Component.");

			T* component = GetComponent<T>(0);// T::GetType());

			return component;
		}

		template <typename T>
		T* GetComponent(uint32_t type) const
		{
			for(auto& entry : mComponents)
			{
				if (entry->GetType() == T::GetStaticType())
					return dynamic_cast<T*>(entry);
			}

			return nullptr;
		}

		const Transform& GetTransform() const;

	private:
		vector<SceneComponent*> mComponents;
		CTransform* mT = nullptr;
		bool mHasTransform;
	};
}
