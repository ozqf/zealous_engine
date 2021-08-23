#include "ze_opengl_internal.h"

///////////////////////////////////////////////////////////
// Shader configuration
///////////////////////////////////////////////////////////

internal GLuint g_samplerDefault2d;

ze_external void ZR_SetProg1i(GLuint prog, char *uniform, GLint value)
{
    i32 loc = glGetUniformLocation(prog, uniform);
    CHECK_GL_ERR
    if (loc == -1)
    {
        printf("Failed to find loc for i uniform %s on prog %d\n", uniform, prog);
        return;
    }
    glUniform1i(loc, value);
    CHECK_GL_ERR
}

ze_external void ZR_SetProg1f(GLuint prog, char *uniform, GLfloat value)
{
    i32 loc = glGetUniformLocation(prog, uniform);
    CHECK_GL_ERR
    if (loc == -1)
    {
        printf("Failed to find loc for f uniform %s on prog %d\n", uniform, prog);
        return;
    }
    glUniform1f(loc, value);
    CHECK_GL_ERR
}
#if 1
ze_external void ZR_SetProgVec3f(GLuint prog, char *uniform, Vec3 vec)
{
    i32 loc = glGetUniformLocation(prog, uniform);
    CHECK_GL_ERR
    if (loc == -1)
    {
        printf("Failed to find loc for v3f uniform %s on prog %d\n", uniform, prog);
        return;
    }
    glUniform3f(loc, vec.x, vec.y, vec.z);
    CHECK_GL_ERR
}

ze_external void ZR_SetProgVec4f(GLuint prog, char *uniform, Vec4 vec)
{
    i32 loc = glGetUniformLocation(prog, uniform);
    CHECK_GL_ERR
    if (loc == -1)
    {
        printf("Failed to find loc for v4f uniform %s on prog %d\n", uniform, prog);
        return;
    }
    glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
    CHECK_GL_ERR
}
#endif
ze_external void ZR_SetProgM4x4(GLuint prog, char *uniform, f32 *matrix)
{
    i32 loc = glGetUniformLocation(prog, uniform);
    CHECK_GL_ERR
    if (loc == -1)
    {
        printf("Failed to find loc for m4x4 uniform %s on prog %d\n", uniform, prog);
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);
    CHECK_GL_ERR
}

ze_external void ZR_PrepareTextureUnit1D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    GLint sampler)
{
    GLint loc = glGetUniformLocation(programId, uniformName);
    if (loc == -1)
    {
        printf("Failed to find loc for tex unit uniform %s on prog %d\n",
               uniformName, programId);
        return;
    }
    glUniform1i(loc, textureUnit);
    CHECK_GL_ERR
    glActiveTexture(glTextureUnit);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_1D, texture);
    CHECK_GL_ERR
    glBindSampler(textureUnit, sampler);
    CHECK_GL_ERR
}

ze_external void ZR_PrepareTextureUnit2D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    i32 samplerType)
{
    GLint loc = glGetUniformLocation(programId, uniformName);
    glUniform1i(loc, textureUnit);
    CHECK_GL_ERR
    glActiveTexture(glTextureUnit);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, texture);
    CHECK_GL_ERR
    GLint sampler;
    // would check sampler here
    switch (samplerType)
    {
        case ZR_TEX_SAMPLER_DEFAULT:
        default:
            sampler = g_samplerDefault2d;
            break;
    }
    glBindSampler(textureUnit, sampler);
    CHECK_GL_ERR
}

ze_external void ZR_PrepareTextureUnitCubeMap(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    GLint sampler)
{
    GLint loc = glGetUniformLocation(programId, uniformName);
    glUniform1i(loc, textureUnit);
    CHECK_GL_ERR
    glActiveTexture(glTextureUnit);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    CHECK_GL_ERR
    glBindSampler(textureUnit, sampler);
    CHECK_GL_ERR
}

///////////////////////////////////////////////////////////
// Build shaders
///////////////////////////////////////////////////////////

ze_internal ErrorCode ZRGL_LinkProgram(GLuint programId)
{
    glLinkProgram(programId);
    CHECK_GL_ERR
    i32 linkStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    CHECK_GL_ERR
    if (linkStatus == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);
        CHECK_GL_ERR
        printf("!LINK FAILED for prog %d. logged %d chars\n", programId, maxLength);
        GLchar *buf = (GLchar *)Platform_Alloc(sizeof(GLchar) * maxLength + 1);
        glGetProgramInfoLog(programId, maxLength, NULL, buf);
        CHECK_GL_ERR
        printf("LOG: (%d chars):\n%s\n", maxLength, buf);
        Platform_Free((void *)buf);

        //ZE_ASSERT(0, "Shader link failed");
        return ZE_ERROR_OPERATION_FAILED;
    }
    return ZE_ERROR_NONE;
}

ze_external void ZRGL_PrintShaderCompileLog(GLuint shaderId)
{
    GLint maxLength = 0;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
    GLchar *buf = (GLchar *)Platform_Alloc(sizeof(GLchar) * maxLength + 1);
    glGetShaderInfoLog(shaderId, maxLength, NULL, buf);
    CHECK_GL_ERR
    printf("LOG: (%d chars):\n%s\n", maxLength, buf);
    Platform_Free((void *)buf);
}

ze_external ErrorCode ZRGL_CreateProgram(
    const char *vertexShader,
    const char *fragmentShader,
    char *shaderName,
    const i32 drawObjType,
    const i32 bIsBatchable,
    ZRShader *result)
{
    //printf("---------------------------------\n");
    //printf("Building shader program \"%s\"\n", shaderName);
    *result = {};
    result->name = shaderName;
    result->drawObjType = drawObjType;
    result->bBatchable = bIsBatchable;

    // Vertex shader
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShader, NULL);
    glCompileShader(vertexShaderId);

    int vertCompilationStatus;
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &vertCompilationStatus);
    //printf("Vert shader compile status: %d\n", vertCompilationStatus);
    if (vertCompilationStatus == GL_FALSE)
    {
        printf("ERROR: Vert shader for \"%s\" compile error\n", shaderName);
        ZRGL_PrintShaderCompileLog(vertexShaderId);
        return ZE_ERROR_OPERATION_FAILED;
    }

    // Fragment shader
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShader, NULL);
    glCompileShader(fragmentShaderId);

    int fragCompilationStatus;
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &fragCompilationStatus);
    //printf("frag shader compile status: %d\n", fragCompilationStatus);
    if (fragCompilationStatus == GL_FALSE)
    {
        printf("ERROR: Frag shader for \"%s\" compile error\n", shaderName);
        ZRGL_PrintShaderCompileLog(fragmentShaderId);
        return ZE_ERROR_OPERATION_FAILED;
    }

    // Create program
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // Link
    ErrorCode err = ZRGL_LinkProgram(programId);
    if (err != ZE_ERROR_NONE)
    {
        printf("FAILED TO LINK SHADER \"%s\", Id %d\n", shaderName, programId);
        printf("---------------------------------\n");
        return err;
    }

    result->handle = programId;
    //*resultProgramId = programId;
    CHECK_GL_ERR

    printf("Built shader \"%s\", Id %d\n", shaderName, programId);
    return ZE_ERROR_NONE;
}

ze_external zErrorCode ZRGL_InitShaders()
{
    glGenSamplers(1, &g_samplerDefault2d);
    return ZE_ERROR_NONE;
}
