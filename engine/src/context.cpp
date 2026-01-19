// context.cpp

#include "context.hpp"
#include "memory_pool.hpp"
#include "render_context.hpp"

namespace harpy
{
	void cContext::CreateMemoryAllocator()
	{
		_allocator = new cMemoryAllocator();
		_allocator->SetBins(65536);
	}

	void cContext::RegisterSubsystem(iObject* object)
	{
		ClassType type = object->GetType();
		const auto it = _subsystems.find(type);
		if (it == _subsystems.end())
			_subsystems.insert({ type, std::make_shared<iObject>(this) });
	}
}