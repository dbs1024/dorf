// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Input/InputManager.h"

#include "Core.Base/Assert.h"
#include <cstring>
#include <windows.h>

struct ButtonState
{
	unsigned char isPressed   : 1;
	unsigned char wasPressed  : 1;
	unsigned char wasReleased : 1;
};

struct InputManager
{
	void*       window;
	int         mouseX;
	int         mouseY;
	ButtonState keys[static_cast<unsigned>(KeyboardButton::KB_Count)];
	ButtonState mouseButtons[static_cast<unsigned>(MouseButton::MB_Count)];
	alignas(RAWINPUT) BYTE rawInputBuffer[128 * sizeof(RAWINPUT)];
};

struct MouseButtonMapping
{
	USHORT      downFlag;
	USHORT      upFlag;
	MouseButton button;
};

static const MouseButtonMapping s_mouseButtonMappings[] =
{
	{ RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_1_UP, MouseButton::MB_Left   },
	{ RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_2_UP, MouseButton::MB_Right  },
	{ RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_3_UP, MouseButton::MB_Middle },
	{ RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP, MouseButton::MB_X1     },
	{ RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP, MouseButton::MB_X2     },
};

// Helpers precede callers — no forward declarations.
static KeyboardButton mapVKeyboardButton(USHORT vkey, bool isE0)
{
	switch (vkey)
	{
	case '0': return KeyboardButton::KB_0;
	case '1': return KeyboardButton::KB_1;
	case '2': return KeyboardButton::KB_2;
	case '3': return KeyboardButton::KB_3;
	case '4': return KeyboardButton::KB_4;
	case '5': return KeyboardButton::KB_5;
	case '6': return KeyboardButton::KB_6;
	case '7': return KeyboardButton::KB_7;
	case '8': return KeyboardButton::KB_8;
	case '9': return KeyboardButton::KB_9;
	case 'A': return KeyboardButton::KB_A;
	case 'B': return KeyboardButton::KB_B;
	case 'C': return KeyboardButton::KB_C;
	case 'D': return KeyboardButton::KB_D;
	case 'E': return KeyboardButton::KB_E;
	case 'F': return KeyboardButton::KB_F;
	case 'G': return KeyboardButton::KB_G;
	case 'H': return KeyboardButton::KB_H;
	case 'I': return KeyboardButton::KB_I;
	case 'J': return KeyboardButton::KB_J;
	case 'K': return KeyboardButton::KB_K;
	case 'L': return KeyboardButton::KB_L;
	case 'M': return KeyboardButton::KB_M;
	case 'N': return KeyboardButton::KB_N;
	case 'O': return KeyboardButton::KB_O;
	case 'P': return KeyboardButton::KB_P;
	case 'Q': return KeyboardButton::KB_Q;
	case 'R': return KeyboardButton::KB_R;
	case 'S': return KeyboardButton::KB_S;
	case 'T': return KeyboardButton::KB_T;
	case 'U': return KeyboardButton::KB_U;
	case 'V': return KeyboardButton::KB_V;
	case 'W': return KeyboardButton::KB_W;
	case 'X': return KeyboardButton::KB_X;
	case 'Y': return KeyboardButton::KB_Y;
	case 'Z': return KeyboardButton::KB_Z;
	case VK_F1:  return KeyboardButton::KB_F1;
	case VK_F2:  return KeyboardButton::KB_F2;
	case VK_F3:  return KeyboardButton::KB_F3;
	case VK_F4:  return KeyboardButton::KB_F4;
	case VK_F5:  return KeyboardButton::KB_F5;
	case VK_F6:  return KeyboardButton::KB_F6;
	case VK_F7:  return KeyboardButton::KB_F7;
	case VK_F8:  return KeyboardButton::KB_F8;
	case VK_F9:  return KeyboardButton::KB_F9;
	case VK_F10: return KeyboardButton::KB_F10;
	case VK_F11: return KeyboardButton::KB_F11;
	case VK_F12: return KeyboardButton::KB_F12;
	case VK_UP:     return KeyboardButton::KB_Up;
	case VK_DOWN:   return KeyboardButton::KB_Down;
	case VK_LEFT:   return KeyboardButton::KB_Left;
	case VK_RIGHT:  return KeyboardButton::KB_Right;
	case VK_HOME:   return KeyboardButton::KB_Home;
	case VK_END:    return KeyboardButton::KB_End;
	case VK_PRIOR:  return KeyboardButton::KB_PageUp;
	case VK_NEXT:   return KeyboardButton::KB_PageDown;
	case VK_INSERT: return KeyboardButton::KB_Insert;
	case VK_DELETE: return KeyboardButton::KB_Delete;
	case VK_SPACE:  return KeyboardButton::KB_Space;
	case VK_RETURN: return isE0 ? KeyboardButton::KB_NumpadEnter : KeyboardButton::KB_Enter;
	case VK_BACK:   return KeyboardButton::KB_Backspace;
	case VK_TAB:    return KeyboardButton::KB_Tab;
	case VK_LSHIFT:   return KeyboardButton::KB_LShift;
	case VK_RSHIFT:   return KeyboardButton::KB_RShift;
	case VK_LCONTROL: return KeyboardButton::KB_LCtrl;
	case VK_RCONTROL: return KeyboardButton::KB_RCtrl;
	case VK_LMENU:    return KeyboardButton::KB_LAlt;
	case VK_RMENU:    return KeyboardButton::KB_RAlt;
	case VK_LWIN:     return KeyboardButton::KB_LWin;
	case VK_RWIN:     return KeyboardButton::KB_RWin;
	case VK_CAPITAL:  return KeyboardButton::KB_CapsLock;
	case VK_NUMLOCK:  return KeyboardButton::KB_NumLock;
	case VK_SCROLL:   return KeyboardButton::KB_ScrollLock;
	case VK_ESCAPE:   return KeyboardButton::KB_Escape;
	case VK_SNAPSHOT: return KeyboardButton::KB_PrintScreen;
	case VK_PAUSE:    return KeyboardButton::KB_Pause;
	case VK_APPS:     return KeyboardButton::KB_Menu;
	case VK_NUMPAD0:  return KeyboardButton::KB_Numpad0;
	case VK_NUMPAD1:  return KeyboardButton::KB_Numpad1;
	case VK_NUMPAD2:  return KeyboardButton::KB_Numpad2;
	case VK_NUMPAD3:  return KeyboardButton::KB_Numpad3;
	case VK_NUMPAD4:  return KeyboardButton::KB_Numpad4;
	case VK_NUMPAD5:  return KeyboardButton::KB_Numpad5;
	case VK_NUMPAD6:  return KeyboardButton::KB_Numpad6;
	case VK_NUMPAD7:  return KeyboardButton::KB_Numpad7;
	case VK_NUMPAD8:  return KeyboardButton::KB_Numpad8;
	case VK_NUMPAD9:  return KeyboardButton::KB_Numpad9;
	case VK_ADD:      return KeyboardButton::KB_NumpadAdd;
	case VK_SUBTRACT: return KeyboardButton::KB_NumpadSubtract;
	case VK_MULTIPLY: return KeyboardButton::KB_NumpadMultiply;
	case VK_DIVIDE:   return KeyboardButton::KB_NumpadDivide;
	case VK_DECIMAL:  return KeyboardButton::KB_NumpadDecimal;
	case VK_OEM_COMMA:  return KeyboardButton::KB_Comma;
	case VK_OEM_PERIOD: return KeyboardButton::KB_Period;
	case VK_OEM_2:      return KeyboardButton::KB_Slash;
	case VK_OEM_1:      return KeyboardButton::KB_Semicolon;
	case VK_OEM_7:      return KeyboardButton::KB_Apostrophe;
	case VK_OEM_4:      return KeyboardButton::KB_LBracket;
	case VK_OEM_6:      return KeyboardButton::KB_RBracket;
	case VK_OEM_5:      return KeyboardButton::KB_Backslash;
	case VK_OEM_MINUS:  return KeyboardButton::KB_Minus;
	case VK_OEM_PLUS:   return KeyboardButton::KB_Equals;
	case VK_OEM_3:      return KeyboardButton::KB_Grave;
	default:            return KeyboardButton::KB_Count;
	}
}

static void processRawInput(InputManager* inputManager, const RAWINPUT* raw)
{
	if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		const RAWKEYBOARD& kb = raw->data.keyboard;
		KeyboardButton key = mapVKeyboardButton(kb.VKey, (kb.Flags & RI_KEY_E0) != 0);
		if (key == KeyboardButton::KB_Count)
			return;

		bool isDown = !(kb.Flags & RI_KEY_BREAK);
		ButtonState& state = inputManager->keys[static_cast<unsigned>(key)];

		if (isDown && !state.isPressed)
			state.wasPressed = 1;
		else if (!isDown && state.isPressed)
			state.wasReleased = 1;
		state.isPressed = isDown ? 1 : 0;
	}
	else if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		USHORT flags = raw->data.mouse.usButtonFlags;
		for (unsigned i = 0; i < static_cast<unsigned>(MouseButton::MB_Count); ++i)
		{
			ButtonState& state = inputManager->mouseButtons[static_cast<unsigned>(s_mouseButtonMappings[i].button)];
			if (flags & s_mouseButtonMappings[i].downFlag)
			{
				state.wasPressed = 1;
				state.isPressed  = 1;
			}
			else if (flags & s_mouseButtonMappings[i].upFlag)
			{
				state.wasReleased = 1;
				state.isPressed   = 0;
			}
		}
	}
}

InputManagerError createInputManager(InputManager** outInputManager, const InputManagerCreateParams& params)
{
	if (!outInputManager || !params.window)
		return InputManagerError::InvalidArg;

	InputManager* inputManager = new InputManager;
	memset(inputManager, 0, sizeof(InputManager));
	inputManager->window = params.window;

	RAWINPUTDEVICE devices[2];

	devices[0].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
	devices[0].usUsage     = 0x06; // HID_USAGE_GENERIC_KEYBOARD
	devices[0].dwFlags     = 0;
	devices[0].hwndTarget  = static_cast<HWND>(params.window);

	devices[1].usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
	devices[1].usUsage     = 0x02; // HID_USAGE_GENERIC_MOUSE
	devices[1].dwFlags     = 0;
	devices[1].hwndTarget  = static_cast<HWND>(params.window);

	if (!RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE)))
	{
		delete inputManager;
		return InputManagerError::Failed;
	}

	*outInputManager = inputManager;
	return InputManagerError::Ok;
}

void destroyInputManager(InputManager* inputManager)
{
	ACE_ASSERT(inputManager);
	if (!inputManager)
		return;

	delete inputManager;
}

void updateInputManager(InputManager* inputManager)
{
	ACE_ASSERT(inputManager);
	if (!inputManager)
		return;

	for (unsigned i = 0; i < static_cast<unsigned>(KeyboardButton::KB_Count); ++i)
	{
		inputManager->keys[i].wasPressed  = 0;
		inputManager->keys[i].wasReleased = 0;
	}
	for (unsigned i = 0; i < static_cast<unsigned>(MouseButton::MB_Count); ++i)
	{
		inputManager->mouseButtons[i].wasPressed  = 0;
		inputManager->mouseButtons[i].wasReleased = 0;
	}

	for (;;)
	{
		UINT bufferSize = sizeof(inputManager->rawInputBuffer);
		UINT count = GetRawInputBuffer(reinterpret_cast<RAWINPUT*>(inputManager->rawInputBuffer), &bufferSize, sizeof(RAWINPUTHEADER));
		if (count == UINT(-1) || count == 0)
			break;

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(inputManager->rawInputBuffer);
		for (UINT i = 0; i < count; ++i)
		{
			processRawInput(inputManager, raw);
			raw = reinterpret_cast<RAWINPUT*>((reinterpret_cast<ULONG_PTR>(raw) + raw->header.dwSize + 7) & ~static_cast<ULONG_PTR>(7));
		}
	}

	POINT pt;
	GetCursorPos(&pt);
	inputManager->mouseX = pt.x;
	inputManager->mouseY = pt.y;
}

bool isKeyboardButtonPressed(InputManager* inputManager, KeyboardButton key)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(key < KeyboardButton::KB_Count);
	return inputManager->keys[static_cast<unsigned>(key)].isPressed;
}

bool wasKeyboardButtonPressed(InputManager* inputManager, KeyboardButton key)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(key < KeyboardButton::KB_Count);
	return inputManager->keys[static_cast<unsigned>(key)].wasPressed;
}

bool wasKeyboardButtonReleased(InputManager* inputManager, KeyboardButton key)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(key < KeyboardButton::KB_Count);
	return inputManager->keys[static_cast<unsigned>(key)].wasReleased;
}

bool isMouseButtonPressed(InputManager* inputManager, MouseButton button)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(button < MouseButton::MB_Count);
	return inputManager->mouseButtons[static_cast<unsigned>(button)].isPressed;
}

bool wasMouseButtonPressed(InputManager* inputManager, MouseButton button)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(button < MouseButton::MB_Count);
	return inputManager->mouseButtons[static_cast<unsigned>(button)].wasPressed;
}

bool wasMouseButtonReleased(InputManager* inputManager, MouseButton button)
{
	ACE_ASSERT(inputManager);
	ACE_ASSERT(button < MouseButton::MB_Count);
	return inputManager->mouseButtons[static_cast<unsigned>(button)].wasReleased;
}
