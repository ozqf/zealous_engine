#pragma once

#define DEFAULT_CONSOLE_CHARSET_PATH "textures\\charset_128x128.bmp"

void            Tex_Init(Heap* heap, AppPlatform platform);
i32             Tex_RenderModuleReloaded();
void            Tex_DebugPrint();
i32             Tex_GetTextureIndexByName(char* textureName);
void            Tex_LoadTextureList(char** textures);
i32             Tex_LoadAndBindTexture(char *filePath);
void            Tex_BindTexture(Texture2DHeader *header);
Texture2DHeader* Tex_GetTextureByName(char* textureName);
Texture2DHeader* Tex_AllocateTexture(char* name, i32 width, i32 height);
