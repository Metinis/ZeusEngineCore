#pragma once
#include "ZeusEngineCore/core/API.h"

namespace ZEN {
    class ZEN_API UUID {
    public:
        UUID();
        constexpr UUID(uint64_t handle) noexcept
            : m_Handle(handle) {}
        //UUID(uint64_t handle);

        operator uint64_t() const {
            return m_Handle;
        }
    private:
        uint64_t m_Handle{};
    };
}

namespace std {
    template<>
    struct hash<ZEN::UUID>{
        size_t operator()(const ZEN::UUID& uuid) const {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}