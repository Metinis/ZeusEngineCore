#include "APIRenderer.h"

bool ZEN::OGLAPI::APIRenderer::BeginFrame() {
    return false;
}

void ZEN::OGLAPI::APIRenderer::DrawWithCallback(const std::function<void(void *)> &uiExtraDrawCallback) {

}

void ZEN::OGLAPI::APIRenderer::SubmitAndPresent() {

}
