// log.hpp

#pragma once

#include <iostream>

namespace harpy
{
	template<typename T>
	void Print(const T& message)
	{
		std::cout << message << std::endl;
	}

	template<typename T>
	void Print(const T& message, std::ostream& stream)
	{
		stream << message << std::endl;
	}
}