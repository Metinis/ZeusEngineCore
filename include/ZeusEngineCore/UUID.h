#pragma once

namespace ZEN {
    class UUID {
    public:
        UUID();
        UUID(uint64_t handle);

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