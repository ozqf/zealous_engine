
#include "game_client.h"
#include "../../sim/sim.h"

#define GAME_MAX_INPUT_ACTIONS 256
internal InputAction g_inputActionItems[GAME_MAX_INPUT_ACTIONS];
internal InputActionSet g_inputActions = {
    g_inputActionItems,
    0
};

#define CL_DEBUG_FLAG_DRAW_LOCAL_SERVER (1 << 0)
#define CL_DEBUG_FLAG_NO_ENEMY_TICK (1 << 1)
#define CL_DEBUG_FLAG_NO_PLAYER_SMOOTHING (1 << 2)
#define CL_DEBUG_FLAG_DRAW_REAL_LOCAL_POSITION (1 << 3)
#define CL_DEBUG_FLAG_DEBUG_CAMERA (1 << 4)

#define CL_DEBUG_FLAG_VERBOSE_FRAME (1 << 31)

struct GameClient
{
    Transform camera;
    SimActorInput debugInput;
    i32 playerId;
    i32 avatarId;
    i32 remoteState;
    i32 bIsRunning;
    u32 debugFlags;
    ClientRenderSettings rendCfg;
    ClientView view;
};

internal GameClient g_cl = {};
internal ClientRenderer* g_rend;
internal SimScene* g_sim;
internal ZRAssetDB *g_db;

extern "C" Transform CL_GetCamera(SimScene* sim)
{
    SimEntity* ent = Sim_GetEntityBySerial(sim, g_cl.avatarId);
    if (ent != NULL)
    {
        Transform t = ent->body.t;
        t.pos.y += 0.5f;
        return t;
        
    }
    if (IF_BIT(g_cl.debugFlags, CL_DEBUG_FLAG_DEBUG_CAMERA)) { return g_cl.camera; }
    return sim->info.observePos;
}

extern "C" ClientView CL_GetClientView(SimScene* sim)
{
    return g_cl.view;
}

extern "C" void CL_ClearActionInputs()
{
    Input_ClearValues(&g_inputActions);
}

internal void CL_RefreshHUDState(i32 gameRules, i32 playerState)
{
    g_cl.view.textFieldFlags = 0;
    if (gameRules == SIM_GAME_RULES_NONE)
    {
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_TITLE;
        return;
    }
    switch (playerState)
    {
        case SIM_PLAYER_STATE_IN_GAME:
        //g_cl.view.textFieldFlags ^= CLR_HUD_ITEM_SPAWN_PROMPT;
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_PLAYER_STATUS;
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_CROSSHAIR;
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_CROSSHAIR;
        break;
        case SIM_PLAYER_STATE_DEAD:
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_DEAD;
        break;
        case SIM_PLAYER_STATE_OBSERVING:
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_SPAWN_PROMPT;
        break;
        case SIM_PLAYER_STATE_NONE:
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_TITLE;
        break;
    }
}

internal void CL_CreateActions(InputActionSet* actions)
{
    Input_InitAction(actions, Z_INPUT_CODE_V, Z_INPUT_CODE_NULL, "debug_forward");
    Input_InitAction(actions, Z_INPUT_CODE_C, Z_INPUT_CODE_NULL, "debug_backward");
	Input_InitAction(actions, Z_INPUT_CODE_X, Z_INPUT_CODE_NULL, "debug_camera");
    Input_InitAction(actions, Z_INPUT_CODE_R, Z_INPUT_CODE_NULL, "reset");
    Input_InitAction(actions, Z_INPUT_CODE_ESCAPE, Z_INPUT_CODE_NULL, "menu");

    Input_InitAction(actions, Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    Input_InitAction(actions, Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    Input_InitAction(actions, Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_forward");
    Input_InitAction(actions, Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_backward");
    Input_InitAction(actions, Z_INPUT_CODE_SPACE, Z_INPUT_CODE_NULL, "move_up");
    Input_InitAction(actions, Z_INPUT_CODE_LEFT_CONTROL, Z_INPUT_CODE_NULL, "move_down");
    Input_InitAction(actions, Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, "move_special_1");
    Input_InitAction(actions, Z_INPUT_CODE_Q, Z_INPUT_CODE_NULL, "roll_left");
    Input_InitAction(actions, Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, "roll_right");

    Input_InitAction(actions, Z_INPUT_CODE_G, Z_INPUT_CODE_NULL, "spawn_test");
    Input_InitAction(actions, Z_INPUT_CODE_H, Z_INPUT_CODE_NULL, "spawn_test_2");
    Input_InitAction(actions, Z_INPUT_CODE_J, Z_INPUT_CODE_NULL, "spawn_test_3");
    Input_InitAction(actions, Z_INPUT_CODE_P, Z_INPUT_CODE_NULL, "pause");

    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_POS_X, Z_INPUT_CODE_NULL, "mouse_pos_x");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_POS_Y, Z_INPUT_CODE_NULL, "mouse_pos_y");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_MOVE_X, Z_INPUT_CODE_NULL, "mouse_move_x");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_MOVE_Y, Z_INPUT_CODE_NULL, "mouse_move_y");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_1, Z_INPUT_CODE_0, "attack_1");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_2, Z_INPUT_CODE_NULL, "attack_2");

    Input_InitAction(actions, Z_INPUT_CODE_1, Z_INPUT_CODE_NULL, "slot_1");
    Input_InitAction(actions, Z_INPUT_CODE_2, Z_INPUT_CODE_NULL, "slot_2");
    Input_InitAction(actions, Z_INPUT_CODE_3, Z_INPUT_CODE_NULL, "slot_3");
    Input_InitAction(actions, Z_INPUT_CODE_4, Z_INPUT_CODE_NULL, "slot_4");

    // Robotron style shooting
    Input_InitAction(actions, Z_INPUT_CODE_LEFT, Z_INPUT_CODE_NULL, "shoot_left");
    Input_InitAction(actions, Z_INPUT_CODE_RIGHT, Z_INPUT_CODE_NULL, "shoot_right");
    Input_InitAction(actions, Z_INPUT_CODE_UP, Z_INPUT_CODE_NULL, "shoot_up");
    Input_InitAction(actions, Z_INPUT_CODE_DOWN, Z_INPUT_CODE_NULL, "shoot_down");
}

extern "C" void CL_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db)
{
    g_db = db;

    CL_CreateActions(&g_inputActions);

    Transform_SetToIdentity(&g_cl.camera);
    g_cl.camera.pos.z = 10;
    g_cl.camera.pos.y += 34;
    Transform_SetRotation(&g_cl.camera, -(80.0f    * DEG2RAD), 0, 0);
	g_cl.debugInput = {};
	g_cl.debugInput.degrees.x = -80;
    
	// scene render
	g_cl.rendCfg = {};
	g_cl.rendCfg.extraLightsMax = 16;
	g_cl.rendCfg.worldLightsMax = 16;
	g_rend = CLR_Create(fatalFunc, db, 128);

    g_cl.view = {};
    g_cl.view.textFieldFlags |= CLR_HUD_ITEM_SPAWN_PROMPT;
}

extern "C" void CL_Start(SimScene* sim)
{
    printf("CL Start\n");
    g_cl.bIsRunning = YES;
    g_cl.playerId = 0;
    g_cl.avatarId = 0;
    g_sim = sim;

    g_cl.view = {};
    g_cl.view.rightHandmatIndex = g_db->GetMaterialByName(g_db, SIM_MAT_WORLD)->header.index;
    g_cl.view.leftHandmatIndex = g_cl.view.rightHandmatIndex;
    CL_RefreshHUDState(sim->info.gameRules, SIM_PLAYER_STATE_NONE);
}

extern "C" void CL_Stop()
{
    printf("CL Stop\n");
    g_cl.bIsRunning = NO;
}

extern "C" void CL_Save(SimScene* sim, SimSaveFileInfo* saveInfo, i32 file, ZEFileIO files)
{
    saveInfo->client = files.FilePosition(file);
    saveInfo->numClientBytes = sizeof(GameClient);
    files.WriteToFile(file, (u8*)&g_cl, sizeof(GameClient));
}

extern "C" void CL_Resume(SimScene* sim, SimSaveFileInfo* saveInfo, ZEBuffer* saveData)
{
    if (saveInfo->numClientBytes != sizeof(GameClient))
    {
        printf("Cl_Read incorrect size in save file\n");
        return;
    }
    u8* read = saveData->GetAtOffset(saveInfo->client);
    g_cl = *((GameClient*)read);
}

extern "C" void CL_InputCheckButton(
    InputActionSet* actions,
    char* inputName,
    u32* flags,
    u32 buttonFlag)
{
    if (Input_GetActionValue(actions, inputName))
    {
        *flags |= buttonFlag;
    }
}

extern "C" void CL_ReadInputEvent(SysInputEvent* ev, frameInt frameNumber)
{
    if (!g_cl.bIsRunning) { return; }
    Input_TestForAction(&g_inputActions, ev->value, ev->normalised, ev->inputID, frameNumber);
}

internal void CL_UpdateActorInput(InputActionSet* actions, SimActorInput* input)
{
	// record_Last frame
	input->prevButtons = input->buttons;
    // Clear buttons and rebuild. Keep mouse position values
    u32 flags = 0;

    const f32 mouseMoveMultiplier = 80;
    const f32 mouseInvertedMultiplier = -1;
    const f32 KEY_TURN_RATE = 4.f;

	// Read
    CL_InputCheckButton(actions, "move_forward", &flags, ACTOR_INPUT_MOVE_FORWARD);
    CL_InputCheckButton(actions, "move_backward", &flags, ACTOR_INPUT_MOVE_BACKWARD);
    CL_InputCheckButton(actions, "move_left", &flags, ACTOR_INPUT_MOVE_LEFT);
    CL_InputCheckButton(actions, "move_right", &flags, ACTOR_INPUT_MOVE_RIGHT);

    CL_InputCheckButton(actions, "move_up", &flags, ACTOR_INPUT_MOVE_UP);
    CL_InputCheckButton(actions, "move_down", &flags, ACTOR_INPUT_MOVE_DOWN);

    CL_InputCheckButton(actions, "move_special_1", &flags, ACTOR_INPUT_MOVE_SPECIAL1);

    CL_InputCheckButton(actions, "shoot_up", &flags, ACTOR_INPUT_SHOOT_UP);
    CL_InputCheckButton(actions, "shoot_down", &flags, ACTOR_INPUT_SHOOT_DOWN);
    CL_InputCheckButton(actions, "shoot_left", &flags, ACTOR_INPUT_SHOOT_LEFT);
    CL_InputCheckButton(actions, "shoot_right", &flags, ACTOR_INPUT_SHOOT_RIGHT);

    CL_InputCheckButton(actions, "attack_1", &flags, ACTOR_INPUT_ATTACK);
    CL_InputCheckButton(actions, "attack_2", &flags, ACTOR_INPUT_ATTACK2);

    CL_InputCheckButton(actions, "slot_1", &flags, ACTOR_INPUT_SLOT_1);
    CL_InputCheckButton(actions, "slot_2", &flags, ACTOR_INPUT_SLOT_2);
    CL_InputCheckButton(actions, "slot_3", &flags, ACTOR_INPUT_SLOT_3);
    CL_InputCheckButton(actions, "slot_4", &flags, ACTOR_INPUT_SLOT_4);

    #if 0 // old mouse input reads movement in pixels directly - resolution dependent!
    f32 mouseX = ((f32)Input_GetActionValue(actions, "mouse_move_x") / (f32)Z_INPUT_MOUSE_SCALAR);
    f32 mouseY = ((f32)Input_GetActionValue(actions, "mouse_move_y") / (f32)Z_INPUT_MOUSE_SCALAR);
    printf("CL Mouse move X/Y %f, %f\n", mouseX, mouseY);
    printf("CL normalised mouse move: %f, %f\n",
        Input_GetActionValueNormalised(actions, "mouse_move_x") / (f32)1000,
        Input_GetActionValueNormalised(actions, "mouse_move_y") / (f32)1000
        );
    #endif

    f32 mouseX = Input_GetActionValueNormalised(actions, "mouse_move_x") / Z_INPUT_MOUSE_SCALAR;
    f32 mouseY = Input_GetActionValueNormalised(actions, "mouse_move_y") / Z_INPUT_MOUSE_SCALAR;
    const f32 sensitivity = 2.f;
    mouseX *= sensitivity;
    mouseY *= sensitivity;
	// apply
    input->buttons = flags;

    // Apply yaw
    input->degrees.y -= mouseX * mouseMoveMultiplier;;
    input->degrees.y = COM_CapAngleDegrees(input->degrees.y);

    if (flags & ACTOR_INPUT_SHOOT_LEFT)
    { input->degrees.y += KEY_TURN_RATE; }
    if (flags & ACTOR_INPUT_SHOOT_RIGHT)
    { input->degrees.y -= KEY_TURN_RATE; }
    
    // Apply pitch
    input->degrees.x -= ((mouseY
		* mouseMoveMultiplier))
        * mouseInvertedMultiplier;
    
    if (flags & ACTOR_INPUT_SHOOT_UP)
    { input->degrees.x += (KEY_TURN_RATE * mouseInvertedMultiplier); }
    if (flags & ACTOR_INPUT_SHOOT_DOWN)
    { input->degrees.x -= (KEY_TURN_RATE * mouseInvertedMultiplier); }

	if (input->degrees.x < -89)
	{
		input->degrees.x = -89;
	}
	if (input->degrees.x > 89)
	{
		input->degrees.x = 89;
	}
}

internal void CL_CheckForSimEvents(ZEBuffer* buf)
{
    ZCMD_BEGIN_ITERATE(buf)
        switch (cmdHeader->type)
        {
            case SIM_CMD_TYPE_PARTICLES:
            {
                SimEvent_Particles* cmd = (SimEvent_Particles*)cmdHeader;
                for (i32 i = 0; i < 5; ++i)
                {
                    f32 rand = COM_STDRandf32();
                    Vec3 vel;
                    vel.x = COM_STDRandomInRange(-15, 15);
                    vel.y = COM_STDRandomInRange(-10, 15);
                    vel.z = COM_STDRandomInRange(-15, 15);
                    CLR_SpawnTestParticle(g_rend, CLR_PARTICLE_TYPE_TEST, cmd->pos, vel);
                }
            } break;
            case SIM_CMD_TYPE_RESTORE_ENTITY:
            {
                SimEvent_Spawn* cmd = (SimEvent_Spawn*)cmdHeader;
                if (cmd->serial != g_cl.playerId) { continue; }

                printf("Client saw self spawn\n");
                g_cl.view.textFieldFlags ^= CLR_HUD_ITEM_SPAWN_PROMPT;
                g_cl.view.textFieldFlags |= CLR_HUD_ITEM_PLAYER_STATUS;
                g_cl.view.textFieldFlags |= CLR_HUD_ITEM_CROSSHAIR;
            }
            break;
            case SIM_CMD_TYPE_REMOVE_ENTITY:
            {
                SimEvent_RemoveEnt* cmd = (SimEvent_RemoveEnt*)cmdHeader;
                if (cmd->entityId != g_cl.avatarId) { continue; }
                printf("Client saw avatar removal\n");
            }
            break;
            case SIM_CMD_TYPE_PLAYER_STATE:
            {
                //SimEvent_PlayerState* cmd = (SimEvent_PlayerState*)
                ZCMD_CAST_DOWN(cmd, SimEvent_PlayerState, cmdHeader)
				if (cmd->playerId == g_cl.playerId)
				{
					printf("Client saw own player state\n"); 
					g_cl.avatarId = cmd->avatarId;
					g_cl.remoteState = cmd->state;
                    CL_RefreshHUDState(g_sim->info.gameRules, g_cl.remoteState);
				}
            } break;
        }
    ZCMD_END_ITERATE

    #if 0
    u8* read = buf->start;
    u8* end = buf->cursor;
    while (read < end)
    {
        ZECommand* header = (ZECommand*)read;
        read += header->size;
        switch (header->type)
        {
            case SIM_CMD_TYPE_PARTICLES:
            {
                SimEvent_Particles* cmd = (SimEvent_Particles*)header;
                for (i32 i = 0; i < 5; ++i)
                {
                    f32 rand = COM_STDRandf32();
                    Vec3 vel;
                    vel.x = COM_STDRandomInRange(-15, 15);
                    vel.y = COM_STDRandomInRange(-10, 15);
                    vel.z = COM_STDRandomInRange(-15, 15);
                    CLR_SpawnTestParticle(g_rend, CLR_PARTICLE_TYPE_TEST, cmd->pos, vel);
                }
            } break;
            case SIM_CMD_TYPE_RESTORE_ENTITY:
            {
                SimEvent_Spawn* cmd = (SimEvent_Spawn*)header;
                if (cmd->serial != g_player.avatarId) { continue; }

                printf("Client saw self spawn\n");
                g_cl.view.textFieldFlags ^= CLR_HUD_ITEM_SPAWN_PROMPT;
                g_cl.view.textFieldFlags |= CLR_HUD_ITEM_PLAYER_STATUS;
                g_cl.view.textFieldFlags |= CLR_HUD_ITEM_CROSSHAIR;
            }
            break;
            case SIM_CMD_TYPE_REMOVE_ENTITY:
            {
                SimEvent_RemoveEnt* cmd = (SimEvent_RemoveEnt*)header;
                if (cmd->entityId != g_player.avatarId) { continue; }
                printf("Client saw avatar removal\n");
            }
            break;
        }
    }
    #endif
}

extern "C" void CL_RegisterLocalPlayer(SimScene* sim, i32 playerId)
{
    g_cl.playerId = playerId;
    //sim->info.localAvatarId = g_player.avatarId;
    printf("GCL plyr Id %d avatar %d\n", g_cl.playerId, g_cl.avatarId);
}

extern "C" void CL_PreTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta)
{
    if (!g_cl.bIsRunning) { return; }

    CL_UpdateActorInput(&g_inputActions, &g_cl.debugInput);
    Sim_TickDebugCamera(&g_cl.camera, g_cl.debugInput, 16, delta);

    // Write input update to Sim
    SimEvent_PlayerInput input = {};
    input.header.sentinel = ZCMD_SENTINEL;
    input.header.size = sizeof(SimEvent_PlayerInput);
    input.header.type = SIM_CMD_TYPE_PLAYER_INPUT;
    input.playerId = g_cl.playerId;
    input.input = g_cl.debugInput;

    ZCmd_Write(&input.header, &buf->GetWrite()->cursor);
    CL_CheckForSimEvents(buf->GetRead());
}

internal Transform CL_CalcHandWorldPos(SimScene* sim, Transform* camera, Vec3 handOffset)
{
    TRANSFORM_CREATE(base)
    base.pos = handOffset;
    Transform* links[2];
    links[0] = camera;
    links[1] = &base;
    Transform_ApplyChain(&base, links, 2);
    return base;
}

extern "C" void CL_PostTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta)
{
    if (!g_cl.bIsRunning) { return; }
    // update view and detech player state changes
    SimEntity* ent = NULL;
    if (g_cl.avatarId != SIM_ENT_NULL_SERIAL)
    {
        ent = Sim_GetEntityBySerial(sim, g_cl.avatarId);
    }
    if (ent != NULL)
    {
        g_cl.view.camera = ent->body.t;
        // offset camera up a bit
        //g_cl.view.camera.pos.y += 0.5f;
        g_cl.view.health = ent->life.health;
        //g_cl.view.showHud = 1;
        SimInventoryItem* item = SVI_GetItem(ent->inventory.index);
        g_cl.view.rightHandModelIndex = g_db->GetMeshByName(g_db, item->model)->header.index;
        //g_cl.view.leftHand = 1;

        // TODO: Setting position of hand here causes jitter
        // as this update rate does not match frame rate

        // Calculate position of right hand
        g_cl.view.rightHand = CL_CalcHandWorldPos(sim, &g_cl.view.camera, { 0.5f, -0.5f, -1.f});
        // Vec3 pos = g_cl.view.rightHand.pos;
        // printf("CL Set right hand pos %.3f, %.3f, %.3f\n", pos.x, pos.y, pos.z);
    }
    else
    {
        g_cl.view.camera = g_cl.camera;
        g_cl.view.health = -999;
        //g_cl.view.showHud = NO;
        g_cl.view.rightHandModelIndex = 0;
        g_cl.view.leftHandModelIndex = 0;
    }
    // Update particles
    CLR_TickTestParticles(g_rend, delta);
}

extern "C" void CL_WriteDrawFrame(SimScene* sim, ZRViewFrame* frame)
{
    Transform cam = CL_GetCamera(sim);
	g_cl.rendCfg.viewModels = CL_GetClientView(sim);
	CLR_WriteDrawFrame(g_rend, frame, sim, &cam, NULL, 0, g_cl.rendCfg);
}

extern "C" void CL_ClearDebugFlags()
{
    g_cl.rendCfg.debugFlags = 0;
}

extern "C" void CL_ToggleDrawFlag(i32 flag)
{
    g_cl.rendCfg.debugFlags ^= flag;
}
