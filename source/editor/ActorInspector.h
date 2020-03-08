#pragma once
#include <vector>

namespace Utopian
{
	class Actor;
	class ComponentInspector;

	class ActorInspector
	{
	public:
		ActorInspector();
		~ActorInspector();

		void UpdateUi();

		void SetActor(Actor* actor);
	private:
		void AddInspectors();
		void ClearInspectors();
		std::vector<ComponentInspector*> mComponentInspectors;
		Actor* mActor;
		char mActorName[256];
	};
}
