#include "ZeusEngineCore/engine/ZeusPhysicsSystem.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"

using namespace ZEN;

static BPLayerInterface s_BroadPhaseLayer;
static ObjectVsBroadPhaseLayerFilter s_ObjectVsBroadPhase;
static ZeusObjectLayerPairFilter s_ObjectLayerPair;

ZeusPhysicsSystem::ZeusPhysicsSystem() {

}

ZeusPhysicsSystem::~ZeusPhysicsSystem() {
    shutdownJolt();
}

void ZeusPhysicsSystem::init() {
    m_Scene = &Application::get().getEngine()->getScene();
    initJolt();
}

JPH::BodyID ZeusPhysicsSystem::createAddBody(JPH::BodyCreationSettings& settings, entt::entity entity) {
    JPH::EActivation activation = JPH::EActivation::Activate;

    settings.mUserData = static_cast<uint64_t>(entity);

    if (settings.mMotionType == JPH::EMotionType::Static)
        activation = JPH::EActivation::DontActivate;

    return m_BodyInterface->CreateAndAddBody(settings, activation);
}

void ZeusPhysicsSystem::onUpdate(float dt) {
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

void ZeusPhysicsSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayModeRun(e); });

}
void ZeusPhysicsSystem::syncECSBodyToPhysics(Entity entity, bool wake = true) {
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
JPH::Ref<JPH::Shape> ZeusPhysicsSystem::buildShapeForEntity(Entity entity) {
    JPH::StaticCompoundShapeSettings compound;
    bool hasShape = false;

    if (entity.hasComponent<BoxColliderComp>()) {
        auto& box = entity.getComponent<BoxColliderComp>();
        JPH::Vec3 halfExtents(box.halfExtents.x, box.halfExtents.y, box.halfExtents.z);
        JPH::Ref<JPH::Shape> shape = new JPH::BoxShape(halfExtents);

        compound.AddShape(
            JPH::Vec3(box.offset.x, box.offset.y, box.offset.z),
            JPH::Quat::sIdentity(),
            shape
        );

        hasShape = true;
    }

    if (entity.hasComponent<SphereColliderComp>()) {
        auto& sphere = entity.getComponent<SphereColliderComp>();

        JPH::Ref<JPH::Shape> shape = new JPH::SphereShape(sphere.radius);

        compound.AddShape(
            JPH::Vec3(sphere.offset.x, sphere.offset.y, sphere.offset.z),
            JPH::Quat::sIdentity(),
            shape
        );

        hasShape = true;
    }

    if (!hasShape)
        return nullptr;

    return compound.Create().Get();
}
void ZeusPhysicsSystem::loadPlayMode() {
    for (auto entity : m_Scene->getEntities<RigidBodyComp, TransformComp>()) {
        auto& transform = entity.getComponent<TransformComp>();
        auto& rigidbody = entity.getComponent<RigidBodyComp>();

        //Build shape from colliders
        JPH::Ref<JPH::Shape> shape = buildShapeForEntity(entity);

        if (!shape)
            continue; // no collider → no physics body

        //Convert transform
        JPH::Vec3 position(
            transform.localPosition.x,
            transform.localPosition.y,
            transform.localPosition.z
        );

        JPH::Quat rotation(
            transform.localRotation.x,
            transform.localRotation.y,
            transform.localRotation.z,
            transform.localRotation.w
        );

        //Choose layer
        uint8_t layer =
            rigidbody.motionType == JPH::EMotionType::Static
                ? Layers::NON_MOVING
                : Layers::MOVING;

        //Create body settings
        JPH::BodyCreationSettings settings(
            shape,
            position,
            rotation,
            rigidbody.motionType,
            layer
        );

        //Mass + damping
        if (rigidbody.motionType == JPH::EMotionType::Dynamic) {
            settings.mOverrideMassProperties =
                JPH::EOverrideMassProperties::CalculateInertia;

            settings.mMassPropertiesOverride.mMass = rigidbody.mass;
        }

        settings.mLinearDamping  = rigidbody.linearDamping;
        settings.mAngularDamping = rigidbody.angularDamping;
        settings.mAllowSleeping = rigidbody.allowSleep;


        //Axis locking
        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs::All;

        if (rigidbody.lockPosX) dofs &= ~JPH::EAllowedDOFs::TranslationX;
        if (rigidbody.lockPosY) dofs &= ~JPH::EAllowedDOFs::TranslationY;
        if (rigidbody.lockPosZ) dofs &= ~JPH::EAllowedDOFs::TranslationZ;

        if (rigidbody.lockRotX) dofs &= ~JPH::EAllowedDOFs::RotationX;
        if (rigidbody.lockRotY) dofs &= ~JPH::EAllowedDOFs::RotationY;
        if (rigidbody.lockRotZ) dofs &= ~JPH::EAllowedDOFs::RotationZ;

        settings.mAllowedDOFs = dofs;

        //Create body
        JPH::BodyID bodyID = createAddBody(
            settings,
            (entt::entity)entity
        );

        PhysicsBodyComp comp(this);
        comp.bodyID = bodyID;
        entity.addComponent<PhysicsBodyComp>(comp);
    }
}

void ZeusPhysicsSystem::unloadPlayMode() {
    //unload all physics handles
    for (auto entity : m_Scene->getEntities<PhysicsBodyComp>()) {
        auto& phys = entity.getComponent<PhysicsBodyComp>();

        if (!phys.bodyID.IsInvalid()) {
            m_BodyInterface->RemoveBody(phys.bodyID);
            m_BodyInterface->DestroyBody(phys.bodyID);
        }
        entity.removeComponent<PhysicsBodyComp>();
    }
}
bool ZeusPhysicsSystem::onPlayModeRun(const RunPlayModeEvent &e) {
    m_IsPlaying = e.getPlaying();
    if (m_IsPlaying) {
        for (auto entity : m_Scene->getEntities<PhysicsBodyComp, TransformComp>()) {
            syncECSBodyToPhysics(entity, true);
        }
        loadPlayMode();
    }
    else {
        unloadPlayMode();
    }
    return false;
}

void ZeusPhysicsSystem::initJolt() {
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
    m_ContactListener = std::make_unique<ZeusContactListener>();
    m_PhysicsSystem.SetContactListener(m_ContactListener.get());
}

void ZeusPhysicsSystem::shutdownJolt() {
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}
BPLayerInterface::BPLayerInterface() {
    m_Map[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    m_Map[Layers::MOVING] = BroadPhaseLayers::MOVING;
}

uint32_t BPLayerInterface::GetNumBroadPhaseLayers() const {
    return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer BPLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer layer) const {
    return m_Map[layer];
}
bool ZeusObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const {
    if (a == Layers::NON_MOVING)
        return b == Layers::MOVING;
    return true;
}
bool ObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bp) const {
    if (layer == Layers::NON_MOVING)
        return bp == BroadPhaseLayers::MOVING;
    return true;
}

ZeusContactListener::ZeusContactListener() : m_Scene(&Application::get().getEngine()->getScene()){}

JPH::ValidateResult ZeusContactListener::OnContactValidate(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    JPH::RVec3Arg,
    const JPH::CollideShapeResult &
) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void ZeusContactListener::OnContactAdded(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    const JPH::ContactManifold &inManifold,
    JPH::ContactSettings &ioSettings
) {
    auto entityA = Entity((entt::entity)inBody1.GetUserData());
    auto entityB = Entity((entt::entity)inBody2.GetUserData());
    m_Scene->onCollisionEnter(entityA, entityB);
}

void ZeusContactListener::OnContactPersisted(
    const JPH::Body &inBody1,
    const JPH::Body &inBody2,
    const JPH::ContactManifold &inManifold,
    JPH::ContactSettings &ioSettings
) {
    auto entityA = Entity((entt::entity)inBody1.GetUserData());
    auto entityB = Entity((entt::entity)inBody2.GetUserData());
    m_Scene->onCollisionStay(entityA, entityB);
}


void ZeusContactListener::OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) {
    Entity entityA((entt::entity)inSubShapePair.GetBody1ID().GetIndex());
    Entity entityB((entt::entity)inSubShapePair.GetBody2ID().GetIndex());
    m_Scene->onCollisionExit(entityA, entityB);
}


