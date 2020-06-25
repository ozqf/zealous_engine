#ifndef WIN_MAP_GLFW_INPUT_H
#define WIN_MAP_GLFW_INPUT_H

#include "../../lib/glfw3_vc2015/glfw3.h"
#include "../ze_common/ze_input.h"

static zeInputCode Win_GlfwToZEKey(i32 glfwKeyCode)
{
    switch (glfwKeyCode)
    {
        //case GLFW_KEY_: return Z_INPUT_CODE_NULL;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_1;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_2;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_3;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_4;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_5;
        //case GLFW_KEY_: return Z_INPUT_CODE_MWHEELUP;
        //case GLFW_KEY_: return Z_INPUT_CODE_MWHEELDOWN;
        case GLFW_KEY_A: return Z_INPUT_CODE_A;
        case GLFW_KEY_B: return Z_INPUT_CODE_B;
        case GLFW_KEY_C: return Z_INPUT_CODE_C;
        case GLFW_KEY_D: return Z_INPUT_CODE_D;
        case GLFW_KEY_E: return Z_INPUT_CODE_E;
        case GLFW_KEY_F: return Z_INPUT_CODE_F;
        case GLFW_KEY_G: return Z_INPUT_CODE_G;
        case GLFW_KEY_H: return Z_INPUT_CODE_H;
        case GLFW_KEY_I: return Z_INPUT_CODE_I;
        case GLFW_KEY_J: return Z_INPUT_CODE_J;
        case GLFW_KEY_K: return Z_INPUT_CODE_K;
        case GLFW_KEY_L: return Z_INPUT_CODE_L;
        case GLFW_KEY_M: return Z_INPUT_CODE_M;
        case GLFW_KEY_N: return Z_INPUT_CODE_N;
        case GLFW_KEY_O: return Z_INPUT_CODE_O;
        case GLFW_KEY_P: return Z_INPUT_CODE_P;
        case GLFW_KEY_Q: return Z_INPUT_CODE_Q;
        case GLFW_KEY_R: return Z_INPUT_CODE_R;
        case GLFW_KEY_S: return Z_INPUT_CODE_S;
        case GLFW_KEY_T: return Z_INPUT_CODE_T;
        case GLFW_KEY_U: return Z_INPUT_CODE_U;
        case GLFW_KEY_V: return Z_INPUT_CODE_V;
        case GLFW_KEY_W: return Z_INPUT_CODE_W;
        case GLFW_KEY_X: return Z_INPUT_CODE_X;
        case GLFW_KEY_Y: return Z_INPUT_CODE_Y;
        case GLFW_KEY_Z: return Z_INPUT_CODE_Z;
        case GLFW_KEY_SPACE: return Z_INPUT_CODE_SPACE;
        case GLFW_KEY_LEFT_SHIFT: return Z_INPUT_CODE_LEFT_SHIFT;
        case GLFW_KEY_RIGHT_SHIFT: return Z_INPUT_CODE_RIGHT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return Z_INPUT_CODE_LEFT_CONTROL;
        case GLFW_KEY_RIGHT_CONTROL: return Z_INPUT_CODE_RIGHT_CONTROL;
        case GLFW_KEY_ESCAPE: return Z_INPUT_CODE_ESCAPE;
        case GLFW_KEY_ENTER: return Z_INPUT_CODE_RETURN;
        case GLFW_KEY_KP_ENTER: return Z_INPUT_CODE_ENTER;
        case GLFW_KEY_0: return Z_INPUT_CODE_0;
        case GLFW_KEY_1: return Z_INPUT_CODE_1;
        case GLFW_KEY_2: return Z_INPUT_CODE_2;
        case GLFW_KEY_3: return Z_INPUT_CODE_3;
        case GLFW_KEY_4: return Z_INPUT_CODE_4;
        case GLFW_KEY_5: return Z_INPUT_CODE_5;
        case GLFW_KEY_6: return Z_INPUT_CODE_6;
        case GLFW_KEY_7: return Z_INPUT_CODE_7;
        case GLFW_KEY_8: return Z_INPUT_CODE_8;
        case GLFW_KEY_9: return Z_INPUT_CODE_9;
        case GLFW_KEY_UP: return Z_INPUT_CODE_UP;
        case GLFW_KEY_DOWN: return Z_INPUT_CODE_DOWN;
        case GLFW_KEY_LEFT: return Z_INPUT_CODE_LEFT;
        case GLFW_KEY_RIGHT: return Z_INPUT_CODE_RIGHT;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_POS_X;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_POS_Y;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_MOVE_X;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_MOVE_Y;
        case GLFW_KEY_F1: return Z_INPUT_CODE_F1;
        case GLFW_KEY_F2: return Z_INPUT_CODE_F2;
        case GLFW_KEY_F3: return Z_INPUT_CODE_F3;
        case GLFW_KEY_F4: return Z_INPUT_CODE_F4;
        case GLFW_KEY_F5: return Z_INPUT_CODE_F5;
        case GLFW_KEY_F6: return Z_INPUT_CODE_F6;
        case GLFW_KEY_F7: return Z_INPUT_CODE_F7;
        case GLFW_KEY_F8: return Z_INPUT_CODE_F8;
        case GLFW_KEY_F9: return Z_INPUT_CODE_F9;
        case GLFW_KEY_F10: return Z_INPUT_CODE_F10;
        case GLFW_KEY_F11: return Z_INPUT_CODE_F11;
        case GLFW_KEY_F12: return Z_INPUT_CODE_F12;
		case 162: return Z_INPUT_CODE_BACKSLASH;
		case GLFW_KEY_BACKSLASH: return Z_INPUT_CODE_BACKSLASH;
        default:
        printf("Found no match for GLFW Key %d\n", glfwKeyCode);
        return Z_INPUT_CODE_NULL;
    }
}

#endif // WIN_MAP_GLFW_INPUT_H