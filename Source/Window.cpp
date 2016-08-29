#include "Window.h"

#define UNICODE
#define _WIN32_WINNT 0x500

#include <Windows.h>
#include <cstdio>

#include "WindowsGL.h"

const wchar_t * errorMessage = L"";
bool moduleReady = false;

long long counterCurrent;
long long counterPrevious;
long long counterFrequecy;
long long counterFirst;

const int InputBufferSize = 64;
wchar_t inputBuffer[InputBufferSize + 1];
int inputBufferIndex = 0;

int key_state[256];
bool key_pressed[256];
bool key_down[256];

int mx, my;
int wheel;

int old_mx, old_my;
int old_wheel;

bool windowAlive;
int windowWidth;
int windowHeight;
// HMODULE hmod;
HGLRC hglrc;
HWND hwnd;
HDC hdc;

PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// nSize
	1,								// nVersion
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_OPENGL |
	PFD_GENERIC_ACCELERATED |
	PFD_DOUBLEBUFFER,				// dwFlags
	0,								// iPixelType
	32,								// cColorBits
	0,								// cRedBits
	0,								// cRedShift
	0,								// cGreenBits
	0,								// cGreenShift
	0,								// cBlueBits
	0,								// cBlueShift
	0,								// cAlphaBits
	0,								// cAlphaShift
	32,								// cAccumBits
	0,								// cAccumRedBits
	0,								// cAccumGreenBits
	0,								// cAccumBlueBits
	0,								// cAccumAlphaBits
	32,								// cDepthBits
	32,								// cStencilBits
	0,								// cAuxBuffers
	0,								// iLayerType
	0,								// bReserved
	0,								// dwLayerMask
	0,								// dwVisibleMask
	0								// dwDamageMask
};

// bool ExtensionLoader() {
// 	HMODULE hinst = GetModuleHandle(0);

// 	if (!hinst) {
// 		errorMessage = L"GetModuleHandle failed.";
// 		return false;
// 	}

// 	WNDCLASSW extClass = {
// 		CS_OWNDC,						// style
// 		DefWindowProc,					// lpfnWndProc
// 		0,								// cbClsExtra
// 		0,								// cbWndExtra
// 		hinst,							// hInstance
// 		0,								// hIcon
// 		0,								// hCursor
// 		0,								// hbrBackground
// 		0,								// lpszMenuName
// 		L"ExtOpenGL",					// lpszClassName
// 	};

// 	if (!RegisterClass(&extClass)) {
// 		errorMessage = L"RegisterClass failed.";
// 		return false;
// 	}

// 	HWND loader_hwnd = CreateWindowEx(
// 		0,
// 		L"ExtOpenGL",					// lpClassName
// 		0,								// lpWindowName
// 		0,								// dwStyle
// 		0,								// x
// 		0,								// y
// 		0,								// nWidth
// 		0,								// nHeight
// 		0,								// hWndParent
// 		0,								// hMenu
// 		hinst,							// hInstance
// 		0								// lpParam
// 	);

// 	if (!loader_hwnd) {
// 		errorMessage = L"CreateWindowEx failed.";
// 		return false;
// 	}

// 	HDC loader_hdc = GetDC(loader_hwnd);

// 	if (!loader_hdc) {
// 		errorMessage = L"GetDC failed.";
// 		return false;
// 	}

// 	int loader_pixelformat = ChoosePixelFormat(loader_hdc, &pfd);

// 	if (!loader_pixelformat) {
// 		errorMessage = L"ChoosePixelFormat failed.";
// 		return false;
// 	}

// 	if (!SetPixelFormat(loader_hdc, loader_pixelformat, &pfd)) {
// 		errorMessage = L"SetPixelFormat failed.";
// 		return false;
// 	}

// 	HGLRC loader_hglrc = wglCreateContext(loader_hdc);

// 	if (!loader_hglrc) {
// 		errorMessage = L"wglCreateContext failed.";
// 		return false;
// 	}

// 	if (!wglMakeCurrent(loader_hdc, loader_hglrc)) {
// 		errorMessage = L"wglMakeCurrent failed.";
// 		return false;
// 	}

// 	wglCreateContextAttribs = (wglCreateContextAttribsProc)wglGetProcAddress("wglCreateContextAttribsARB");
// 	if (!wglCreateContextAttribs) {
// 		errorMessage = L"wglCreateContextAttribsARB not found.";
// 		return false;
// 	}

// 	wglChoosePixelFormat = (wglChoosePixelFormatProc)wglGetProcAddress("wglChoosePixelFormatARB");
// 	if (!wglChoosePixelFormat) {
// 		errorMessage = L"wglChoosePixelFormatARB not found.";
// 		return false;
// 	}

// 	wglSwapInterval = (wglSwapIntervalProc)wglGetProcAddress("wglSwapIntervalEXT");
// 	if (!wglSwapInterval) {
// 		errorMessage = L"wglSwapIntervalEXT not found.";
// 		return false;
// 	}

// 	if (!wglMakeCurrent(0, 0)) {
// 		errorMessage = L"wglMakeCurrent failed.";
// 		return false;
// 	}

// 	if (!wglDeleteContext(loader_hglrc)) {
// 		errorMessage = L"wglDeleteContext failed.";
// 		return false;
// 	}

// 	if (!ReleaseDC(loader_hwnd, loader_hdc)) {
// 		errorMessage = L"ReleaseDC failed.";
// 		return false;
// 	}

// 	if (!DestroyWindow(loader_hwnd)) {
// 		errorMessage = L"DestroyWindow failed.";
// 		return false;
// 	}

// 	if (!UnregisterClass(L"ExtOpenGL", hinst)) {
// 		errorMessage = L"UnregisterClass failed.";
// 		return false;
// 	}
	
// 	return true;
// }

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CHAR: {
			if (inputBufferIndex < InputBufferSize) {
				inputBuffer[inputBufferIndex++] = (wchar_t)wParam;
			}
			break;
		}
		case WM_MOUSEWHEEL: {
			wheel += GET_WHEEL_DELTA_WPARAM(wParam);
			break;
		}
		case WM_MOUSEMOVE: {
			POINT cursor;
			GetCursorPos(&cursor);
			int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
			int cy = GetSystemMetrics(SM_CYSCREEN) / 2;
			if (cursor.x != cx || cursor.y != cy) {
				mx += cursor.x - cx;
				my += cursor.y - cy;
				SetCursorPos(cx, cy);
			}
			break;
		}
		case WM_LBUTTONDOWN: {
			key_pressed[1] = true;
			key_down[1] = true;
			break;
		}
		case WM_LBUTTONUP: {
			key_down[1] = false;
			break;
		}
		case WM_RBUTTONDOWN: {
			key_pressed[2] = true;
			key_down[2] = true;
			break;
		}
		case WM_RBUTTONUP: {
			key_down[2] = false;
			break;
		}
		case WM_MBUTTONDOWN: {
			key_pressed[3] = true;
			key_down[3] = true;
			break;
		}
		case WM_MBUTTONUP: {
			key_down[3] = false;
			break;
		}
		case WM_KEYDOWN: {
			key_pressed[wParam] = true;
			key_down[wParam] = true;
			break;
		}
		case WM_KEYUP: {
			key_down[wParam] = false;
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(hWnd);
			return(0);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return(0);
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

namespace Window {

	long long GetCounter() {
		long long result;
		QueryPerformanceCounter((LARGE_INTEGER *)&result);
		return result;
	}

	bool BuildFullscreen(int major, int minor, int samples, const wchar_t * title) {
		if (!moduleReady) {
			errorMessage = L"Module was not initialized properly.";
			return false;
		}

		bool version_valid = true;
		if (minor < 0) {
			version_valid = false;
		}
		if (major == 3) {
			if (minor > 3) {
				version_valid = false;
			}
		} else if (major == 4) {
			if (minor > 5) {
				version_valid = false;
			}
		} else {
			version_valid = false;
		}

		if (!version_valid) {
			errorMessage = L"Invalid OpenGL version.";
			return false;
		}

		if (windowAlive) {
			errorMessage = L"Window already exists.";
			return false;
		}

		if (samples != 0 && samples != 1 && samples != 2 && samples != 4 && samples != 8 && samples != 16 && samples != 32) {
			errorMessage = L"Invalid number of samples.";
			return false;
		}

		HMODULE hinst = GetModuleHandle(0);

		if (!hinst) {
			errorMessage = L"GetModuleHandle failed.";
			return false;
		}

		LPCWSTR largeIcon = 0; // MAKEINTRESOURCE(10001);

		WNDCLASSW wndClass = {
			CS_OWNDC,						// style
			WindowProc,						// lpfnWndProc
			0,								// cbClsExtra
			0,								// cbWndExtra
			hinst,							// hInstance
			0, //LoadIcon(hmod, largeIcon),	// hIcon
			LoadCursor(0, IDC_ARROW),		// hCursor
			0,								// hbrBackground
			0,								// lpszMenuName
			L"OpenGL",						// lpszClassName
		};

		if (!RegisterClass(&wndClass)) {
			errorMessage = L"RegisterClass failed.";
			return false;
		}

		windowWidth = GetSystemMetrics(SM_CXSCREEN);
		windowHeight = GetSystemMetrics(SM_CYSCREEN);

		hwnd = CreateWindowEx(
			0,
			L"OpenGL",						// lpClassName
			title,							// lpWindowName
			WS_POPUP,						// dwStyle
			0,								// x
			0,								// y
			windowWidth,					// nWidth
			windowHeight,					// nHeight
			0,								// hWndParent
			0,								// hMenu
			hinst,							// hInstance
			0								// lpParam
		);

		if (!hwnd) {
			errorMessage = L"CreateWindowEx failed.";
			return false;
		}

		hdc = GetDC(hwnd);

		if (!hdc) {
			errorMessage = L"GetDC failed.";
			return false;
		}

		int pixelformat = 0;

		while (true) {
			int attributes[] = {
				WindowsGL::WGL_SUPPORT_OPENGL, 1,
				WindowsGL::WGL_DRAW_TO_WINDOW, 1,
				WindowsGL::WGL_DOUBLE_BUFFER, 1,
				WindowsGL::WGL_ACCELERATION, WindowsGL::WGL_FULL_ACCELERATION,
				WindowsGL::WGL_PIXEL_TYPE, WindowsGL::WGL_TYPE_RGBA,
				WindowsGL::WGL_COLOR_BITS, 24,
				WindowsGL::WGL_DEPTH_BITS, 24,
				WindowsGL::WGL_SAMPLES, samples,
				0, 0,
			};

			unsigned numFormats = 0;

			if (!WindowsGL::wglChoosePixelFormat(hdc, attributes, 0, 1, &pixelformat, &numFormats)) {
				errorMessage = L"wglChoosePixelFormat failed.";
				return false;
			}
			if (numFormats) {
				break;
			}
			if (!samples) {
				errorMessage = L"wglChoosePixelFormat failed.";
				return false;
			}
			samples /= 2;
		}

		if (!WindowsGL::SetPixelFormat(hdc, pixelformat, &pfd)) {
			errorMessage = L"SetPixelFormat failed.";
			return false;
		}

		// int attriblist[] = {
		// 	WindowsGL::WGL_CONTEXT_MAJOR_VERSION, major,
		// 	WindowsGL::WGL_CONTEXT_MINOR_VERSION, minor,
		// 	0, 0,
		// };

		hglrc = WindowsGL::wglCreateContextAttribs(hdc, 0, 0);

		if (!hglrc) {
			errorMessage = L"wglCreateContextAttribs failed.";
			return false;
		}

		if (!WindowsGL::wglMakeCurrent(hdc, hglrc)) {
			errorMessage = L"wglMakeCurrent failed.";
			return false;
		}

		QueryPerformanceFrequency((LARGE_INTEGER *)&counterFrequecy);
		counterFirst = GetCounter();
		counterCurrent = counterFirst;
		counterPrevious = counterFirst;
		windowAlive = true;

		int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
		int cy = GetSystemMetrics(SM_CYSCREEN) / 2;
		SetCursorPos(cx, cy);
		ShowCursor(false);

		ShowWindow(hwnd, SW_SHOW);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);
		SetFocus(hwnd);
		return true;
	}

	const wchar_t * GetError() {
		const wchar_t * result = errorMessage;
		errorMessage = L"";
		return result;
	}

	WindowSize GetSize() {
		WindowSize result = {
			windowWidth,
			windowHeight,
		};
		return result;
	}

	double GetFPS() {
		return (double)counterFrequecy / (counterCurrent - counterPrevious);
	}

	double GetTime() {
		return (double)(GetCounter() - counterFirst) / counterFrequecy;
	}

	int GetKey(int code) {
		return key_state[code];
	}

	Mouse GetMouse() {
		Mouse result = {
			mx,
			my,
			wheel,
		};
		return result;
	}

	Mouse GetMouseDelta() {
		Mouse result = {
			mx - old_mx,
			my - old_my,
			wheel - old_wheel,
		};
		return result;
	}

	const wchar_t * GetInput() {
		return inputBuffer;
	}

	bool Update() {
		MSG msg;
		inputBuffer[inputBufferIndex] = 0;
		inputBufferIndex = 0;

		old_wheel = wheel;
		old_mx = mx;
		old_my = my;

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
			if (msg.message == WM_QUIT) {
				windowAlive = false;
				return false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		WindowsGL::SwapBuffers(hdc);

		for (int i = 0; i < 256; ++i) {
			bool was_pressed = key_state[i] & KEY_PRESSED;
			key_state[i] = 0;
			if (key_down[i]) {
				key_state[i] |= KEY_DOWN;
			}
			if (key_pressed[i]) {
				key_pressed[i] = false;
				key_state[i] |= KEY_PRESSED;
				if (was_pressed) {
					key_state[i] |= KEY_RELEASED;
				}
			} else {
				if (!key_down[i]) {
					key_state[i] |= KEY_RELEASED;
				}
			}
		}

		counterPrevious = counterCurrent;
		counterCurrent = GetCounter();
		return true;
	}

	void SwapControl(bool on) {
		WindowsGL::wglSwapInterval(on);
	}

	bool InitializeWindow() {
		// moduleReady = ExtensionLoader();
		moduleReady = WindowsGL::InitializeWindowsGL();
		return moduleReady;
	}

}

// BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
// 	switch (reason) {
// 		case DLL_PROCESS_ATTACH:
// 			hmod = hModule;
// 			break;

// 		case DLL_THREAD_ATTACH:
// 		case DLL_THREAD_DETACH:
// 		case DLL_PROCESS_DETACH:
// 			break;
// 	}

// 	return true;
// }
