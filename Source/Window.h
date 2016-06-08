#pragma once

namespace Window {

	enum KeyStateEnum {
		KEY_DOWN     = 0x1,
		KEY_PRESSED  = 0x2,
		KEY_RELEASED = 0x4,
	};

	enum Buttons {
		BTN_LEFT_CLICK  = 0x01,
		BTN_RIGHT_CLICK = 0x02,
		BTN_CANCEL      = 0x03,
		BTN_MBUTTON     = 0x04,
		BTN_BACK        = 0x08,
		BTN_TAB         = 0x09,
		BTN_ENTER       = 0x0D,
		BTN_SHIFT       = 0x10,
		BTN_CTRL        = 0x11,
		BTN_ALT         = 0x12,
		BTN_ESCAPE      = 0x1B,
		BTN_SPACE       = 0x20,
		BTN_PAGE_UP     = 0x21,
		BTN_PAGE_DOWN   = 0x22,
		BTN_END         = 0x23,
		BTN_HOME        = 0x24,
		BTN_LEFT        = 0x25,
		BTN_UP          = 0x26,
		BTN_RIGHT       = 0x27,
		BTN_DOWN        = 0x28,
		BTN_INSERT      = 0x2D,
		BTN_DELETE      = 0x2E,
		BTN_NUMPAD_0    = 0x60,
		BTN_NUMPAD_1    = 0x61,
		BTN_NUMPAD_2    = 0x62,
		BTN_NUMPAD_3    = 0x63,
		BTN_NUMPAD_4    = 0x64,
		BTN_NUMPAD_5    = 0x65,
		BTN_NUMPAD_6    = 0x66,
		BTN_NUMPAD_7    = 0x67,
		BTN_NUMPAD_8    = 0x68,
		BTN_NUMPAD_9    = 0x69,
		BTN_MULTIPLY    = 0x6A,
		BTN_ADD         = 0x6B,
		BTN_SEPARATOR   = 0x6C,
		BTN_SUBTRACT    = 0x6D,
		BTN_DECIMAL     = 0x6E,
		BTN_DIVIDE      = 0x6F,
		BTN_F1          = 0x70,
		BTN_F2          = 0x71,
		BTN_F3          = 0x72,
		BTN_F4          = 0x73,
		BTN_F5          = 0x74,
		BTN_F6          = 0x75,
		BTN_F7          = 0x76,
		BTN_F8          = 0x77,
		BTN_F9          = 0x78,
		BTN_F10         = 0x79,
		BTN_F11         = 0x7A,
		BTN_F12         = 0x7B,
		BTN_NUMLOCK     = 0x90,
		BTN_LEFT_SHIFT  = 0xA0,
		BTN_RIGHT_SHIFT = 0xA1,
		BTN_LEFT_CTRL   = 0xA2,
		BTN_RIGHT_CTRL  = 0xA3,
		BTN_LELFT_ALT   = 0xA4,
		BTN_RIGHT_ALT   = 0xA5,
	};

	struct WindowSize {
		int width;
		int height;
	};

	struct Mouse {
		int x;
		int y;
		int wheel;
	};

	extern bool InitializeWindow();
	extern const wchar_t * GetError();
	extern bool BuildFullscreen(int major = 4, int minor = 0, int samples = 16, const wchar_t * title = L"");
	extern WindowSize GetSize();
	extern double GetFPS();
	extern double GetTime();
	extern int GetKey(int code);
	extern const wchar_t * GetInput();
	extern Mouse GetMouse();
	extern Mouse GetMouseDelta();
	extern bool Update();
	extern void SwapControl(bool on);

}
