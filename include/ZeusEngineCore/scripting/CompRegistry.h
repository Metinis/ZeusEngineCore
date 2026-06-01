#pragma once
#include <vector>
#include "RuntimeComp.h"

namespace ZEN {
    struct EngineContext;
    class CompRegistry {
    public:
        CompRegistry() = default;

        ~CompRegistry() {
            clearComponents();
        }

        void init(EngineContext* ctx) {};

        std::vector<ComponentInfo>& getComponents() {
            return m_Components;
        }

        void registerComponent(ComponentInfo &&component_info) {
            m_Components.emplace_back(std::move(component_info));
        }

        void clearComponents() {
            m_Components.clear();
        }

    private:
        std::vector<ComponentInfo> m_Components{};
    };

}
