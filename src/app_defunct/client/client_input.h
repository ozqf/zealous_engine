#pragma once

#include "client.h"

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
    Input_InitAction(actions, Z_INPUT_CODE_CONTROL, "Move Down");
    Input_InitAction(actions, Z_INPUT_CODE_SHIFT, "MoveSpecial1");
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

    f32 mouseMoveMultiplier = 1;
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

    val = (f32)Input_GetActionValue(actions, "Mouse Move X") * mouseMoveMultiplier;
    input->degrees.y -= val;
    input->degrees.y = COM_CapAngleDegrees(input->degrees.y);

    // original:
    //input->degrees.y -= (((f32)Input_GetActionValue(actions, "Mouse Move X") * mouseMoveMultiplier));
    //input->degrees.y = COM_CapAngleDegrees(input->degrees.y);

    input->degrees.x -= (((f32)Input_GetActionValue(actions, "Mouse Move Y")
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

    input->buttons = flags;


    //printf("Mouse pos %d, %d\n",
    //    Input_GetActionValue(actions, "Mouse Pos X"),
    //    Input_GetActionValue(actions, "Mouse Pos Y")
    //);
}
