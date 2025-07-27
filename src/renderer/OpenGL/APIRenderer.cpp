#include "APIRenderer.h"

bool ZEN::OGLAPI::APIRenderer::BeginFrame() {
    return false;
}

void ZEN::OGLAPI::APIRenderer::DrawWithCallback(const std::function<void(void *)> &uiExtraDrawCallback) {

}

void ZEN::OGLAPI::APIRenderer::SubmitAndPresent() {

}

void ZEN::OGLAPI::APIRenderer::SetDepth(bool isDepth) {

}

void ZEN::OGLAPI::APIRenderer::Clear(bool shouldClearColor, bool shouldClearDepth) {

}
