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

struct ZInputIdentifier
{
	i32 code;
	char* shortLabel;
	char* longLabel;
};

ze_internal ZEBuffer g_actions;
ze_internal ZEHashTable* g_actionsByName;

ze_internal ZInputIdentifier g_inputIds[Z_INPUT_CODE__COUNT__];

ze_internal InputAction* g_rebindTarget = NULL;

internal void DebugListActions()
{
    printf("-- Input Actions --\n");
    i8 *read = g_actions.start;
    i8 *end = g_actions.cursor;
    while (read < end)
    {
        ZE_CAST_PTR(read, InputAction, action)
        read += action->bytes;
        char* inputA = g_inputIds[action->keyCode1].shortLabel;
        char* inputB = g_inputIds[action->keyCode2].shortLabel;
        printf("Action \"%s\"\t key %s, code %d",
               action->label, inputA, action->keyCode1);
        if (action->keyCode2 != Z_INPUT_CODE_NULL)
        {
            printf("\talt key %s, code %d",
               inputB, action->keyCode2);
        }
        printf("\n");
    }
}

ZCMD_CALLBACK(Exec_ListActions)
{
    DebugListActions();
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


internal f32 Input_GetActionValueNormalised(char *name)
{
    i32 id = ZE_Hash_djb2((u8*)name);
    void* ptr = g_actionsByName->FindPointer(id);
    if (ptr == NULL)
    {
        return 0;
    }
    return ((InputAction*)ptr)->normalised;
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

internal f32 Input_GetActionValueNormalised(char *actionName)
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

internal const char* ZInput_GetInputLongLabel(i32 code)
{
	if (code < 0) { code = 0; }
	if (code >= Z_INPUT_CODE__COUNT__) { code = Z_INPUT_CODE__COUNT__ - 1; }
	return g_inputIds[code].longLabel != NULL ? g_inputIds[code].longLabel : g_inputIds[code].shortLabel;
}

internal const char* ZInput_GetInputShortLabel(i32 code)
{
	if (code < 0) { code = 0; }
	if (code >= Z_INPUT_CODE__COUNT__) { code = Z_INPUT_CODE__COUNT__ - 1; }
	return g_inputIds[code].shortLabel;
}

internal void ZInput_BuildInputsTable()
{
	g_inputIds[Z_INPUT_CODE_NULL] = { Z_INPUT_CODE_NULL, "Empty", "Empty" };
	g_inputIds[Z_INPUT_CODE_MOUSE_1] = { Z_INPUT_CODE_MOUSE_1, "M1", "Mouse Button 1" };
	g_inputIds[Z_INPUT_CODE_MOUSE_2] = { Z_INPUT_CODE_MOUSE_2, "M2", "Mouse Button 2" };
	g_inputIds[Z_INPUT_CODE_MOUSE_3] = { Z_INPUT_CODE_MOUSE_3, "M3", "Mouse Button 3" };
	g_inputIds[Z_INPUT_CODE_MOUSE_4] = { Z_INPUT_CODE_MOUSE_4, "M4", "Mouse Button 4" };
	g_inputIds[Z_INPUT_CODE_MOUSE_5] = { Z_INPUT_CODE_MOUSE_5, "M5", "Mouse Button 5" };
	g_inputIds[Z_INPUT_CODE_MWHEELUP] = { Z_INPUT_CODE_MWHEELUP, "MWUp", "Mouse Wheel Up" };
	g_inputIds[Z_INPUT_CODE_MWHEELDOWN] = { Z_INPUT_CODE_MWHEELDOWN, "MWDown", "Mouse Wheel Down" };
	g_inputIds[Z_INPUT_CODE_A] = { Z_INPUT_CODE_A, "A", NULL };
	g_inputIds[Z_INPUT_CODE_B] = { Z_INPUT_CODE_B, "B", NULL };
	g_inputIds[Z_INPUT_CODE_C] = { Z_INPUT_CODE_C, "C", NULL };
	g_inputIds[Z_INPUT_CODE_D] = { Z_INPUT_CODE_D, "D", NULL };
	g_inputIds[Z_INPUT_CODE_E] = { Z_INPUT_CODE_E, "E", NULL };
	g_inputIds[Z_INPUT_CODE_F] = { Z_INPUT_CODE_F, "F", NULL };
	g_inputIds[Z_INPUT_CODE_G] = { Z_INPUT_CODE_G, "G", NULL };
	g_inputIds[Z_INPUT_CODE_H] = { Z_INPUT_CODE_H, "H", NULL };
	g_inputIds[Z_INPUT_CODE_I] = { Z_INPUT_CODE_I, "I", NULL };
	g_inputIds[Z_INPUT_CODE_J] = { Z_INPUT_CODE_J, "J", NULL };
	g_inputIds[Z_INPUT_CODE_K] = { Z_INPUT_CODE_K, "K", NULL };
	g_inputIds[Z_INPUT_CODE_L] = { Z_INPUT_CODE_L, "L", NULL };
	g_inputIds[Z_INPUT_CODE_M] = { Z_INPUT_CODE_M, "M", NULL };
	g_inputIds[Z_INPUT_CODE_N] = { Z_INPUT_CODE_N, "N", NULL };
	g_inputIds[Z_INPUT_CODE_O] = { Z_INPUT_CODE_O, "O", NULL };
	g_inputIds[Z_INPUT_CODE_P] = { Z_INPUT_CODE_P, "P", NULL };
	g_inputIds[Z_INPUT_CODE_Q] = { Z_INPUT_CODE_Q, "Q", NULL };
	g_inputIds[Z_INPUT_CODE_R] = { Z_INPUT_CODE_R, "R", NULL };
	g_inputIds[Z_INPUT_CODE_S] = { Z_INPUT_CODE_S, "S", NULL };
	g_inputIds[Z_INPUT_CODE_T] = { Z_INPUT_CODE_T, "T", NULL };
	g_inputIds[Z_INPUT_CODE_U] = { Z_INPUT_CODE_U, "U", NULL };
	g_inputIds[Z_INPUT_CODE_V] = { Z_INPUT_CODE_V, "V", NULL };
	g_inputIds[Z_INPUT_CODE_W] = { Z_INPUT_CODE_W, "W", NULL };
	g_inputIds[Z_INPUT_CODE_X] = { Z_INPUT_CODE_X, "X", NULL };
	g_inputIds[Z_INPUT_CODE_Y] = { Z_INPUT_CODE_Y, "Y", NULL };
	g_inputIds[Z_INPUT_CODE_Z] = { Z_INPUT_CODE_Z, "Z", NULL };
	g_inputIds[Z_INPUT_CODE_SPACE] = { Z_INPUT_CODE_SPACE, "Space", "Space Bar" };
	g_inputIds[Z_INPUT_CODE_LEFT_SHIFT] = { Z_INPUT_CODE_LEFT_SHIFT, "LShift", "Left Shift" };
	g_inputIds[Z_INPUT_CODE_RIGHT_SHIFT] = { Z_INPUT_CODE_RIGHT_SHIFT, "RShift", "Right Shift" };
	g_inputIds[Z_INPUT_CODE_LEFT_CONTROL] = { Z_INPUT_CODE_LEFT_CONTROL, "LCtrl", "Left Control" };
	g_inputIds[Z_INPUT_CODE_RIGHT_CONTROL] = { Z_INPUT_CODE_RIGHT_CONTROL, "RCtrl", "Right Control" };
	g_inputIds[Z_INPUT_CODE_ESCAPE] = { Z_INPUT_CODE_ESCAPE, "Esc", "Escape" };
	g_inputIds[Z_INPUT_CODE_RETURN] = { Z_INPUT_CODE_RETURN, "Return", "Return" };
	g_inputIds[Z_INPUT_CODE_ENTER] = { Z_INPUT_CODE_ENTER, "Enter", "Enter" };
	g_inputIds[Z_INPUT_CODE_0] = { Z_INPUT_CODE_0, "0", NULL };
	g_inputIds[Z_INPUT_CODE_1] = { Z_INPUT_CODE_1, "1", NULL };
	g_inputIds[Z_INPUT_CODE_2] = { Z_INPUT_CODE_2, "2", NULL };
	g_inputIds[Z_INPUT_CODE_3] = { Z_INPUT_CODE_3, "3", NULL };
	g_inputIds[Z_INPUT_CODE_4] = { Z_INPUT_CODE_4, "4", NULL };
	g_inputIds[Z_INPUT_CODE_5] = { Z_INPUT_CODE_5, "5", NULL };
	g_inputIds[Z_INPUT_CODE_6] = { Z_INPUT_CODE_6, "6", NULL };
	g_inputIds[Z_INPUT_CODE_7] = { Z_INPUT_CODE_7, "7", NULL };
	g_inputIds[Z_INPUT_CODE_8] = { Z_INPUT_CODE_8, "8", NULL };
	g_inputIds[Z_INPUT_CODE_9] = { Z_INPUT_CODE_9, "9", NULL };
	g_inputIds[Z_INPUT_CODE_UP] = { Z_INPUT_CODE_UP, "Up", "Up Cursor" };
	g_inputIds[Z_INPUT_CODE_DOWN] = { Z_INPUT_CODE_DOWN, "Down", "Down Cursor" };
	g_inputIds[Z_INPUT_CODE_LEFT] = { Z_INPUT_CODE_LEFT, "Left", "Left Cursor" };
	g_inputIds[Z_INPUT_CODE_RIGHT] = { Z_INPUT_CODE_RIGHT, "Right", "Right Cursor" };
	g_inputIds[Z_INPUT_CODE_MOUSE_POS_X] = { Z_INPUT_CODE_MOUSE_POS_X, "MPosX", "Mouse Position X" };
	g_inputIds[Z_INPUT_CODE_MOUSE_POS_Y] = { Z_INPUT_CODE_MOUSE_POS_Y, "MPosY", "Mouse Position Y" };
	g_inputIds[Z_INPUT_CODE_MOUSE_MOVE_X] = { Z_INPUT_CODE_MOUSE_MOVE_X, "MMoveX", "Mouse Movement X" };
	g_inputIds[Z_INPUT_CODE_MOUSE_MOVE_Y] = { Z_INPUT_CODE_MOUSE_MOVE_Y, "MMoveY", "Mouse Movement Y" };
	g_inputIds[Z_INPUT_CODE_F1] = { Z_INPUT_CODE_F1, "F1", NULL };
	g_inputIds[Z_INPUT_CODE_F2] = { Z_INPUT_CODE_F2, "F2", NULL };
	g_inputIds[Z_INPUT_CODE_F3] = { Z_INPUT_CODE_F3, "F3", NULL };
	g_inputIds[Z_INPUT_CODE_F4] = { Z_INPUT_CODE_F4, "F4", NULL };
	g_inputIds[Z_INPUT_CODE_F5] = { Z_INPUT_CODE_F5, "F5", NULL };
	g_inputIds[Z_INPUT_CODE_F6] = { Z_INPUT_CODE_F6, "F6", NULL };
	g_inputIds[Z_INPUT_CODE_F7] = { Z_INPUT_CODE_F7, "F7", NULL };
	g_inputIds[Z_INPUT_CODE_F8] = { Z_INPUT_CODE_F8, "F8", NULL };
	g_inputIds[Z_INPUT_CODE_F9] = { Z_INPUT_CODE_F9, "F9", NULL };
	g_inputIds[Z_INPUT_CODE_F10] = { Z_INPUT_CODE_F10, "F10", NULL };
	g_inputIds[Z_INPUT_CODE_F11] = { Z_INPUT_CODE_F11, "F11", NULL };
	g_inputIds[Z_INPUT_CODE_F12] = { Z_INPUT_CODE_F12, "F12", NULL };
	g_inputIds[Z_INPUT_CODE_BACKSLASH] = { Z_INPUT_CODE_BACKSLASH, "\\", "Backslash" };
	g_inputIds[Z_INPUT_CODE_FORWARDSLASH] = { Z_INPUT_CODE_FORWARDSLASH, "/", "Forwardslash" };
}

ze_external ZInput ZInput_RegisterFunctions()
{
    // allocate
    g_actions = Buf_FromMalloc(Platform_Alloc, KiloBytes(64));
    g_actionsByName = ZE_HashTable_Create(Platform_Alloc, 128, NULL);
	
	ZInput_BuildInputsTable();

    // setup game interface
    ZInput inputExport;
    inputExport.AddAction = ZInput_AddAction;
    inputExport.GetActionValue = ZInput_GetActionValue;
	inputExport.GetActionValueNormalised = Input_GetActionValueNormalised;
	
	inputExport.GetInputShortLabel = ZInput_GetInputShortLabel;
	inputExport.GetInputLongLabel = ZInput_GetInputLongLabel;
    return inputExport;
}

ze_external zErrorCode ZInput_Init()
{
    ZCmdConsole_RegisterInternalCommand("listactions", "List all input actions and their bindings", Exec_ListActions);

    return ZE_ERROR_NONE;
}
