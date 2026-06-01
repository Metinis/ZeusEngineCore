#include "ZeusEngineCore/engine/Components.h"
#include "ZeusEngineCore/engine/ZeusPhysicsSystem.h"

using namespace ZEN;

glm::mat4 TransformComp::getLocalMatrix() const {
    glm::mat4 t = glm::translate(glm::mat4(1.0f), localPosition);
    glm::mat4 r = glm::toMat4(localRotation);
    glm::mat4 s = glm::scale(glm::mat4(1.0f), localScale);
    return t * r * s;
}

glm::mat4 TransformComp::getViewMatrix() const {
    glm::mat4 rot = glm::mat4_cast(glm::conjugate(localRotation)); // camera looks along -Z
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), -localPosition);
    return rot * trans;
}

glm::mat4 TransformComp::getViewMatrixWorld() const {
    return glm::inverse(worldMatrix);
}

glm::vec3 TransformComp::getFront() const {
    return glm::normalize(localRotation * glm::vec3(0, 0, -1));
}

glm::vec3 TransformComp::getRightWorld() const {
    return glm::normalize(glm::vec3(worldMatrix[0]));
}

glm::vec3 TransformComp::getUpWorld() const {
    return glm::normalize(glm::vec3(worldMatrix[1]));
}

glm::vec3 TransformComp::getFrontWorld() const {
    return glm::normalize(-glm::vec3(worldMatrix[2]));
}

glm::vec3 TransformComp::getWorldPosition() const {
    return glm::vec3(worldMatrix[3]);
}

glm::quat TransformComp::getWorldRotation() const {
    glm::vec3 worldScale;
    worldScale.x = glm::length(glm::vec3(worldMatrix[0]));
    worldScale.y = glm::length(glm::vec3(worldMatrix[1]));
    worldScale.z = glm::length(glm::vec3(worldMatrix[2]));

    glm::mat3 rotMat(
        glm::vec3(worldMatrix[0]) / worldScale.x,
        glm::vec3(worldMatrix[1]) / worldScale.y,
        glm::vec3(worldMatrix[2]) / worldScale.z
    );
    glm::quat worldRotation = glm::quat_cast(rotMat);
    return worldRotation;
}

void TransformComp::decomposeTransform(const glm::mat4 &transform) {
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::quat orientation;

    glm::decompose(transform, localScale, orientation, localPosition, skew, perspective);

    localRotation = orientation;
}


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

void PhysicsBodyComp::addVelocity(const glm::vec3 &delta) {
    auto v = getVelocity();
    v += delta;
    setVelocity(v);
}

void PhysicsBodyComp::setRotation(const glm::quat &rotation) {
    glm::quat normalized = glm::normalize(rotation);

    // JPH uses WXYZ
    JPH::Quat newRot(normalized.w, normalized.x, normalized.y, normalized.z);
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
