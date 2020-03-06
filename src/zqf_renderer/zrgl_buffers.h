#ifndef ZRGL_BUFFERS_H
#define ZRGL_BUFFERS_H

#include "zrgl_internal.h"

// Assumes tex is already bound!
// GL_CLAMP_TO_BORDER, GL_REPEAT
static void ZRGL_Buf_PrepTexture2DParams(GLuint handle, GLenum clampMode)
{
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampMode); 
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampMode); 
    CHECK_GL_ERR
    glGenerateMipmap(GL_TEXTURE_2D);
    CHECK_GL_ERR
}

static void ZRGL_Buf_SetTextureBorderColour(f32 r, f32 g, f32 b, f32 a)
{
    float borderColour[] = { r, g, b, a };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
}

static ZRFrameBuffer ZRGL_CreateRenderToTextureBuffers(
    i32 width,
    i32 height
)
{
    ZRFrameBuffer buf = {};
    // Create colour texture
    glGenTextures(1, &buf.colourTex);
    CHECK_GL_ERR

    glBindTexture(GL_TEXTURE_2D, buf.colourTex);
    CHECK_GL_ERR

    glTexImage2D(
        GL_TEXTURE_2D,
        0, GL_RGB,
        width,
        height ,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        NULL);

    ZRGL_Buf_PrepTexture2DParams(buf.colourTex, GL_CLAMP_TO_BORDER);
    
    // Create depth texture
    glGenTextures(1, &buf.depthTex);
    CHECK_GL_ERR

    glBindTexture(GL_TEXTURE_2D, buf.depthTex);
    CHECK_GL_ERR

    // create fbo
    // frame buffer obj
    glGenFramebuffers(1, &buf.fbo);
    CHECK_GL_ERR
    glBindFramebuffer(GL_FRAMEBUFFER, buf.fbo);
    CHECK_GL_ERR

    // depth map
    glGenRenderbuffers(1, &buf.depthRenderBuf);
    CHECK_GL_ERR
    glBindRenderbuffer(GL_RENDERBUFFER, buf.depthRenderBuf);
    CHECK_GL_ERR
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECK_GL_ERR
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buf.depthRenderBuf);
    CHECK_GL_ERR
    // attach fbo <-> tex
    glFramebufferTexture(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buf.colourTex, 0);
    CHECK_GL_ERR

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    CHECK_GL_ERR

    // Finished
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR

    return buf;
}

static ZRFrameBuffer ZRGL_CreateShadowMapBuffers(
    i32 width,
    i32 height
)
{
    ZRFrameBuffer buf = {};
    // create depth texture
    glGenTextures(1, &buf.depthTex);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, buf.depthTex);
    CHECK_GL_ERR
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        width,
        height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL);
    CHECK_GL_ERR
    ZRGL_Buf_PrepTexture2DParams(buf.depthTex, GL_CLAMP_TO_BORDER);
    ZRGL_Buf_SetTextureBorderColour(1, 1, 1, 1);

    // frame buffer obj
    glGenFramebuffers(1, &buf.fbo);
    CHECK_GL_ERR
    glBindFramebuffer(GL_FRAMEBUFFER, buf.fbo);
    CHECK_GL_ERR

    // attach depth texture to fbo
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        buf.depthTex,
        0);
    CHECK_GL_ERR
    glDrawBuffer(GL_NONE);
    CHECK_GL_ERR
    glReadBuffer(GL_NONE);
    CHECK_GL_ERR
    // Check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Shadow map FBO is not complete\n");
    }
    else
    {
        printf("Setup shadow map. FBO: %d Tex: %d, res %d/%d\n",
            buf.fbo, buf.depthTex, ZR_SHADOW_MAP_WIDTH, ZR_SHADOW_MAP_HEIGHT);
    }

    // Finished
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    return buf;
}

static void ZRGL_CreateUniformBufferObject()
{
    //GLint uboBlock;
    //glGenBuffers(1, )
}

#endif // ZRGL_BUFFERS_H