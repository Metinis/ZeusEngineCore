#include "ZeusEngineCore/engine/PhysicsSystem.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

using namespace ZEN;

static BPLayerInterface s_BroadPhaseLayer;
static ObjectVsBroadPhaseLayerFilter s_ObjectVsBroadPhase;
static ObjectLayerPairFilter s_ObjectLayerPair;

PhysicsSystem::PhysicsSystem() {
    initJolt();
}

PhysicsSystem::~PhysicsSystem() {
    shutdownJolt();
}

void PhysicsSystem::init() {
    m_Scene = &Application::get().getEngine()->getScene();
}

JPH::BodyID PhysicsSystem::createAddBody(const JPH::BodyCreationSettings& settings) {
    JPH::EActivation activation = JPH::EActivation::Activate;

    if (settings.mMotionType == JPH::EMotionType::Static)
        activation = JPH::EActivation::DontActivate;

    return m_BodyInterface->CreateAndAddBody(settings, activation);
}

void PhysicsSystem::onUpdate(float dt) {
    if (!m_IsPlaying) {
        return;
    }
    constexpr float fixedStep = 1.0f / 60.0f;
    static float accumulator = 0.0f;

    accumulator += dt;
    while (accumulator >= fixedStep) {
        m_PhysicsSystem.Update(
            fixedStep,
            1,
            m_TempAllocator.get(),
            m_JobSystem.get()
        );
        accumulator -= fixedStep;
        for (auto entity : m_Scene->getEntities<PhysicsBodyComp, TransformComp>()) {
            auto& phys = entity.getComponent<PhysicsBodyComp>();
            auto& trans = entity.getComponent<TransformComp>();

            JPH::Vec3 pos = m_BodyInterface->GetCenterOfMassPosition(phys.bodyID);
            JPH::Quat rot = m_BodyInterface->GetRotation(phys.bodyID);

            trans.localPosition = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
            trans.localRotation = glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
        }
    }
}

void PhysicsSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayModeRun(e); });

}
void PhysicsSystem::syncECSBodyToPhysics(Entity entity, bool wake = true) {
    if (!entity.hasComponent<PhysicsBodyComp>() || !entity.hasComponent<TransformComp>())
        return;

    auto& phys = entity.getComponent<PhysicsBodyComp>();
    auto& trans = entity.getComponent<TransformComp>();

    JPH::EActivation activation = wake ? JPH::EActivation::Activate
                                      : JPH::EActivation::DontActivate;

    m_BodyInterface->SetPositionAndRotation(
        phys.bodyID,
        JPH::Vec3(trans.localPosition.x, trans.localPosition.y, trans.localPosition.z),
        JPH::Quat(trans.localRotation.x, trans.localRotation.y, trans.localRotation.z, trans.localRotation.w),
        activation
    );
}

bool PhysicsSystem::onPlayModeRun(const RunPlayModeEvent &e) {
    m_IsPlaying = e.getPlaying();
    if (m_IsPlaying) {
        for (auto entity : m_Scene->getEntities<PhysicsBodyComp, TransformComp>()) {
            syncECSBodyToPhysics(entity, true);
        }
    }
    return false;
}

void PhysicsSystem::initJolt() {
    JPH::RegisterDefaultAllocator();

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    constexpr uint32_t maxBodies = 10'000;
    constexpr uint32_t numBodyMutexes = 0;
    constexpr uint32_t maxBodyPairs = 65'536;
    constexpr uint32_t maxContactConstraints = 20'000;

    m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(50 * 1024 * 1024);

    m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        std::max(1u, std::thread::hardware_concurrency() - 1)
    );

    m_PhysicsSystem.Init(
        maxBodies,
        numBodyMutexes,
        maxBodyPairs,
        maxContactConstraints,
        s_BroadPhaseLayer,
        s_ObjectVsBroadPhase,
        s_ObjectLayerPair
    );

    m_BodyInterface = &m_PhysicsSystem.GetBodyInterface();
}

void PhysicsSystem::shutdownJolt() {
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}
