// memory_pool.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "types.hpp"

namespace realware
{
    template <typename T>
    class cMemoryPool : public iObject
    {
    public:
        explicit cMemoryPool(cContext* context);

        virtual ~cMemoryPool();

        inline virtual cType GetType() const override final { return cType("MemoryPool"); }

        void Initialize(types::usize maxAllocationCount, types::usize alignment);

        T* Allocate();

        void Deallocate(T* object);

        inline types::usize GetByteSize() const { return _byteSize; }

        inline types::usize GetAlignment() const { return _alignment; }
    
    public:
        static constexpr types::usize ALIGNMENT_NONE = 0;

    private:
        types::usize _maxAllocationCount = 0;
        types::usize _byteSize = 0;
        types::usize _alignment = 0;
        types::usize _allocationCount = 0;
        T* _allocations = nullptr;
    };

    template <typename T>
    void cMemoryPool<T>::Initialize(types::usize maxAllocationCount, types::usize alignment)
    {
        _maxAllocationCount = maxAllocationCount;
        _byteSize = sizeof(T) * _maxAllocationCount;
        _alignment = alignment;

        if (alignment == ALIGNMENT_NONE)
            _allocations = std::malloc(_byteSize);
        else
            _allocations = _aligned_malloc(_byteSize, alignment);

        for (types::usize i = 0; i < _maxAllocationCount; i++)
            new (&_allocations[i]) new T(_context);
    }

    template <typename T>
    T* cMemoryPool<T>::Allocate()
    {
        if (_allocationCount >= _maxAllocationCount)
            return nullptr;

        for (types::usize i = 0; i < _maxAllocationCount; i++)
        {
            if (_allocations[i]._occupied == types::K_FALSE)
            {
                _allocations[i].~T();
                new (&_allocations[i]) new T(_context);
                _allocations[i]._occupied = types::K_TRUE;
                _allocations[i]._allocatorIndex = i;

                return &_allocations[i];
            }
        }

        return nullptr;
    }

    template <typename T>
    void cMemoryPool<T>::Deallocate(T* object)
    {
        for (types::usize i = 0; i < _maxAllocationCount; i++)
        {
            if (_allocations[i]._occupied == types::K_TRUE && _allocations[i]._allocatorIndex == object->_allocatorIndex)
            {
                _allocations[i].~T();
                _allocations[i]._allocatorIndex = 0;
                _allocations[i]._occupied = types::K_FALSE;

                return;
            }
        }
    }
}