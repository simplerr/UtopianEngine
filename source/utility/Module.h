#pragma once

template<class T>
class Module
{
public:

	static T& Instance()
	{
		return *_Instance();
	}

	static T* InstancePtr()
	{
		return _Instance();
	}
	
	template<class ...Args>
	static void Start(Args &&...args)
	{
		_Instance() = new T(std::forward<Args>(args)...);
	}
protected:
	Module()
	{

	}

	virtual ~Module()
	{
		_Instance() = nullptr;
	}

	Module(const Module&) { }
	Module& operator=(const Module&) { return *this; }

	/** Returns a singleton instance of this module. */
	static T*& _Instance()
	{
		static T* inst = nullptr;
		return inst;
	}
private:
};