#pragma once
#include <string>
using namespace std;

namespace Scene
{
	class Object
	{
	public:
		Object();
		Object(string name);

		void Initialize(uint32_t id);

		void SetName(string name);
		void SetId(uint32_t id);

		string GetName();
		uint32_t GetId();

	private:
		string mName;
		uint32_t mId;
	};
}
