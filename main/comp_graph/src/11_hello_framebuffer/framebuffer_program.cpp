//
// Created by efarhan on 28/05/2020.
//
#include "gl/shape.h"
#include "11_hello_framebuffer/framebuffer_program.h"

namespace neko
{


HelloFramebufferProgram::HelloFramebufferProgram() :
screenFrame_(gl::RenderQuad(Vec3f::zero, Vec2f(2.0f,2.0f)))
{

}

void HelloFramebufferProgram::Init()
{
    const auto& config = BasicEngine::GetInstance()->config;
    screenFrame_.Init();
    cube_.Init();
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &fboTexture_);
    glBindTexture(GL_TEXTURE_2D, fboTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.windowSize.x, config.windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture_, 0);

    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.windowSize.x, config.windowSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        logDebug("[Error] Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    screenShader_.LoadFromFile(
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.frag"
            );
    screenInverseShader_.LoadFromFile(
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/screen_inverse.frag"
    );
    screenGrayscaleShader_.LoadFromFile(
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/screen_grayscale.frag"
    );
    screenBlurShader_.LoadFromFile(
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/screen_blur.frag"
    );
    screenEdgeDetectionShader_.LoadFromFile(
            config.dataRootPath+"shaders/11_hello_framebuffer/screen.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/screen_edge_detection.frag"
    );
    containerTexture_.SetPath(config.dataRootPath + "sprites/container.jpg");
    containerTexture_.LoadFromDisk();

    modelShader_.LoadFromFile(config.dataRootPath+"shaders/11_hello_framebuffer/model.vert",
            config.dataRootPath+"shaders/11_hello_framebuffer/model.frag");
    camera_.Init();
}

void HelloFramebufferProgram::Update(seconds dt)
{
    std::lock_guard<std::mutex> lock(updateMutex_);
    const auto& config = BasicEngine::GetInstance()->config;
    camera_.SetAspect(config.windowSize.x, config.windowSize.y);
    camera_.Update(dt);
}

void HelloFramebufferProgram::Destroy()
{
    containerTexture_.Destroy();

    glDeleteFramebuffers(1, &fbo_);
    glDeleteTextures(1, &fboTexture_);
    glDeleteRenderbuffers(1, &rbo_);

    cube_.Destroy();
    screenFrame_.Destroy();

    modelShader_.Destroy();
    screenShader_.Destroy();
    screenGrayscaleShader_.Destroy();
    screenInverseShader_.Destroy();
    screenEdgeDetectionShader_.Destroy();
    screenBlurShader_.Destroy();
}

void HelloFramebufferProgram::DrawImGui()
{

}

void HelloFramebufferProgram::Render()
{
    if(!containerTexture_.IsLoaded())
        return;
    std::lock_guard<std::mutex> lock(updateMutex_);
    //Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //Draw scene
    modelShader_.SetMat4("model", Mat4f::Identity);
    modelShader_.SetMat4("view", camera_.GenerateViewMatrix());
    modelShader_.SetMat4("projection", camera_.GenerateProjectionMatrix());

    modelShader_.SetInt("texture_diffuse1", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, containerTexture_.GetTextureId());
    cube_.Draw();

    //Bind backbuffer
    gl::Shader* currentShader = nullptr;
    switch (postProcessingType_)
    {
        case PostProcessingType::NO_POSTPROCESS:
            currentShader = &screenShader_;
            break;
        case PostProcessingType::INVERSE:
            currentShader = &screenInverseShader_;
            break;
        case PostProcessingType::GRAYSCALE:
            currentShader = &screenGrayscaleShader_;
            break;
        case PostProcessingType::BLUR:
            currentShader = &screenBlurShader_;
            break;
        case PostProcessingType::EDGE_DETECTION:
            currentShader = &screenEdgeDetectionShader_;
            break;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    currentShader->Bind();
    glDisable(GL_DEPTH_TEST);
    currentShader->SetInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture_);
    cube_.Draw();


}
void HelloFramebufferProgram::OnEvent(const SDL_Event& event)
{
    camera_.OnEvent(event);
}



}