#pragma once

#include "client.h"

#define CLI_MAX_RESPONSE_RECORDS 60

internal S2C_InputResponse g_serverResponses[CLI_MAX_RESPONSE_RECORDS];

/////////////////////////////////////
// Record server responses
/////////////////////////////////////
internal void CLI_RecordServerResponse(
    S2C_InputResponse* list, S2C_InputResponse* item)
{
    i32 i = item->header.tick % CLI_MAX_RESPONSE_RECORDS;
    list[i] = *item;
}

internal S2C_InputResponse* CLI_FindLatestInputResponse(
    S2C_InputResponse* list, i32 localTick)
{
    i32 latest = 0;
    S2C_InputResponse* result = NULL;
    for (i32 i = 0; i < CLI_MAX_RESPONSE_RECORDS; ++i)
    {
        S2C_InputResponse* item = &list[i];
        if (item->header.tick == 0) { continue; }

        if (item->header.tick < localTick
            && item->header.tick > latest)
        {
            latest = item->header.tick;
            result = item;
        }
    }
    return result;
}

/////////////////////////////////////
// Recording sent client inputs
/////////////////////////////////////

internal void CL_StoreSentInputCommand(
    C2S_Input* list, C2S_Input* input)
{
    APP_LOG(256,
        "CL Store sent input for tick %d - pos %.3f, %.3f, %.3f\n",
        input->header.tick,
        input->avatarPos.x, input->avatarPos.y, input->avatarPos.z);
    i32 i =  input->userInputSequence % CL_MAX_SENT_INPUT_COMMANDS;
    list[i] = *input;
}

internal C2S_Input* CL_RecallSentInputCommand(
    C2S_Input* list, i32 sequence)
{
    i32 i = sequence % CL_MAX_SENT_INPUT_COMMANDS;
    C2S_Input* result = &list[i];
    if (result->header.sentinel == 0
        || result->userInputSequence != sequence)
    {
        return NULL;
    }
    return result;
}

internal void CL_DumpSentInputs(
    C2S_Input* list, i32 numInputs)
{
	APP_LOG(64, "CL Stored Inputs\n");
    for (i32 i = 0; i < numInputs; ++i)
	{
		C2S_Input* input = &list[i];
		APP_LOG(256,
			"\tSeq %d, Local SV Tick %d - pos %.3f, %.3f, %.3f\n",
			input->userInputSequence, input->header.tick,
			input->avatarPos.x, input->avatarPos.y, input->avatarPos.z
		);
	}
}


internal C2S_Input* CL_RecallSentInputCommandByServerTick(
    C2S_Input* list, i32 serverTick)
{
    for (i32 i = 0; i < CL_MAX_SENT_INPUT_COMMANDS; ++i)
    {
        C2S_Input* result = &list[i];
        if (result->header.tick == serverTick)
        {
            return result;
        }
    }
    return NULL;
}

internal C2S_Input* CL_FindSentInputByPosition(
    C2S_Input* list, Vec3 queryPos, f32 epsilon)
{
    for (i32 i = 0; i < CL_MAX_SENT_INPUT_COMMANDS; ++i)
    {
        Vec3 pos = list[i].avatarPos;
        if (Vec3_AreDifferent(&queryPos, &pos, epsilon) == NO)
        {
            return &list[i];
        }
    }
    return NULL;
}

internal void CL_InitInputs(InputActionSet* actions)
{
    Input_InitAction(actions, Z_INPUT_CODE_V, "Cycle Debug");
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

    // Robotron style shooting
    Input_InitAction(actions, Z_INPUT_CODE_LEFT, "Shoot Left");
    Input_InitAction(actions, Z_INPUT_CODE_RIGHT, "Shoot Right");
    Input_InitAction(actions, Z_INPUT_CODE_UP, "Shoot Up");
    Input_InitAction(actions, Z_INPUT_CODE_DOWN, "Shoot Down");
}

internal void CL_InputCheckButton(
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

internal void CL_UpdateActorInput(InputActionSet* actions, SimActorInput* input)
{
    // Clear buttons and rebuild. Keep mouse position values
    u32 flags = 0;

    f32 mouseMoveMultiplier = 50;
    f32 mouseInvertedMultiplier = -1;

    CL_InputCheckButton(actions, "Move Forward", &flags, ACTOR_INPUT_MOVE_FORWARD);
    CL_InputCheckButton(actions, "Move Backward", &flags, ACTOR_INPUT_MOVE_BACKWARD);
    CL_InputCheckButton(actions, "Move Left", &flags, ACTOR_INPUT_MOVE_LEFT);
    CL_InputCheckButton(actions, "Move Right", &flags, ACTOR_INPUT_MOVE_RIGHT);

    CL_InputCheckButton(actions, "Shoot Up", &flags, ACTOR_INPUT_SHOOT_UP);
    CL_InputCheckButton(actions, "Shoot Down", &flags, ACTOR_INPUT_SHOOT_DOWN);
    CL_InputCheckButton(actions, "Shoot Left", &flags, ACTOR_INPUT_SHOOT_LEFT);
    CL_InputCheckButton(actions, "Shoot Right", &flags, ACTOR_INPUT_SHOOT_RIGHT);

    f32 val;
    f32 mouseX = ((f32)Input_GetActionValue(actions, "Mouse Move X") / (f32)Z_INPUT_MOUSE_SCALAR);
    f32 mouseY = ((f32)Input_GetActionValue(actions, "Mouse Move Y") / (f32)Z_INPUT_MOUSE_SCALAR);
    //printf("CL Mouse Pos %.3f, %.3f\n", mouseX, mouseY);
    val = mouseX * mouseMoveMultiplier;
    input->degrees.y -= val;
    input->degrees.y = COM_CapAngleDegrees(input->degrees.y);

    // original:
    //input->degrees.y -= (((f32)Input_GetActionValue(actions, "Mouse Move X") * mouseMoveMultiplier));
    //input->degrees.y = COM_CapAngleDegrees(input->degrees.y);

    input->degrees.x -= ((mouseY
		* mouseMoveMultiplier))
        * mouseInvertedMultiplier;
    
	if (input->degrees.x < -89)
	{
		input->degrees.x = -89;
	}
	if (input->degrees.x > 89)
	{
		input->degrees.x = 89;
	}
    g_testCameraDegrees = input->degrees;

    input->buttons = flags;
    //printf("CL Mouse angles %.3f, %.3f\n", input->degrees.y, input->degrees.y);

    //printf("Mouse pos %d, %d\n",
    //    Input_GetActionValue(actions, "Mouse Pos X"),
    //    Input_GetActionValue(actions, "Mouse Pos Y")
    //);
}
