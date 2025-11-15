// id_vec.hpp

#pragma once

#include <vector>
#include "log.hpp"
#include "types.hpp"

namespace realware
{
	namespace app
	{
		class cApplication;
	}

	namespace utils
	{
		struct sIdVecObject
		{
			std::string ID = "";
			app::cApplication* App = nullptr;
			types::boolean IsDeleted = types::K_FALSE;
		};

		template <typename T>
		class cIdVec
		{
		public:
			explicit cIdVec(const app::cApplication* const app, const types::usize maxObjectCount);
			~cIdVec() = default;

			template<typename... Args>
			T* Add(const std::string& id, Args&&... args);
			T* Add(const T& object);
			T* Find(const std::string& id);
			void Delete(const std::string& id);

			inline std::vector<T>& GetObjects() { return _objects; }
			inline types::usize GetObjectCount() { return _objectCount; }
			inline types::usize GetMaxObjectCount() { return _maxObjectCount; }

		private:
			app::cApplication* _app = nullptr;
			types::usize _maxObjectCount = 0;
			types::usize _objectCount = 0;
			std::vector<T> _objects = {};
		};

		template<typename T>
		cIdVec<T>::cIdVec(const app::cApplication* const app, const types::usize maxObjectCount) : _app((app::cApplication*)app), _maxObjectCount(maxObjectCount)
		{
			_objects.resize(maxObjectCount);
		}

		template<typename T>
		template<typename... Args>
		T* cIdVec<T>::Add(const std::string& id, Args&&... args)
		{
			if (_objectCount >= _maxObjectCount)
			{
				log::Print("Error: object count limit '" + std::to_string(_maxObjectCount) + "' exceeded!");

				return nullptr;
			}

			types::usize index = _objectCount;
			_objects[index] = T(std::forward<Args>(args)...);
			_objects[index].ID = id;
			_objects[index].App = _app;
			_objectCount += 1;

			return &_objects[index];
		}

		template<typename T>
		T* cIdVec<T>::Add(const T& object)
		{
			if (_objectCount >= _maxObjectCount)
			{
				log::Print("Error: object count limit '" + std::to_string(_maxObjectCount) + "' exceeded!");

				return nullptr;
			}

			types::usize index = _objectCount;
			_objects[index] = object;
			_objectCount += 1;

			return &_objects[index];
		}

		template<typename T>
		T* cIdVec<T>::Find(const std::string& id)
		{
			for (types::usize i = 0; i < _objectCount; i++)
			{
				if (_objects[i].ID == id)
					return &_objects[i];
			}

			return nullptr;
		}

		template<typename T>
		void cIdVec<T>::Delete(const std::string& id)
		{
			for (types::usize i = 0; i < _objectCount; i++)
			{
				if (_objects[i].ID == id)
				{
					const T& last = _objects[_objectCount - 1];
					_objects[i] = last;
					_objectCount -= 1;
				}
			}
		}
	}
}