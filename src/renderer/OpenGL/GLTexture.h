#pragma once
#include "ZeusEngineCore/ITexture.h"

class GLTexture : public ITexture{
public:
	void Init(TextureInfo& textureInfo) override;
};