#include "../../internal_headers/zengine_internal.h"

////////////////////////////////////////////////////////////////////
// Data structures
////////////////////////////////////////////////////////////////////
struct InputAction
{
    i32 bytes = 0;
    u32 keyCode1;
    u32 keyCode2;
    i32 value;
    // value in a range between 0 and 1 (eg resolution independent mouse movement)
    f32 normalised;
    frameInt lastFrame;
    char* label;
};

ze_internal ZEBuffer g_actions;
ze_internal ZEHashTable* g_actionsByName;

internal void DebugListActions()
{
    printf("-- Input Actions --\n");
    i8 *read = g_actions.start;
    i8 *end = g_actions.cursor;
    while (read < end)
    {
        ZE_CAST_PTR(read, InputAction, action)
        read += action->bytes;
        printf("Action \"%s\", codes %d and %d\n",
               action->label, action->keyCode1, action->keyCode2);
    }
}

internal i32 ZInput_GetActionValue(char* name)
{
    i32 id = ZE_Hash_djb2((u8*)name);
    void* ptr = g_actionsByName->FindPointer(id);
    if (ptr == NULL)
    {
        return 0;
    }
    return ((InputAction*)ptr)->value;
}

// struct InputActionSet
// {
//     InputAction *actions;
//     i32 count;
// };

// internal InputActionSet* g_set;
#if 0
internal void Input_InitAction(
    ZEHashTable *actions, u32 keyCode1, u32 keyCode2, char *label)
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
#endif
internal void ZInput_AddAction(u32 keyCode1, u32 keyCode2, char *label)
{
    printf("Add action %s\n", label);
    // measure required space
    i32 labelLen = ZStr_Len(label);
    i32 requiredSpace = sizeof(InputAction) + labelLen;
    if (g_actions.Space() < requiredSpace)
    {
        printf("No space for new action %s\n", label);
        return;
    }
    // create a new action and buffer it + its name
    ZE_BUF_INIT_PTR_IN_PLACE(action, InputAction, (&g_actions))
    // record size so we can iterate actions in buffer
    action->bytes = requiredSpace;
    // set the name in the struct
    action->label = (char*)g_actions.cursor;
    // write the action's name into the buffer after it
    g_actions.cursor += ZE_COPY(label, g_actions.cursor, labelLen);
    // add to the hash table
    g_actionsByName->InsertPointer(ZE_Hash_djb2((u8*)label), action);
    action->keyCode1 = keyCode1;
    action->keyCode2 = keyCode2;

    // DebugListActions();
}

ze_external void ZInput_ReadEvent(SysInputEvent* ev)
{
    // printf("ZInput saw %d set to %d\n", ev->inputID, ev->value);
    // find a control that matches this keycode
    i8* read = g_actions.start;
    i8* end = g_actions.cursor;
    while (read < end)
    {
        ZE_CAST_PTR(read, InputAction, action)
        read += action->bytes;
        if (action->keyCode1 == ev->inputID || action->keyCode2 == ev->inputID)
        {
            action->value = ev->value;
            action->normalised = ev->normalised;
        }
    }
}

ze_external ZInput ZInput_RegisterFunctions()
{
    // allocate
    g_actions = Buf_FromMalloc(Platform_Alloc, KiloBytes(64));
    g_actionsByName = ZE_HashTable_Create(Platform_Alloc, 128, NULL);

    // setup game interface
    ZInput inputExport;
    inputExport.AddAction = ZInput_AddAction;
    inputExport.GetActionValue = ZInput_GetActionValue;
    return inputExport;
}
