#include "ZeusEngineCore/engine/Components.h"
#include "ZeusEngineCore/engine/ZeusPhysicsSystem.h"

using namespace ZEN;

void PhysicsBodyComp::addImpulse(glm::vec3 force) {
    m_PhysicsSystem->getBodyInterface()->AddImpulse(bodyID, JPH::Vec3(force.x, force.y, force.z));
}
void PhysicsBodyComp::setVelocity(glm::vec3 velocity) {
    JPH::Vec3 vel = m_PhysicsSystem->getBodyInterface()->GetLinearVelocity(bodyID);

    vel.SetX(velocity.x);
    vel.SetY(velocity.y);
    vel.SetZ(velocity.z);

    m_PhysicsSystem->getBodyInterface()->SetLinearVelocity(bodyID, vel);
}
glm::vec3 PhysicsBodyComp::getVelocity() {
    JPH::Vec3 vel = m_PhysicsSystem->getBodyInterface()->GetLinearVelocity(bodyID);
    return {vel.GetX(), vel.GetY(), vel.GetZ()};
}
void PhysicsBodyComp::addVelocity(const glm::vec3& delta) {
    auto v = getVelocity();
    v += delta;
    setVelocity(v);
}
void PhysicsBodyComp::setRotation(const glm::quat& rotation) {
    //JPH uses WXYZ
    JPH::Quat newRot(rotation.w, rotation.x, rotation.y, rotation.z);
    m_PhysicsSystem->getBodyInterface()->SetRotation(bodyID, newRot, JPH::EActivation::Activate);
}
glm::quat PhysicsBodyComp::getRotation() const {
    //JPH uses WXYZ
    auto rot = m_PhysicsSystem->getBodyInterface()->GetRotation(bodyID);
    return {rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()};
}
void PhysicsBodyComp::rotate(glm::vec3 axis, float angle) {
    //GLM uses XYZW
    auto rot = glm::rotate(getRotation(), angle, axis);
    setRotation({rot.x, rot.y, rot.z, rot.w});
}