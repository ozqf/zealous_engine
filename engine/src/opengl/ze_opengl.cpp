#include "ze_opengl_internal.h"


struct ZRGPUSpecs
{
    GLint maxTextureSize;          // GL_MAX_TEXTURE_SIZE
    GLint current_mem_kb;          // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
    GLint maxCombinedTextureUnits; // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
    GLint maxVertexAttribs;        // GL_MAX_VERTEX_ATTRIBS

    struct
    {
        GLint maxUniformBlockSize;          // GL_MAX_UNIFORM_BLOCK_SIZE
        GLint uniformBufferOffsetAlignment; // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
        //GLint maxVertexUniformBlocks; // GL_MAX_VERTEX_UNIFORM_BLOCKS
        //GLint maxGeometryUniformBlocks; // GL_MAX_VERTEX_UNIFORM_BLOCKS
        //GLint maxFragmentUniformBLocks; // GL_MAX_FRAGMENT_UNIFORM_BLOCKS
    } uniformBlocks;
};

internal ZRGPUSpecs g_gpuLimits;

static void ZRGL_PrintGPUInfo()
{
    const u8 *vendor = glGetString(GL_VENDOR);
    const u8 *renderer = glGetString(GL_RENDERER);
    i32 maxTexBufSize = -1;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexBufSize);
    printf("Vendor: %s, Renderer: %s\n", vendor, renderer);
    printf("Current memory: %dMB\n", g_gpuLimits.current_mem_kb / 1024);
    printf("Max texture size: %d\n", g_gpuLimits.maxTextureSize);
    printf("Max combined texture units: %d\n", g_gpuLimits.maxCombinedTextureUnits);
    printf("Maximum vertex attribs: %d\n", g_gpuLimits.maxVertexAttribs);
    printf("-- uniforms --\n");
    printf("Max Uniform block size: %dKB\n", g_gpuLimits.uniformBlocks.maxUniformBlockSize / 1024);
    printf("Uniform buffer offset alignment: %d\n",
           g_gpuLimits.uniformBlocks.uniformBufferOffsetAlignment);

    // printf("-- Data texture --\n");
    // printf("Batch data pixel count: %d\n", ZR_BATCH_DATA_STRIDE);
    // printf("Max tex buf size: %d\n", maxTexBufSize);

    i32 drawBuffer;
    glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
    printf("Default draw buffer value: %d (0x%X)\n", drawBuffer, drawBuffer);
    CHECK_GL_ERR
}

static ZRGPUSpecs ZRGL_QueryGPUSpecs()
{
    ZRGPUSpecs specs = {};
    // this is an invalid enum on old intel integrated graphics.
    // not a biggy as we'll never intend to run properly on that mind.
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &specs.current_mem_kb);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &specs.maxTextureSize);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &specs.maxCombinedTextureUnits);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &specs.maxVertexAttribs);
    CHECK_GL_ERR

    // uniform stuff
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &specs.uniformBlocks.maxUniformBlockSize);
    CHECK_GL_ERR
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &specs.uniformBlocks.uniformBufferOffsetAlignment);
    CHECK_GL_ERR

    return specs;
}

ze_external zErrorCode ZR_Init()
{
    g_gpuLimits = ZRGL_QueryGPUSpecs();
    ZRGL_PrintGPUInfo();

    // v-sync - off
    // want to cap frame rate manually... v-sync gives me jip.
    const i32 swapInterval = 1;
    printf("Swap interval - %d\n", swapInterval);
    glfwSwapInterval(swapInterval);

    // Depth buffer
    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERR
    glDepthFunc(GL_LESS);
    CHECK_GL_ERR
    glEnable(GL_CULL_FACE);
    CHECK_GL_ERR

    ZRGL_Debug_Init();

    // Init shaders
    /*
    ZRShader shader;
    zErrorCode err = ZRGL_CreateProgram(
        fallback_vert_text, fallback_frag_text, "fallback", 0, NO, &shader);
    */

    return ZE_ERROR_NONE;
}

ze_external void ZR_ClearFrame(ColourF32 colour)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    glClearColor(colour.r, colour.g, colour.b, 1);
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR
}

ze_external void ZR_ExecuteCommands(ZEBuffer* commandBuffer)
{
    TRANSFORM_CREATE(camera);
    M4x4_CREATE(projection)
    BUF_BLOCK_BEGIN_READ(commandBuffer, header)
        switch (header->type)
        {
            case ZR_DRAW_CMD_SET_CAMERA:
            camera = ((ZRDrawCmdSetCamera*)header)->camera;
            projection = ((ZRDrawCmdSetCamera *)header)->projection;
            break;
            case ZR_DRAW_CMD_SPRITE_BATCH:
            ZE_CREATE_CAST_PTR(header, ZRDrawCmdSpriteBatch, batch);
            ZRDraw_SpriteBatch(batch, &camera, &projection);
            break;
        }
    BUF_BLOCK_END_READ
}

ze_external zErrorCode ZR_DrawTest()
{
    // ZRGL_Debug_DrawCubeTest();
    // OpenglTest_DrawScreenSpaceQuad();
    // ZRGL_Debug_DrawWorldCubeTest();
    ZRGL_Debug_DrawWorldSprites();
    return ZE_ERROR_NONE;
}
