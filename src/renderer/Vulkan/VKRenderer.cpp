#include "VKRenderer.h"
void VKRenderer::Init() {

}

VKRenderer::~VKRenderer() = default;


void VKRenderer::BeginFrame() {

}

void VKRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {

}


void VKRenderer::EndFrame() {

}

void VKRenderer::DrawMesh(const IMesh& mesh, Material& material) {

}