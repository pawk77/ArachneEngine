// context.cpp

#include "context.hpp"

namespace realware
{
	void cContext::CreateMemoryPool()
	{
		_memoryPool = std::make_shared<cMemoryPool>();
	}

	void cContext::RegisterSubsystem(iObject* object)
	{
		ClassType type = object->GetType();
		const auto it = _subsystems.find(type);
		if (it == _subsystems.end())
			_subsystems.insert({ type, std::make_shared<iObject>(object) });
	}
}