#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
	using entity = types::u32;

	struct sComponent {};

	class cSystem : public iObject
	{
	public:
		explicit cSystem(cContext* context);
		virtual ~cSystem() override final = default;

		void Init() = 0;
		void Update() = 0;
		void Shutdown() = 0;
	};

	cSystem::cSystem(cContext* context) : iObject(context) {}
}