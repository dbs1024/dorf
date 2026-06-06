// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct InputManager;

struct InputManagerCreateParams
{
	void* window;
};

enum class InputManagerError : unsigned
{
	Ok,
	InvalidArg,
	Failed,
};

enum class KeyboardButton : unsigned
{
	// Alphanumeric
	KB_0,
	KB_1,
	KB_2,
	KB_3,
	KB_4,
	KB_5,
	KB_6,
	KB_7,
	KB_8,
	KB_9,
	KB_A,
	KB_B,
	KB_C,
	KB_D,
	KB_E,
	KB_F,
	KB_G,
	KB_H,
	KB_I,
	KB_J,
	KB_K,
	KB_L,
	KB_M,
	KB_N,
	KB_O,
	KB_P,
	KB_Q,
	KB_R,
	KB_S,
	KB_T,
	KB_U,
	KB_V,
	KB_W,
	KB_X,
	KB_Y,
	KB_Z,

	// Function keys
	KB_F1,
	KB_F2,
	KB_F3,
	KB_F4,
	KB_F5,
	KB_F6,
	KB_F7,
	KB_F8,
	KB_F9,
	KB_F10,
	KB_F11,
	KB_F12,

	// Navigation
	KB_Up,
	KB_Down,
	KB_Left,
	KB_Right,
	KB_Home,
	KB_End,
	KB_PageUp,
	KB_PageDown,
	KB_Insert,
	KB_Delete,

	// Whitespace / editing
	KB_Space,
	KB_Enter,
	KB_Backspace,
	KB_Tab,

	// Modifiers
	KB_LShift,
	KB_RShift,
	KB_LCtrl,
	KB_RCtrl,
	KB_LAlt,
	KB_RAlt,
	KB_LWin,
	KB_RWin,
	KB_CapsLock,
	KB_NumLock,
	KB_ScrollLock,

	// System
	KB_Escape,
	KB_PrintScreen,
	KB_Pause,
	KB_Menu,

	// Numpad
	KB_Numpad0,
	KB_Numpad1,
	KB_Numpad2,
	KB_Numpad3,
	KB_Numpad4,
	KB_Numpad5,
	KB_Numpad6,
	KB_Numpad7,
	KB_Numpad8,
	KB_Numpad9,
	KB_NumpadAdd,
	KB_NumpadSubtract,
	KB_NumpadMultiply,
	KB_NumpadDivide,
	KB_NumpadDecimal,
	KB_NumpadEnter,

	// Punctuation / symbols
	KB_Comma,
	KB_Period,
	KB_Slash,
	KB_Semicolon,
	KB_Apostrophe,
	KB_LBracket,
	KB_RBracket,
	KB_Backslash,
	KB_Minus,
	KB_Equals,
	KB_Grave,

	KB_Count,
};

enum class MouseButton : unsigned
{
	MB_Left,
	MB_Right,
	MB_Middle,
	MB_X1,
	MB_X2,

	MB_Count,
};

// ---------------------------------------------------------------------------

InputManagerError createInputManager(InputManager** outInputManager, const InputManagerCreateParams& params);
void              destroyInputManager(InputManager* inputManager);
void              updateInputManager(InputManager* inputManager);

bool isKeyboardButtonPressed(InputManager* inputManager, KeyboardButton key);
bool wasKeyboardButtonPressed(InputManager* inputManager, KeyboardButton key);
bool wasKeyboardButtonReleased(InputManager* inputManager, KeyboardButton key);

bool isMouseButtonPressed(InputManager* inputManager, MouseButton button);
bool wasMouseButtonPressed(InputManager* inputManager, MouseButton button);
bool wasMouseButtonReleased(InputManager* inputManager, MouseButton button);
