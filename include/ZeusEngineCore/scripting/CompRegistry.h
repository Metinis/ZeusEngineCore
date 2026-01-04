#pragma once
#include <vector>
#include "entt.hpp"

namespace ZEN {
#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ZEN_API __declspec(dllexport)
#else
#define ZEN_API __declspec(dllimport)
#endif
#else
#define ZEN_API __attribute__((visibility("default")))
#endif

    enum class FieldType {
        Float,
        Int,
        Bool,
    };

    struct FieldInfo {
        const char *name;
        FieldType type;
        size_t offset;
    };

    struct ComponentInfo {
        const char *name;
        size_t size;
        std::vector<FieldInfo> fields;
    };

    template<typename T>
    constexpr ZEN::FieldType DeduceFieldType();

    template<>
    constexpr ZEN::FieldType DeduceFieldType<int>() { return ZEN::FieldType::Int; }

    template<>
    constexpr ZEN::FieldType DeduceFieldType<float>() { return ZEN::FieldType::Float; }

    template<>
    constexpr ZEN::FieldType DeduceFieldType<bool>() { return ZEN::FieldType::Bool; }


#define FIELD(Type, Member)                                  \
ZEN::FieldInfo {                                         \
#Member,                                             \
ZEN::DeduceFieldType<decltype(Type::Member)>(),      \
offsetof(Type, Member)                               \
}
#define REGISTER_COMPONENT(Type, ...)                        \
namespace {                                              \
struct Type##_Registrar {                            \
Type##_Registrar() {                             \
ZEN::CompRegistry::get()->registerComponent(    \
ZEN::ComponentInfo{                      \
#Type,                               \
sizeof(Type),                        \
{ __VA_ARGS__ }                      \
}                                        \
);                                           \
}                                                \
};                                                   \
static Type##_Registrar s_##Type##_registrar;        \
}

    struct RuntimeComponentStorage {
        std::unordered_map<entt::entity, std::vector<uint8_t> > data;
    };

    struct RuntimeComponent {
        std::vector<uint8_t> buffer;
        const ComponentInfo *info;
    };

    inline void *getFieldPtr(RuntimeComponent &comp, const char *fieldName) {
        for (const auto &field: comp.info->fields) {
            if (std::strcmp(field.name, fieldName) == 0) {
                return comp.buffer.data() + field.offset;
            }
        }
        return nullptr;
    }

    template<typename T>
    inline T &getField(RuntimeComponent &comp, const char *fieldName) {
        void *ptr = getFieldPtr(comp, fieldName);
        ENTT_ASSERT(ptr, "Field not found");
        return *reinterpret_cast<T *>(ptr);
    }
#define ZEN_GET(COMP, ENTITY, FIELD) \
m_Scene->getRuntimeField<decltype(COMP::FIELD)>(ENTITY, #COMP, #FIELD)

#define ZEN_SET(COMP, ENTITY, FIELD, VALUE) \
(ZEN_GET(COMP, ENTITY, FIELD) = (VALUE))

    class CompRegistry {
    public:
        CompRegistry() = default;

        ~CompRegistry() {
            clearComponents();
        }

        static CompRegistry *get() {
            return s_Instance;
        }

        std::vector<ComponentInfo>& getComponents() {
            return m_Components;
        }

        void registerComponent(ComponentInfo &&component_info) {
            m_Components.emplace_back(std::move(component_info));
        }

        bool loadSystemDLL(const std::string &path);

        void clearComponents() {
            m_Components.clear();
        }

    private:
        static CompRegistry *s_Instance;
        std::vector<ComponentInfo> m_Components{};
    };
}
