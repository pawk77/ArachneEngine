// object.hpp

#pragma once

#include <string>

namespace realware
{
	class cContext;
	
	class cObject
	{
	public:
		explicit cObject() = default;
		~cObject() = default;
	};

	class iObjectFactory
	{
	public:
		explicit iObjectFactory(cContext* context) : _context(context) {}
		~iObjectFactory() = default;

		virtual cObject* Create() = 0;
		const std::string& GetType() { return _type; }

	protected:
		cContext* _context = nullptr;
		std::string _type = "";
	};

	template <typename T>
	class cObjectFactory : public iObjectFactory
	{
	public:
		explicit cObjectFactory(cContext* context) : iObjectFactory(context) {}

		virtual T* Create() override final {  }
	};
}