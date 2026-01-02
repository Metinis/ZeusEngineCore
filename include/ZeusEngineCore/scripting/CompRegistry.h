#pragma once
#include <vector>

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
        const char* name;
        FieldType type;
        size_t offset;
    };

    struct ComponentInfo {
        const char* name;
        size_t size;
        std::vector<FieldInfo> fields;
    };

    template<typename T>
constexpr ZEN::FieldType DeduceFieldType();

    template<> constexpr ZEN::FieldType DeduceFieldType<int>()   { return ZEN::FieldType::Int; }
    template<> constexpr ZEN::FieldType DeduceFieldType<float>() { return ZEN::FieldType::Float; }
    template<> constexpr ZEN::FieldType DeduceFieldType<bool>()  { return ZEN::FieldType::Bool; }


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




    class CompRegistry {
    public:
        CompRegistry() = default;
        ~CompRegistry() {
            clearComponents();
        }
        static CompRegistry* get() {
            return s_Instance;
        }
        std::vector<ComponentInfo> getComponents() {
            return m_Components;
        }
        void registerComponent(ComponentInfo&& component_info) {
            m_Components.emplace_back(std::move(component_info));
        }
        bool loadSystemDLL(const std::string& path);
        void clearComponents() {
            m_Components.clear();
        }
    private:
        static CompRegistry* s_Instance;
        std::vector<ComponentInfo> m_Components{};
    };
}
