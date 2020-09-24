
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

internal Transform g_camera;
internal SimActorInput g_debugInput;
internal SimPlayer g_player;
internal i32 g_bIsRunning = NO;
internal u32 g_debugFlags;
internal ClientView g_view;

extern "C" Transform CL_GetCamera(SimScene* sim)
{
    SimEntity* ent = Sim_GetEntityBySerial(sim, g_player.avatarId);
    if (ent != NULL)
    {
        return ent->body.t;
        
    }
    if (IF_BIT(g_debugFlags, CL_DEBUG_FLAG_DEBUG_CAMERA)) { return g_camera; }
    return sim->observePos;
}

extern "C" ClientView CL_GetClientView(SimScene* sim)
{
    return g_view;
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

extern "C" void CL_Init(ZE_FatalErrorFunction fatalFunc)
{
    CL_CreateActions(&g_inputActions);

    Transform_SetToIdentity(&g_camera);
    g_camera.pos.z = 10;
    g_camera.pos.y += 34;
    Transform_SetRotation(&g_camera, -(80.0f    * DEG2RAD), 0, 0);
	g_debugInput = {};
	g_debugInput.degrees.x = -80;

    g_view = {};
    g_view.textFieldFlags |= CLR_HUD_ITEM_SPAWN_PROMPT;
}

extern "C" void CLG_Start(SimScene* sim)
{
    printf("CL Start\n");
    g_bIsRunning = YES;
    g_player = {};

    g_view = {};
    if (sim->gameRules == SIM_GAME_RULES_NONE)
    {
        g_view.textFieldFlags |= CLR_HUD_ITEM_TITLE;
    }
    else if (sim->gameRules == SIM_GAME_RULES_SURVIVAL)
    {
        g_view.textFieldFlags |= CLR_HUD_ITEM_SPAWN_PROMPT;
    }
}

extern "C" void CL_Stop()
{
    printf("CL Stop\n");
    g_bIsRunning = NO;
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
    if (!g_bIsRunning) { return; }
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

internal void CL_CheckForSimEvents(ZEByteBuffer* buf)
{
    u8* read = buf->start;
    u8* end = buf->cursor;
    while (read < end)
    {
        ZECommand* header = (ZECommand*)read;
        read += header->size;
        if (header->type != SIM_CMD_TYPE_RESTORE_ENTITY) { continue; }
        SimEvent_Spawn* cmd = (SimEvent_Spawn*)header;
        if (cmd->serial != g_player.avatarId) { continue; }
        printf("Client saw self spawn\n");
        g_view.textFieldFlags ^= CLR_HUD_ITEM_SPAWN_PROMPT;
        g_view.textFieldFlags |= CLR_HUD_ITEM_PLAYER_STATUS;
        g_view.textFieldFlags |= CLR_HUD_ITEM_CROSSHAIR;
    }
}

extern "C" void CL_RegisterLocalPlayer(SimScene* sim, SimPlayer plyr)
{
    g_player = plyr;
    sim->localAvatarId = g_player.avatarId;
    printf("GCL plyr Id %d avatar %d\n", g_player.id, g_player.avatarId);
}

extern "C" void CL_PreTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
{
    if (!g_bIsRunning) { return; }

    CL_UpdateActorInput(&g_inputActions, &g_debugInput);
    Sim_TickDebugCamera(&g_camera, g_debugInput, 16, delta);

    SimEvent_PlayerInput input = {};
    input.header.sentinel = ZCMD_SENTINEL;
    input.header.size = sizeof(SimEvent_PlayerInput);
    input.header.type = SIM_CMD_TYPE_PLAYER_INPUT;
    input.playerId = g_player.id;
    input.input = g_debugInput;

    ZCmd_Write(&input.header, &buf->GetWrite()->cursor);
    CL_CheckForSimEvents(buf->GetRead());
}

extern "C" void CL_PostTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
{
    if (!g_bIsRunning) { return; }
    // update view and detech player state changes
    SimEntity* ent = Sim_GetEntityBySerial(sim, g_player.avatarId);
    if (ent != NULL)
    {
        g_view.camera = ent->body.t;
        //g_view.showHud = 1;
        g_view.rightHand = 1;
        g_view.leftHand = 1;
    }
    else
    {
        g_view.camera = g_camera;
        //g_view.showHud = NO;
        g_view.rightHand = 0;
        g_view.leftHand = 0;
    }
    
    
}
