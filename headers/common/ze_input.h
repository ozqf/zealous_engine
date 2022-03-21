#ifndef ZE_INPUT_H
#define ZE_INPUT_H

#include "../ze_common.h"

enum ZMouseMode
{
    Free = 0,
    Captured = 1
};

typedef u32 zeInputCode;

#define Z_INPUT_MOUSE_SCALAR 100000.f

//////////////////////////////////////////////////////////////////////
// Input codes
//////////////////////////////////////////////////////////////////////
#define Z_INPUT_CODE_NULL 0
#define Z_INPUT_CODE_MOUSE_1 1
#define Z_INPUT_CODE_MOUSE_2 2
#define Z_INPUT_CODE_MOUSE_3 3
#define Z_INPUT_CODE_MOUSE_4 4
#define Z_INPUT_CODE_MOUSE_5 5
#define Z_INPUT_CODE_MWHEELUP 6
#define Z_INPUT_CODE_MWHEELDOWN 7
#define Z_INPUT_CODE_A 8
#define Z_INPUT_CODE_B 9
#define Z_INPUT_CODE_C 10
#define Z_INPUT_CODE_D 11
#define Z_INPUT_CODE_E 12
#define Z_INPUT_CODE_F 13
#define Z_INPUT_CODE_G 14
#define Z_INPUT_CODE_H 15
#define Z_INPUT_CODE_I 16
#define Z_INPUT_CODE_J 17
#define Z_INPUT_CODE_K 18
#define Z_INPUT_CODE_L 19
#define Z_INPUT_CODE_M 20
#define Z_INPUT_CODE_N 21
#define Z_INPUT_CODE_O 22
#define Z_INPUT_CODE_P 23
#define Z_INPUT_CODE_Q 24
#define Z_INPUT_CODE_R 25
#define Z_INPUT_CODE_S 26
#define Z_INPUT_CODE_T 27
#define Z_INPUT_CODE_U 28
#define Z_INPUT_CODE_V 29
#define Z_INPUT_CODE_W 30
#define Z_INPUT_CODE_X 31
#define Z_INPUT_CODE_Y 32
#define Z_INPUT_CODE_Z 33
#define Z_INPUT_CODE_SPACE 34
#define Z_INPUT_CODE_LEFT_SHIFT 35
#define Z_INPUT_CODE_RIGHT_SHIFT 36
#define Z_INPUT_CODE_LEFT_CONTROL 37
#define Z_INPUT_CODE_RIGHT_CONTROL 38
#define Z_INPUT_CODE_ESCAPE 39
#define Z_INPUT_CODE_RETURN 40
#define Z_INPUT_CODE_ENTER 41
#define Z_INPUT_CODE_0 42
#define Z_INPUT_CODE_1 43
#define Z_INPUT_CODE_2 44
#define Z_INPUT_CODE_3 45
#define Z_INPUT_CODE_4 46
#define Z_INPUT_CODE_5 47
#define Z_INPUT_CODE_6 48
#define Z_INPUT_CODE_7 49
#define Z_INPUT_CODE_8 50
#define Z_INPUT_CODE_9 51
#define Z_INPUT_CODE_UP 52
#define Z_INPUT_CODE_DOWN 53
#define Z_INPUT_CODE_LEFT 54
#define Z_INPUT_CODE_RIGHT 55
#define Z_INPUT_CODE_MOUSE_POS_X 56
#define Z_INPUT_CODE_MOUSE_POS_Y 57
#define Z_INPUT_CODE_MOUSE_MOVE_X 58
#define Z_INPUT_CODE_MOUSE_MOVE_Y 59
#define Z_INPUT_CODE_F1 60
#define Z_INPUT_CODE_F2 61
#define Z_INPUT_CODE_F3 62
#define Z_INPUT_CODE_F4 63
#define Z_INPUT_CODE_F5 64
#define Z_INPUT_CODE_F6 65
#define Z_INPUT_CODE_F7 66
#define Z_INPUT_CODE_F8 67
#define Z_INPUT_CODE_F9 68
#define Z_INPUT_CODE_F10 69
#define Z_INPUT_CODE_F11 70
#define Z_INPUT_CODE_F12 71
#define Z_INPUT_CODE_BACKSLASH 72
#define Z_INPUT_CODE_FORWARDSLASH 73
#define Z_INPUT_CODE_DELETE 74

#define Z_INPUT_CODE__COUNT__ 74

// struct InputItem
// {
//     char on = 0;
//     u32 lastChangeFrame = 0;
// };

/*
Convert a screen pixel position to opengl screen coords (-1 to +1)
*/
static Vec2 Input_NormaliseScreenPos(f32 scrPosX, f32 scrPosY, i32 scrWidth, i32 scrHeight)
{
    Vec2 p = {};
    p.x = scrPosX / scrWidth;
    p.x = (p.x * 2.f) - 1.f;

    p.y = scrPosY / scrHeight;
    p.y = (p.y * 2.f) - 1.f;
    p.y *= -1.f;
    return p;
}

#endif // ZE_INPUT_H
