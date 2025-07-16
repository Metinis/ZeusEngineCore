#pragma once
#include <cstddef>

namespace ZEN{
    inline constexpr std::size_t buffering_v{ 2 };
    template<typename Type>
    using Buffered = std::array<Type, buffering_v>;
}
