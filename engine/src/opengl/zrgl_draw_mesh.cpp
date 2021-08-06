#include "ze_opengl_internal.h"

ze_external void ZRGL_DrawMesh(ZRDrawCmdMesh* meshCmd, M4x4* camera, M4x4* view)
{
    printf("ZRGL draw mesh id %d\n", meshCmd->obj.data.model.meshId);
}
