#pragma once

#include <Windows.h>

namespace WindowsGL {

	const unsigned WGL_ACCELERATION            = 0x2003;
	const unsigned WGL_COLOR_BITS              = 0x2014;
	const unsigned WGL_CONTEXT_FLAGS           = 0x2094;
	const unsigned WGL_CONTEXT_MAJOR_VERSION   = 0x2091;
	const unsigned WGL_CONTEXT_MINOR_VERSION   = 0x2092;
	const unsigned WGL_DEPTH_BITS              = 0x2022;
	const unsigned WGL_DOUBLE_BUFFER           = 0x2011;
	const unsigned WGL_DRAW_TO_WINDOW          = 0x2001;
	const unsigned WGL_FULL_ACCELERATION       = 0x2027;
	const unsigned WGL_PIXEL_TYPE              = 0x2013;
	const unsigned WGL_SAMPLES                 = 0x2042;
	const unsigned WGL_STENCIL_BITS            = 0x2023;
	const unsigned WGL_SUPPORT_OPENGL          = 0x2010;
	const unsigned WGL_TYPE_RGBA               = 0x202B;

	extern const void * GetWindowsFunctionFromOpenGL(const char * name);

	extern bool InitializeWindowsGLModule();
	extern bool InitializeWindowsGLContextFunctions();

	extern PIXELFORMATDESCRIPTOR default_pfd;

	typedef int (WINAPI * PROC_ChoosePixelFormat)(HDC hdc, const PIXELFORMATDESCRIPTOR * ppfd);
	typedef BOOL (WINAPI * PROC_SetPixelFormat)(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR * ppfd);
	typedef BOOL (WINAPI * PROC_SwapBuffers)(HDC hdc);

	typedef HGLRC (WINAPI * PROC_wglGetCurrentContext)();
	typedef HGLRC (WINAPI * PROC_wglCreateContext)(HDC hdc);
	typedef BOOL (WINAPI * PROC_wglDeleteContext)(HGLRC hglrc);
	typedef PROC (WINAPI * PROC_wglGetProcAddress)(LPCSTR lpszProc);
	typedef BOOL (WINAPI * PROC_wglMakeCurrent)(HDC hdc, HGLRC hglrc);

	typedef int (WINAPI * PROC_wglChoosePixelFormat)(HDC hdc, const int * piAttribIList, const float * pfAttribFList, unsigned nMaxFormats, int * piFormats, unsigned * nNumFormats);
	typedef HGLRC (WINAPI * PROC_wglCreateContextAttribs)(HDC hdc, HGLRC hglrc, const int * attribList);
	typedef BOOL (WINAPI * PROC_wglSwapInterval)(int interval);

	// 

	extern bool InitializeWindowsGL();

	extern PROC_ChoosePixelFormat ChoosePixelFormat;
	extern PROC_SetPixelFormat SetPixelFormat;
	extern PROC_SwapBuffers SwapBuffers;
	extern PROC_wglGetCurrentContext wglGetCurrentContext;
	extern PROC_wglCreateContext wglCreateContext;
	extern PROC_wglDeleteContext wglDeleteContext;
	extern PROC_wglGetProcAddress wglGetProcAddress;
	extern PROC_wglMakeCurrent wglMakeCurrent;
	extern PROC_wglChoosePixelFormat wglChoosePixelFormat;
	extern PROC_wglCreateContextAttribs wglCreateContextAttribs;
	extern PROC_wglSwapInterval wglSwapInterval;
}
