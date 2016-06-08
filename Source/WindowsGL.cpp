#include "WindowsGL.h"

namespace WindowsGL {

	HMODULE gdi32;
	HMODULE opengl32;

	PROC_ChoosePixelFormat ChoosePixelFormat;
	PROC_SetPixelFormat SetPixelFormat;
	PROC_SwapBuffers SwapBuffers;
	PROC_wglGetCurrentContext wglGetCurrentContext;
	PROC_wglCreateContext wglCreateContext;
	PROC_wglDeleteContext wglDeleteContext;
	PROC_wglGetProcAddress wglGetProcAddress;
	PROC_wglMakeCurrent wglMakeCurrent;
	PROC_wglChoosePixelFormat wglChoosePixelFormat;
	PROC_wglCreateContextAttribs wglCreateContextAttribs;
	PROC_wglSwapInterval wglSwapInterval;

	bool LoadWindowsFunctionsFromGDI() {
		ChoosePixelFormat = (PROC_ChoosePixelFormat)GetProcAddress(gdi32, "ChoosePixelFormat");
		if (!ChoosePixelFormat) {
			return false;
		}
		SetPixelFormat = (PROC_SetPixelFormat)GetProcAddress(gdi32, "SetPixelFormat");
		if (!SetPixelFormat) {
			return false;
		}
		SwapBuffers = (PROC_SwapBuffers)GetProcAddress(gdi32, "SwapBuffers");
		if (!SwapBuffers) {
			return false;
		}
		return true;
	}

	bool LoadWindowsFunctionsFromOpenGL() {
		wglGetCurrentContext = (PROC_wglGetCurrentContext)GetProcAddress(opengl32, "wglGetCurrentContext");
		if (!wglGetCurrentContext) {
			return false;
		}
		wglCreateContext = (PROC_wglCreateContext)GetProcAddress(opengl32, "wglCreateContext");
		if (!wglCreateContext) {
			return false;
		}
		wglDeleteContext = (PROC_wglDeleteContext)GetProcAddress(opengl32, "wglDeleteContext");
		if (!wglDeleteContext) {
			return false;
		}
		wglGetProcAddress = (PROC_wglGetProcAddress)GetProcAddress(opengl32, "wglGetProcAddress");
		if (!wglGetProcAddress) {
			return false;
		}
		wglMakeCurrent = (PROC_wglMakeCurrent)GetProcAddress(opengl32, "wglMakeCurrent");
		if (!wglMakeCurrent) {
			return false;
		}
		return true;
	}

	bool LoadWindowsFunctionsFromContext() {
		wglChoosePixelFormat = (PROC_wglChoosePixelFormat)wglGetProcAddress("wglChoosePixelFormatARB");
		if (!wglChoosePixelFormat) {
			return false;
		}
		wglCreateContextAttribs = (PROC_wglCreateContextAttribs)wglGetProcAddress("wglCreateContextAttribsARB");
		if (!wglCreateContextAttribs) {
			return false;
		}
		wglSwapInterval = (PROC_wglSwapInterval)wglGetProcAddress("wglSwapIntervalEXT");
		if (!wglSwapInterval) {
			return false;
		}
		return true;
	}

	const void * GetWindowsFunctionFromOpenGL(const char * name) {
		return (const void *)GetProcAddress(opengl32, name);
	}

	PIXELFORMATDESCRIPTOR default_pfd = {
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
		0,								// cAccumBits
		0,								// cAccumRedBits
		0,								// cAccumGreenBits
		0,								// cAccumBlueBits
		0,								// cAccumAlphaBits
		32,								// cDepthBits
		8,								// cStencilBits
		0,								// cAuxBuffers
		0,								// iLayerType
		0,								// bReserved
		0,								// dwLayerMask
		0,								// dwVisibleMask
		0,								// dwDamageMask
	};

	bool InitializeWindowsGLModuleDone = false;

	bool InitializeWindowsGLModule() {
		if (InitializeWindowsGLModuleDone) {
			return true;
		}

		gdi32 = LoadLibrary("gdi32.dll");
		if (!gdi32) {
			return false;
		}

		if (!LoadWindowsFunctionsFromGDI()) {
			return false;
		}

		opengl32 = LoadLibrary("opengl32.dll");
		if (!opengl32) {
			return false;
		}

		if (!LoadWindowsFunctionsFromOpenGL()) {
			return false;
		}

		InitializeWindowsGLModuleDone = true;
		return true;
	}

	bool InitializeWindowsGLContextFunctionsDone = false;

	bool InitializeWindowsGLContextFunctions() {
		if (InitializeWindowsGLContextFunctionsDone) {
			return true;
		}

		if (!LoadWindowsFunctionsFromContext()) {
			return false;
		}

		InitializeWindowsGLContextFunctionsDone = true;
		return true;
	}

	bool InitializeWindowsGL() {
		if (!InitializeWindowsGLModule()) {
			return false;
		}

		HMODULE hinst = GetModuleHandle(0);

		if (!hinst) {
			return false;
		}

		WNDCLASS extClass = {
			CS_OWNDC,						// style
			DefWindowProc,					// lpfnWndProc
			0,								// cbClsExtra
			0,								// cbWndExtra
			hinst,							// hInstance
			0,								// hIcon
			0,								// hCursor
			0,								// hbrBackground
			0,								// lpszMenuName
			"WindowsGL",					// lpszClassName
		};

		if (!RegisterClass(&extClass)) {
			return false;
		}

		HWND loader_hwnd = CreateWindowEx(
			0,
			"WindowsGL",					// lpClassName
			0,								// lpWindowName
			0,								// dwStyle
			0,								// x
			0,								// y
			0,								// nWidth
			0,								// nHeight
			0,								// hWndParent
			0,								// hMenu
			hinst,							// hInstance
			0								// lpParam
		);

		if (!loader_hwnd) {
			return false;
		}

		HDC loader_hdc = GetDC(loader_hwnd);

		if (!loader_hdc) {
			return false;
		}

		int loader_pixelformat = WindowsGL::ChoosePixelFormat(loader_hdc, &default_pfd);

		if (!loader_pixelformat) {
			return false;
		}

		if (!WindowsGL::SetPixelFormat(loader_hdc, loader_pixelformat, &default_pfd)) {
			return false;
		}

		HGLRC loader_hglrc = WindowsGL::wglCreateContext(loader_hdc);

		if (!loader_hglrc) {
			return false;
		}

		if (!WindowsGL::wglMakeCurrent(loader_hdc, loader_hglrc)) {
			return false;
		}

		if (!InitializeWindowsGLContextFunctions()) {
			return false;
		}

		if (!WindowsGL::wglMakeCurrent(0, 0)) {
			return false;
		}

		if (!WindowsGL::wglDeleteContext(loader_hglrc)) {
			return false;
		}

		if (!ReleaseDC(loader_hwnd, loader_hdc)) {
			return false;
		}

		if (!DestroyWindow(loader_hwnd)) {
			return false;
		}

		if (!UnregisterClass("WindowsGL", hinst)) {
			return false;
		}

		return true;
	}

}
