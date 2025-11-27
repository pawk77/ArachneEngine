// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "types.hpp"

namespace realware
{
	class cMemoryPool;

	class cContext
	{
		REALWARE_CLASS(cContext)

	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T>
		cFactoryObject* Create();

		void CreateMemoryPool();

		template <typename T>
		void RegisterFactory();

		void RegisterSubsystem(iObject* object);

	private:
		std::shared_ptr<cMemoryPool> _memoryPool;
		std::unordered_map<ClassType, std::shared_ptr<iFactory>> _factories;
		std::unordered_map<ClassType, std::shared_ptr<iObject>> _subsystems;
	};

	template <typename T>
	cFactoryObject* cContext::Create()
	{
		ClassType type = T::GetType();
		const auto it = _factories.find(type);
		if (it != _factories.end())
			return _factories[type]->Create();
		else
			return nullptr;
	}

	template <typename T>
	void cContext::RegisterFactory()
	{
		ClassType type = T::GetType();
		const auto it = _factories.find(type);
		if (it == _factories.end())
			_factories.insert({type, std::make_shared<cFactory<T>>(this)});
	}
}