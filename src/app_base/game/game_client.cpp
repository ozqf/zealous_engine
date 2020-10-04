
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

extern "C" Transform CL_GetCamera(SimScene* sim)
{
    SimEntity* ent = Sim_GetEntityBySerial(sim, g_cl.avatarId);
    if (ent != NULL)
    {
        return ent->body.t;
        
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

internal void CL_CreateActions(InputActionSet* actions)
{
    Input_InitAction(actions, Z_INPUT_CODE_V, "Debug Forward");
    Input_InitAction(actions, Z_INPUT_CODE_C, "Debug Backward");
	Input_InitAction(actions, Z_INPUT_CODE_X, "Debug Camera");
    Input_InitAction(actions, Z_INPUT_CODE_R, "Reset");
    Input_InitAction(actions, Z_INPUT_CODE_ESCAPE, "Menu");

    Input_InitAction(actions, Z_INPUT_CODE_A, "Move Left");
    Input_InitAction(actions, Z_INPUT_CODE_D, "Move Right");
    Input_InitAction(actions, Z_INPUT_CODE_W, "Move Forward");
    Input_InitAction(actions, Z_INPUT_CODE_S, "Move Backward");
    Input_InitAction(actions, Z_INPUT_CODE_SPACE, "Move Up");
    Input_InitAction(actions, Z_INPUT_CODE_LEFT_CONTROL, "Move Down");
    Input_InitAction(actions, Z_INPUT_CODE_LEFT_SHIFT, "MoveSpecial1");
    Input_InitAction(actions, Z_INPUT_CODE_Q, "Roll Left");
    Input_InitAction(actions, Z_INPUT_CODE_E, "Roll Right");

    Input_InitAction(actions, Z_INPUT_CODE_G, "Spawn Test");
    Input_InitAction(actions, Z_INPUT_CODE_H, "Spawn Test 2");
    Input_InitAction(actions, Z_INPUT_CODE_J, "Spawn Test 3");
    Input_InitAction(actions, Z_INPUT_CODE_P, "Pause");

    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_POS_X, "Mouse Pos X");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_POS_Y, "Mouse Pos Y");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_MOVE_X, "Mouse Move X");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_MOVE_Y, "Mouse Move Y");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_1, "Attack1");
    Input_InitAction(actions, Z_INPUT_CODE_MOUSE_2, "Attack2");

    Input_InitAction(actions, Z_INPUT_CODE_1, "Slot1");
    Input_InitAction(actions, Z_INPUT_CODE_2, "Slot2");
    Input_InitAction(actions, Z_INPUT_CODE_3, "Slot3");
    Input_InitAction(actions, Z_INPUT_CODE_4, "Slot4");

    // Robotron style shooting
    Input_InitAction(actions, Z_INPUT_CODE_LEFT, "Shoot Left");
    Input_InitAction(actions, Z_INPUT_CODE_RIGHT, "Shoot Right");
    Input_InitAction(actions, Z_INPUT_CODE_UP, "Shoot Up");
    Input_InitAction(actions, Z_INPUT_CODE_DOWN, "Shoot Down");
}

extern "C" void CL_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db)
{
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

    g_cl.view = {};
    if (sim->info.gameRules == SIM_GAME_RULES_NONE)
    {
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_TITLE;
    }
    else if (sim->info.gameRules == SIM_GAME_RULES_SURVIVAL)
    {
        g_cl.view.textFieldFlags |= CLR_HUD_ITEM_SPAWN_PROMPT;
    }
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
	// record last frame
	input->prevButtons = input->buttons;
    // Clear buttons and rebuild. Keep mouse position values
    u32 flags = 0;

    const f32 mouseMoveMultiplier = 80;
    const f32 mouseInvertedMultiplier = -1;
    const f32 KEY_TURN_RATE = 4.f;

	// Read
    CL_InputCheckButton(actions, "Move Forward", &flags, ACTOR_INPUT_MOVE_FORWARD);
    CL_InputCheckButton(actions, "Move Backward", &flags, ACTOR_INPUT_MOVE_BACKWARD);
    CL_InputCheckButton(actions, "Move Left", &flags, ACTOR_INPUT_MOVE_LEFT);
    CL_InputCheckButton(actions, "Move Right", &flags, ACTOR_INPUT_MOVE_RIGHT);

    CL_InputCheckButton(actions, "Move Up", &flags, ACTOR_INPUT_MOVE_UP);
    CL_InputCheckButton(actions, "Move Down", &flags, ACTOR_INPUT_MOVE_DOWN);

    CL_InputCheckButton(actions, "MoveSpecial1", &flags, ACTOR_INPUT_MOVE_SPECIAL1);

    CL_InputCheckButton(actions, "Shoot Up", &flags, ACTOR_INPUT_SHOOT_UP);
    CL_InputCheckButton(actions, "Shoot Down", &flags, ACTOR_INPUT_SHOOT_DOWN);
    CL_InputCheckButton(actions, "Shoot Left", &flags, ACTOR_INPUT_SHOOT_LEFT);
    CL_InputCheckButton(actions, "Shoot Right", &flags, ACTOR_INPUT_SHOOT_RIGHT);

    CL_InputCheckButton(actions, "Attack1", &flags, ACTOR_INPUT_ATTACK);
    CL_InputCheckButton(actions, "Attack2", &flags, ACTOR_INPUT_ATTACK2);

    CL_InputCheckButton(actions, "Slot1", &flags, ACTOR_INPUT_SLOT_1);
    CL_InputCheckButton(actions, "Slot2", &flags, ACTOR_INPUT_SLOT_2);
    CL_InputCheckButton(actions, "Slot3", &flags, ACTOR_INPUT_SLOT_3);
    CL_InputCheckButton(actions, "Slot4", &flags, ACTOR_INPUT_SLOT_4);

    #if 0 // old mouse input reads movement in pixels directly - resolution dependent!
    f32 mouseX = ((f32)Input_GetActionValue(actions, "Mouse Move X") / (f32)Z_INPUT_MOUSE_SCALAR);
    f32 mouseY = ((f32)Input_GetActionValue(actions, "Mouse Move Y") / (f32)Z_INPUT_MOUSE_SCALAR);
    printf("CL Mouse move X/Y %f, %f\n", mouseX, mouseY);
    printf("CL normalised mouse move: %f, %f\n",
        Input_GetActionValueNormalised(actions, "Mouse Move X") / (f32)1000,
        Input_GetActionValueNormalised(actions, "Mouse Move Y") / (f32)1000
        );
    #endif

    f32 mouseX = Input_GetActionValueNormalised(actions, "Mouse Move X") / Z_INPUT_MOUSE_SCALAR;
    f32 mouseY = Input_GetActionValueNormalised(actions, "Mouse Move Y") / Z_INPUT_MOUSE_SCALAR;
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
        g_cl.view.health = ent->life.health;
        //g_cl.view.showHud = 1;
        g_cl.view.rightHand = 1;
        g_cl.view.leftHand = 1;
    }
    else
    {
        g_cl.view.camera = g_cl.camera;
        g_cl.view.health = -999;
        //g_cl.view.showHud = NO;
        g_cl.view.rightHand = 0;
        g_cl.view.leftHand = 0;
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
