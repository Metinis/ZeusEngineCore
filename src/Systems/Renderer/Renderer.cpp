#include "ZeusEngineCore/Renderer.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "ZeusEngineCore/Application.h"
#include "ZeusEngineCore/InputEvents.h"

using namespace ZEN;

float xCorner = 0, yCorner = 0;

Renderer::Renderer() : m_Window(Application::get().getWindow()->getNativeWindow()){
    m_Context = IContext::create();
    m_ResourceManager = IResourceManager::create();
    m_ViewUBO.uboID = m_ResourceManager->createUBO(0);
    m_InstanceUBO.uboID = m_ResourceManager->createUBO(1);
    m_GlobalUBO.uboID = m_ResourceManager->createUBO(2);
    m_MaterialUBO.uboID = m_ResourceManager->createUBO(3);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
    m_Width = fbWidth;
    m_Height = fbHeight;
    m_MainFBO.fboID = m_ResourceManager->createFBO();
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ColorTex.textureID = m_ResourceManager->createColorTex(fbWidth, fbHeight);
    m_DepthRBO.rboID = m_ResourceManager->createDepthStencilBuffer(fbWidth, fbHeight);

    m_CaptureFBO.fboID = m_ResourceManager->createFBO();
    m_ResourceManager->bindFBO(m_CaptureFBO.fboID);
    m_CaptureRBO.rboID = m_ResourceManager->createDepthBuffer(512, 512);
}

void Renderer::beginFrame() {
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->clear(true, true);

    if(m_Resized) {
        m_ResourceManager->bindFBO(m_MainFBO.fboID);
        m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);
        m_ResourceManager->deleteTexture(m_ColorTex.textureID);
        m_ResourceManager->deleteDepthBuffer(m_DepthRBO.rboID);
        m_ColorTex.textureID = m_ResourceManager->createColorTex(m_Width, m_Height);
        m_DepthRBO.rboID = m_ResourceManager->createDepthBuffer(m_Width, m_Height);
        m_Resized = false;
    }
}

void Renderer::bindDefaultFBO() {
    m_ResourceManager->bindFBO(0);
    m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);

}
const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const glm::mat4 captureViews[] =
{
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
 };
void Renderer::renderToCubeMapHDR(uint32_t cubemapTexID, uint32_t eqToCubeMapShader, uint32_t hdrTexID, const GPUMesh& drawable) {

    // convert HDR equirectangular environment map to cubemap equivalent
    m_ResourceManager->bindShader(eqToCubeMapShader);
    m_ResourceManager->bindTexture(hdrTexID, 0);
    m_Context->setViewport(1024, 1024);
    m_ResourceManager->bindFBO(m_CaptureFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_CaptureRBO.rboID);
    m_ResourceManager->updateDepthBufferDimensions(1024, 1024);

    m_ResourceManager->bindUBO(m_ViewUBO.uboID);
    m_Context->disableCullFace();
    for (unsigned int i = 0; i < 6; ++i) {
        glm::mat4 vp = captureProjection * captureViews[i];

        writeToUBO(m_ViewUBO.uboID, vp);

        m_ResourceManager->setFBOCubeMapTexture(i, cubemapTexID, 0);

        m_Context->clear(true, true);

        m_Context->drawMesh(*m_ResourceManager, drawable);


    }
    m_ResourceManager->genMipMapCubeMap(cubemapTexID);
    m_Context->enableCullFace();
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);
}

void Renderer::renderToIrradianceMap(uint32_t cubemapTexID, uint32_t irradianceTexID, uint32_t irradianceShader,
    const GPUMesh &drawable) {
    // convert HDR equirectangular environment map to cubemap equivalent
    m_ResourceManager->bindShader(irradianceShader);
    m_ResourceManager->bindCubeMapTexture(cubemapTexID, 0);
    m_Context->setViewport(32, 32);
    m_ResourceManager->bindFBO(m_CaptureFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_CaptureRBO.rboID);
    m_ResourceManager->updateDepthBufferDimensions(32, 32);

    m_ResourceManager->bindUBO(m_ViewUBO.uboID);
    m_Context->disableCullFace();
    for (unsigned int i = 0; i < 6; ++i) {
        glm::mat4 vp = captureProjection * captureViews[i];

        writeToUBO(m_ViewUBO.uboID, vp);

        m_ResourceManager->setFBOCubeMapTexture(i, irradianceTexID, 0);

        m_Context->clear(true, true);

        m_Context->drawMesh(*m_ResourceManager, drawable);


    }
    m_Context->enableCullFace();
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);
}

void Renderer::renderToPrefilterMap(uint32_t cubemapTexID, uint32_t prefilterTexID, uint32_t prefilterShader,
    const GPUMesh &drawable) {
    // convert HDR equirectangular environment map to cubemap equivalent
    m_ResourceManager->bindShader(prefilterShader);
    m_ResourceManager->bindCubeMapTexture(cubemapTexID, 0);
    m_ResourceManager->bindFBO(m_CaptureFBO.fboID);


    m_ResourceManager->bindUBO(m_ViewUBO.uboID);
    m_Context->disableCullFace();
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth  = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        m_ResourceManager->bindDepthBuffer(m_CaptureRBO.rboID);
        m_ResourceManager->updateDepthBufferDimensions(mipWidth, mipHeight);
        m_Context->setViewport(mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        m_ResourceManager->pushFloat(prefilterShader, "u_Roughness", roughness);

        for (unsigned int i = 0; i < 6; ++i) {
            glm::mat4 vp = captureProjection * captureViews[i];

            writeToUBO(m_ViewUBO.uboID, vp);

            m_ResourceManager->setFBOCubeMapTexture(i, prefilterTexID, mip);

            m_Context->clear(true, true);

            m_Context->drawMesh(*m_ResourceManager, drawable);


        }
    }
    m_Context->enableCullFace();
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);
}

void Renderer::renderToBRDFLUT(uint32_t brdfTexID, uint32_t brdfShader, const GPUMesh& drawable) {
    m_Context->disableCullFace();

    m_ResourceManager->bindFBO(m_CaptureFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_CaptureRBO.rboID);
    m_ResourceManager->updateDepthBufferDimensions(1024, 1024);
    m_ResourceManager->setFBOTexture2D(0, brdfTexID, 0);
    m_Context->setViewport(1024, 1024);

    m_ResourceManager->bindShader(brdfShader);
    m_Context->clear(true, true);

    m_Context->drawMesh(*m_ResourceManager, drawable); //quad

    m_Context->enableCullFace();

    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->setViewport(xCorner, yCorner, m_Width, m_Height);

}

void Renderer::renderToScreenQuad(uint32_t quadShader, const GPUMesh &drawable) {
    m_Context->disableCullFace();
    bindDefaultFBO();
    m_ResourceManager->bindShader(quadShader);
    m_ResourceManager->bindTexture(m_ColorTex.textureID, 0);
    m_ResourceManager->bindDepthBuffer(m_DepthRBO.rboID);
    m_Context->clear(true, true);
    m_Context->drawMesh(*m_ResourceManager, drawable); //quad
    m_Context->enableCullFace();
}

void Renderer::endFrame() {
    m_Context->swapBuffers();
}

void Renderer::setSize(float width, float height) {
    std::cout << "Width: "<<width<<" Height: "<<height<<"\n";
    float requiredHeightOfViewport = width * (1.0f / m_AspectRatio);
    if (requiredHeightOfViewport > height)
    {
        float requiredWidthOfViewport = height * m_AspectRatio;
        if (requiredWidthOfViewport > width)
        {
            std::cout << "Error: Couldn't find dimensions that preserve the aspect ratio\n";
        }
        else
        {
            m_Width = static_cast<int>(requiredWidthOfViewport);
            m_Height = height;
            float widthOfTheTwoVerticalBars = width - m_Width;
            xCorner = (int)glm::round(widthOfTheTwoVerticalBars * 0.5f);
            yCorner = 0;
        }
    }
    else
    {
        m_Width = width;
        m_Height = static_cast<int>(requiredHeightOfViewport);
        float heightOfTheTwoHorizontalBars = height - m_Height;
        xCorner = 0;
        yCorner = (int)glm::round(heightOfTheTwoHorizontalBars * 0.5f);
    }
    m_Resized = true;
}


void* Renderer::getColorTextureHandle() {
    return (void*)m_ResourceManager->getTexture(m_ColorTex.textureID);
}

