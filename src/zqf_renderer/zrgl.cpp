#ifndef ZRGL_H
#define ZRGL_H

#include "../ze_common/ze_common.h"

// Performs non-opengl related logic
#include "zr_groups.h"

// third party
#include "../../lib/glad/glad.h"
#include "../../lib/glfw3_vc2015/glfw3.h"

#include "zrgl_internal.h"

// opengl specific implementations
#include "zrgl_buffers.h"
#include "zrgl_shaders.h"
#include "zrgl_prefabs.h"
#include "zrgl_shadows.h"
// Forward
#include "zrgl_forward_draw.h"
#include "zrgl_forward_main.h"
// deferred
#include "zrgl_deferred_draw.h"
#include "zrgl_gbuffer.h"
#include "zrgl_deferred_main.h"

#include "zrgl_init.h"

#endif // ZRGL_H