#include "ZeusEngineCore/engine/UUID.h"
#include <random>

using namespace ZEN;

static std::random_device s_Device{};
static std::mt19937_64 s_RandEngine{s_Device()};
static std::uniform_int_distribution<uint64_t> s_Dist{};

UUID::UUID() : m_Handle(s_Dist(s_RandEngine)) {

}

UUID::UUID(uint64_t handle) : m_Handle(handle) {

}
