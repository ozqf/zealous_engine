#ifndef GAME_HUD_H
#define GAME_HUD_H

#include "../../../headers/common/ze_common.h"
#include "../../zqf_renderer.h"
#include "game_draw.h"

extern "C" void Hud_AddViewModels(
	ClientRenderer* cr,
    ZRViewFrame* frame,
    ZRSceneFrame* scene,
    ClientRenderSettings cfg);

extern "C" void Hud_AddDrawObjects(
	ClientRenderer* cr, ZRViewFrame* frame, ClientRenderSettings cfg);

#endif // GAME_HUD_H