#pragma once
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include "ZeusEngineCore/core/InputEvents.h"
#include "ZeusEngineCore/core/Layer.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "Entity.h"

namespace ZEN {
    class Scene;

    class ZeusPhysicsSystem : public Layer {
    public:
        ZeusPhysicsSystem();
        ~ZeusPhysicsSystem() override;
        void init();
        JPH::BodyID createAddBody(JPH::BodyCreationSettings &settings, entt::entity entity);
        void syncECSBodyToPhysics(Entity entity, bool wake);
        void onUpdate(float dt) override;
        void onEvent(Event &event) override;
        JPH::BodyInterface *getBodyInterface() const { return m_BodyInterface; }
        JPH::PhysicsSystem &getJoltPhysicsSystem() { return m_PhysicsSystem; }

    private:
        void initJolt();
        void shutdownJolt();
        JPH::Ref<JPH::Shape> buildShapeForEntity(Entity entity);
        void loadPlayMode();
        void unloadPlayMode();
        bool onPlayModeRun(const RunPlayModeEvent &e);

        bool m_IsPlaying{};
        Scene *m_Scene{};

        JPH::PhysicsSystem m_PhysicsSystem{};
        JPH::BodyInterface *m_BodyInterface{};
        std::unique_ptr<JPH::ContactListener> m_ContactListener;

        std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator{};
        std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem{};
    };
}

namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr uint32_t NUM_LAYERS = 2;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING{0};
    static constexpr JPH::BroadPhaseLayer MOVING{1};
    static constexpr uint32_t NUM_LAYERS = 2;
}

class BPLayerInterface final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterface();
    uint32_t GetNumBroadPhaseLayers() const override;
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char *GetBroadPhaseLayerName(
        JPH::BroadPhaseLayer layer
    ) const override {
        switch (layer.GetValue()) {
            case 0: return "NON_MOVING";
            case 1: return "MOVING";
            default: return "UNKNOWN";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_Map[Layers::NUM_LAYERS];
};

class ZeusObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override;
};

class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bp) const override;
};

class ZeusContactListener final : public JPH::ContactListener {
public:
    ZeusContactListener();
    JPH::ValidateResult OnContactValidate( const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg,
        const JPH::CollideShapeResult &
    ) override;

    void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings
    ) override;

    void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings
    ) override;

    void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override;
private:
    ZEN::Scene* m_Scene{};
};
