#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H


// Finding this makes it easier to hunt down specific error locations
// vs using opengl's error callback...
#define CHECK_GL_ERR \
{ GLenum checkGlErrVal; \
while ((checkGlErrVal = glGetError()) != GL_NO_ERROR) \
{ \
    char* checkGlErrType = "Unknown"; \
    switch (checkGlErrVal) \
    { \
        case 0x500: checkGlErrType = "GL_INVALID_ENUM"; break; \
        case 0x501: checkGlErrType = "GL_INVALID_VALUE"; break; \
        case 0x502: checkGlErrType = "GL_INVALID_OPERATION"; break; \
    } \
    printf("GL Error 0x%X (%s) at %s line %d\n", checkGlErrVal, checkGlErrType, __FILE__, (__LINE__)); \
} }


#endif // OPENGL_UTILS_H