#pragma once

#include <string>
#include <array>
#include "types.hpp"

namespace triton
{
    class cContext;

    class cTag
    {
    public:
        static constexpr types::usize kMaxTagByteSize = 32;
        using chars = std::array<types::u8, kMaxTagByteSize>;

    public:
        explicit cTag();
        explicit cTag(const std::string& text);
        explicit cTag(const types::u8* chars, types::usize charsByteSize);
        ~cTag() = default;

        types::boolean Compare(const std::string& text);

        inline const chars& GetData() const { return _data; }
        inline types::usize GetByteSize() const { return _byteSize; }

    private:
        void FillZeros();
        void CopyChars(const types::u8* chars, types::usize charsByteSize);

    private:
        chars _data = {};
        types::usize _byteSize = 0;
    };
}