#include "../../internal_headers/zengine_internal.h"

////////////////////////////////////////////////////////////////////
// Data structures
////////////////////////////////////////////////////////////////////
struct InputAction
{
    u32 keyCode1;
    u32 keyCode2;
    i32 value;
    // value in a range between 0 and 1 (eg resolution independent mouse movement)
    f32 normalised;
    frameInt lastFrame;
    char label[16];
};

struct InputActionSet
{
    InputAction *actions;
    i32 count;
};

internal void Input_InitAction(
    InputActionSet *actions, u32 keyCode1, u32 keyCode2, char *label)
{
    i32 index = actions->count++;
    actions->actions[index].keyCode1 = keyCode1;
    actions->actions[index].keyCode2 = keyCode2;
    actions->actions[index].value = 0;
    actions->actions[index].lastFrame = 0;
    i32 len = strlen(label);
    if (len > 16)
    {
        printf("Cannot add action %s - label must be below 16 chars\n",
               label);
        return;
    }
    ZStr_CopyLimited(label, actions->actions[index].label, 16);
}

internal void Input_ClearValues(InputActionSet *actions)
{
    for (i32 i = 0; i < actions->count; ++i)
    {
        actions->actions[i].value = 0;
        actions->actions[i].normalised = 0;
    }
}

// Find an action... duh
internal InputAction *Input_FindAction(InputAction *actions, i32 numActions, char *name)
{
    for (i32 i = 0; i < numActions; ++i)
    {
        InputAction *action = &actions[i];
        if (!ZStr_Compare(action->label, name))
        {
            return action;
        }
    }
    printf("Failed to find action %s\n", name);
    return NULL;
}

internal i32 Input_GetActionValue(InputAction *actions, i32 numActions, char *actionName)
{
    InputAction *action = Input_FindAction(actions, numActions, actionName);
    ZE_ASSERT(action != NULL, actionName);
    return action->value;
}

internal i32 Input_GetActionValue(InputActionSet *actions, char *actionName)
{
    return Input_GetActionValue(actions->actions, actions->count, actionName);
}

internal f32 Input_GetActionValueNormalised(InputActionSet *actions, char *actionName)
{
    InputAction *action = Input_FindAction(actions->actions, actions->count, actionName);
    ZE_ASSERT(action != NULL, actionName);
    return action->normalised;
}

internal u8 Input_CheckActionToggledOn(InputActionSet *actions, char *actionName, frameInt frameNumber)
{
    InputAction *action = Input_FindAction(actions->actions, actions->count, actionName);
    ZE_ASSERT(action != NULL, actionName);
    //printf("Action frame %lld vs platform frame %lld\n",
    //    action->lastFrame, frameNumber);
    return (action->value != 0 && action->lastFrame == frameNumber);
}

internal u8 Input_CheckActionToggledOff(InputActionSet *actions, char *actionName, frameInt frameNumber)
{
    InputAction *action = Input_FindAction(actions->actions, actions->count, actionName);
    ZE_ASSERT(action != NULL, actionName);

    return (action->value == 0 && action->lastFrame == frameNumber);
}

// Test an input event vs actions array. Return an input if it has changed, NULL if nothing changed
internal InputAction *Input_TestForAction(
    InputActionSet *actions, i32 inputValue, f32 normalised, u32 inputKeyCode, frameInt frameNumber)
{
    for (i32 i = 0; i < actions->count; ++i)
    {
        InputAction *action = &actions->actions[i];
        if (
            (action->keyCode1 == inputKeyCode || action->keyCode2 == inputKeyCode) && action->value != inputValue)
        {
            action->value = inputValue;
            action->lastFrame = frameNumber;
            action->normalised = normalised;
            return action;
        }
    }
    return NULL;
}

internal void ZInput_AddAction(u32 keyCode1, u32 keyCode2, char *label)
{
    printf("Add action %s\n", label);
}

ze_external zErrorCode ZInput_Init(ZInput *inputExport)
{
    inputExport->AddAction = ZInput_AddAction;
    return ZE_ERROR_NONE;
}
