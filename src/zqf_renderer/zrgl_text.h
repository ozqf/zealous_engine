
#include "zrgl_internal.h"

////////////////////////////////////////////////////////////
// Draw text
////////////////////////////////////////////////////////////
static void ZR_ExecuteTextDraw(
    const ZRDrawCmd_Text* cmd, M4x4* prj, char* str, ZRPerformanceStats* stats)
{
	Point textSize = {};
	ZE_StrMeasure2D(str, &textSize.x, &textSize.y);
    // Size of quad on screen, to offset verts from char position
    f32 charHalfWidth = (cmd->charSize * (cmd->aspectRatio - 1.f)) / 2.f;
    f32 charHalfHeight = cmd->charSize / 2.f;

    Vec3 origin = cmd->origin;
    switch (cmd->alignmentMode)
    {
		case ZR_TEXT_ALIGNMENT_TOP_LEFT:
		{
            origin.x += charHalfWidth;
            origin.y -= charHalfHeight;
        } break;
		case ZR_TEXT_ALIGNMENT_CENTRE:
        {
            origin.x -= (charHalfWidth * textSize.x);
            //origin.y -= charHalfHeight;
        } break;
		case ZR_TEXT_ALIGNMENT_TOP_RIGHT:
        {
            origin.x += charHalfWidth;
            origin.y -= charHalfHeight;
        } break;
        default:
        {
            printf("Unsupported text alignment mode %d\n", cmd->alignmentMode);
            return;
        } break;
    }
    
    #if 1
    // Get character quad prefab and use to stamp out characters.
    ZRDBMesh* mesh = AssetDb()->GetMeshByName(AssetDb(), ZRDB_MESH_NAME_DYNAMIC_QUAD);
    if (mesh == NULL) { return; }
    ZRDBTexture* tex = AssetDb()->GetTextureByName(AssetDb(), ZRDB_DEFAULT_CHARSET_NAME);
    if (tex == NULL) { return; }
    M4x4_CREATE(modelView)
    // Setup shader
    
    ZRGL_SetupProg_Text(
        prj,
        &modelView,
        g_programs[ZR_SHADER_TYPE_TEXT].handle,
        tex->apiHandle);

    glBindVertexArray(mesh->handles.vao);
    CHECK_GL_ERR
    glBindBuffer(GL_ARRAY_BUFFER, mesh->handles.vbo);
    CHECK_GL_ERR
    // glBindTexture(GL_TEXTURE_2D, tex->apiHandle);
    // CHECK_GL_ERR

    // Setup buffers
    // Verts and uvs must be rewritten. Normals can remain the same.
    // NOTE: This is assuming a non-interleaved geometry layout!
    u8 quadBuffer[ZRGL_BYTES_FOR_QUAD_VERTS + ZRGL_BYTES_FOR_QUAD_UVS];
    Vec3* verts = (Vec3*)quadBuffer;
    Vec2* uvs = (Vec2*)(quadBuffer + ZRGL_BYTES_FOR_QUAD_VERTS);

    // Divide up character set tiles
    const f32 stride = 1.f / (f32)ZRGL_ASCI_CHARSET_CHARS_WIDE;

    char* txt = str;//(char*)((u8*)cmd + cmd->offsetToString);

    Vec3 pos = origin;

    while (*txt != NULL)
    {
        char c = *txt;
        txt++;

        if (c == '\n')
        {
            pos.x = origin.x;
            pos.y -= charHalfHeight * 2.f;
            continue;
        }
        else if (c == '\t')
        {
            pos.x += (charHalfWidth * 2.f) * 4.f;
            continue;
        }

        // convert asci to sheet position
        i32 sheetX = c % ZRGL_ASCI_CHARSET_CHARS_WIDE;
        i32 sheetY = c / ZRGL_ASCI_CHARSET_CHARS_WIDE;
        // Sheet is top -> down but opengl is bottom -> up so flip the Y coord
    	sheetY = (16 - 1) - sheetY;

        // position on sheet
        f32 uvLeft = stride * (f32)sheetX;
        f32 uvRight = uvLeft + stride;

        f32 uvBottom = stride * (f32)sheetY;
        f32 uvTop = uvBottom + stride;

        uvs[0] = { uvLeft, uvBottom };
	    uvs[1] = { uvRight, uvBottom };
	    uvs[2] = { uvRight, uvTop };
	    uvs[3] = { uvLeft, uvBottom };
	    uvs[4] = { uvRight, uvTop };
	    uvs[5] = { uvLeft, uvTop };

        // triangle 1
        verts[0] = { pos.x - charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[1] = { pos.x + charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[2] = { pos.x + charHalfWidth, pos.y + charHalfHeight, pos.z };
        // triangle 2
        verts[3] = { pos.x - charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[4] = { pos.x + charHalfWidth, pos.y + charHalfHeight, pos.z };
        verts[5] = { pos.x - charHalfWidth, pos.y + charHalfHeight, pos.z };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadBuffer), quadBuffer);
        CHECK_GL_ERR
        glDrawArrays(GL_TRIANGLES, 0, 6);
        CHECK_GL_ERR

        stats->drawCalls++;
        stats->trisSingle += 2;

        pos.x += charHalfWidth * 2.f;
    }
    // Reset quad geometry
    uvs[0] = { 0, 0 };
	uvs[1] = { 1, 0 };
	uvs[2] = { 1, 1 };
	uvs[3] = { 0, 0 };
	uvs[4] = { 1, 1 };
	uvs[5] = { 0, 1 };

    // triangle 1
    verts[0] = { -0.5, -0.5f, 0 };
    verts[1] = { 0.5, -0.5, 0 };
    verts[2] = { 0.5, 0.5, 0 };
    // triangle 2
    verts[3] = { -0.5, -0.5, 0 };
    verts[4] = { 0.5, 0.5, 0 };
    verts[5] = { -0.5, 0.5, 0 };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadBuffer), quadBuffer);
    CHECK_GL_ERR
    #endif
}

////////////////////////////////////////////////////////////
// Draw text old Partial implementation
////////////////////////////////////////////////////////////
static void ZR_ExecuteTextDraw_Old(
    const ZRDrawCmd_Text* cmd, M4x4* prj, char* str, ZRPerformanceStats* stats)
{
    // Size of quad on screen, to offset verts from char position
    f32 charHalfWidth = (cmd->charSize * (cmd->aspectRatio - 1.f)) / 2.f;
    f32 charHalfHeight = cmd->charSize / 2.f;

    Vec3 origin = cmd->origin;
    switch (cmd->alignmentMode)
    {
		case ZR_TEXT_ALIGNMENT_TOP_LEFT:
		case ZR_TEXT_ALIGNMENT_CENTRE:
        case ZR_TEXT_ALIGNMENT_TOP_RIGHT:
        {
            origin.x += charHalfWidth;
            origin.y -= charHalfHeight;
        } break;
        default:
        {
            printf("Unsupported text alignment mode %d\n", cmd->alignmentMode);
            return;
        } break;
    }
    
    #if 1
    // Get character quad prefab and use to stamp out characters.
    ZRDBMesh* mesh = AssetDb()->GetMeshByName(AssetDb(), ZRDB_MESH_NAME_DYNAMIC_QUAD);
    if (mesh == NULL) { return; }
    ZRDBTexture* tex = AssetDb()->GetTextureByName(AssetDb(), ZRDB_DEFAULT_CHARSET_NAME);
    if (tex == NULL) { return; }
    M4x4_CREATE(modelView)
    // Setup shader
    
    ZRGL_SetupProg_Text(
        prj,
        &modelView,
        g_programs[ZR_SHADER_TYPE_TEXT].handle,
        tex->apiHandle);

    glBindVertexArray(mesh->handles.vao);
    CHECK_GL_ERR
    glBindBuffer(GL_ARRAY_BUFFER, mesh->handles.vbo);
    CHECK_GL_ERR
    // glBindTexture(GL_TEXTURE_2D, tex->apiHandle);
    // CHECK_GL_ERR

    // Setup buffers
    // Verts and uvs must be rewritten. Normals can remain the same.
    // NOTE: This is assuming a non-interleaved geometry layout!
    u8 quadBuffer[ZRGL_BYTES_FOR_QUAD_VERTS + ZRGL_BYTES_FOR_QUAD_UVS];
    Vec3* verts = (Vec3*)quadBuffer;
    Vec2* uvs = (Vec2*)(quadBuffer + ZRGL_BYTES_FOR_QUAD_VERTS);

    // Divide up character set tiles
    const f32 stride = 1.f / (f32)ZRGL_ASCI_CHARSET_CHARS_WIDE;

    char* txt = str;//(char*)((u8*)cmd + cmd->offsetToString);

    Vec3 pos = origin;

    while (*txt != NULL)
    {
        char c = *txt;
        txt++;

        if (c == '\n')
        {
            pos.x = origin.x;
            pos.y -= charHalfHeight * 2.f;
            continue;
        }
        else if (c == '\t')
        {
            pos.x += (charHalfWidth * 2.f) * 4.f;
            continue;
        }

        // convert asci to sheet position
        i32 sheetX = c % ZRGL_ASCI_CHARSET_CHARS_WIDE;
        i32 sheetY = c / ZRGL_ASCI_CHARSET_CHARS_WIDE;
        // Sheet is top -> down but opengl is bottom -> up so flip the Y coord
    	sheetY = (16 - 1) - sheetY;

        // position on sheet
        f32 uvLeft = stride * (f32)sheetX;
        f32 uvRight = uvLeft + stride;

        f32 uvBottom = stride * (f32)sheetY;
        f32 uvTop = uvBottom + stride;

        uvs[0] = { uvLeft, uvBottom };
	    uvs[1] = { uvRight, uvBottom };
	    uvs[2] = { uvRight, uvTop };
	    uvs[3] = { uvLeft, uvBottom };
	    uvs[4] = { uvRight, uvTop };
	    uvs[5] = { uvLeft, uvTop };

        // triangle 1
        verts[0] = { pos.x - charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[1] = { pos.x + charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[2] = { pos.x + charHalfWidth, pos.y + charHalfHeight, pos.z };
        // triangle 2
        verts[3] = { pos.x - charHalfWidth, pos.y - charHalfHeight, pos.z };
        verts[4] = { pos.x + charHalfWidth, pos.y + charHalfHeight, pos.z };
        verts[5] = { pos.x - charHalfWidth, pos.y + charHalfHeight, pos.z };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadBuffer), quadBuffer);
        CHECK_GL_ERR
        glDrawArrays(GL_TRIANGLES, 0, 6);
        CHECK_GL_ERR

        stats->drawCalls++;
        stats->trisSingle += 2;

        pos.x += charHalfWidth * 2.f;
    }
    // Reset quad geometry
    uvs[0] = { 0, 0 };
	uvs[1] = { 1, 0 };
	uvs[2] = { 1, 1 };
	uvs[3] = { 0, 0 };
	uvs[4] = { 1, 1 };
	uvs[5] = { 0, 1 };

    // triangle 1
    verts[0] = { -0.5, -0.5f, 0 };
    verts[1] = { 0.5, -0.5, 0 };
    verts[2] = { 0.5, 0.5, 0 };
    // triangle 2
    verts[3] = { -0.5, -0.5, 0 };
    verts[4] = { 0.5, 0.5, 0 };
    verts[5] = { -0.5, 0.5, 0 };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadBuffer), quadBuffer);
    CHECK_GL_ERR
    #endif
}


