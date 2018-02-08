#include "scene/Object.h"

namespace Utopian
{
	Object::Object()
	{

	}

	Object::Object(string name)
	{
		SetName(name);
	}
	
	void Object::Initialize(uint32_t id)
	{
		SetId(id);
	}

	void Object::SetName(string name)
	{
		mName = name;
	}

	void Object::SetId(uint32_t id)
	{
		mId = id;
	}

	string Object::GetName()
	{
		return mName;
	}

	uint32_t Object::GetId()
	{
		return mId;
	}
}