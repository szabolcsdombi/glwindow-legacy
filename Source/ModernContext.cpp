#include "ModernContext.hpp"

const unsigned WGL_ACCELERATION             = 0x2003;
const unsigned WGL_COLOR_BITS               = 0x2014;
const unsigned WGL_CONTEXT_FLAGS            = 0x2094;
const unsigned WGL_CONTEXT_MAJOR_VERSION    = 0x2091;
const unsigned WGL_CONTEXT_MINOR_VERSION    = 0x2092;
const unsigned WGL_DEPTH_BITS               = 0x2022;
const unsigned WGL_DOUBLE_BUFFER            = 0x2011;
const unsigned WGL_DRAW_TO_WINDOW           = 0x2001;
const unsigned WGL_FULL_ACCELERATION        = 0x2027;
const unsigned WGL_PIXEL_TYPE               = 0x2013;
const unsigned WGL_SAMPLES                  = 0x2042;
const unsigned WGL_STENCIL_BITS             = 0x2023;
const unsigned WGL_SUPPORT_OPENGL           = 0x2010;
const unsigned WGL_TYPE_RGBA                = 0x202B;
const unsigned WGL_CONTEXT_PROFILE_MASK     = 0x9126;
const unsigned WGL_CONTEXT_CORE_PROFILE_BIT = 0x0001;

typedef int (WINAPI * mglChoosePixelFormatProc)(HDC hdc, const int * piAttribIList, const float * pfAttribFList, unsigned nMaxFormats, int * piFormats, unsigned * nNumFormats);
typedef HGLRC (WINAPI * mglCreateContextAttribsProc)(HDC hdc, HGLRC hglrc, const int * attribList);

mglChoosePixelFormatProc mglChoosePixelFormat;
mglCreateContextAttribsProc mglCreateContextAttribs;

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
	0,								// cAccumBits
	0,								// cAccumRedBits
	0,								// cAccumGreenBits
	0,								// cAccumBlueBits
	0,								// cAccumAlphaBits
	24,								// cDepthBits
	0,								// cStencilBits
	0,								// cAuxBuffers
	0,								// iLayerType
	0,								// bReserved
	0,								// dwLayerMask
	0,								// dwVisibleMask
	0,								// dwDamageMask
};

bool initialized = false;

void InitModernContext() {
	if (initialized) {
		return;
	}
	
	initialized = true;

	HMODULE hinst = GetModuleHandle(0);

	if (!hinst) {
		return;
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
		"ContextLoader",				// lpszClassName
	};

	if (!RegisterClass(&extClass)) {
		return;
	}

	HWND loader_hwnd = CreateWindow(
		"ContextLoader",				// lpClassName
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
		return;
	}

	HDC loader_hdc = GetDC(loader_hwnd);

	if (!loader_hdc) {
		return;
	}

	int loader_pixelformat = ChoosePixelFormat(loader_hdc, &pfd);

	if (!loader_pixelformat) {
		return;
	}

	if (!SetPixelFormat(loader_hdc, loader_pixelformat, &pfd)) {
		return;
	}

	HGLRC loader_hglrc = wglCreateContext(loader_hdc);

	if (!loader_hglrc) {
		return;
	}

	if (!wglMakeCurrent(loader_hdc, loader_hglrc)) {
		return;
	}

	mglCreateContextAttribs = (mglCreateContextAttribsProc)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!mglCreateContextAttribs) {
		return;
	}

	mglChoosePixelFormat = (mglChoosePixelFormatProc)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!mglChoosePixelFormat) {
		return;
	}

	if (!wglMakeCurrent(0, 0)) {
		return;
	}

	if (!wglDeleteContext(loader_hglrc)) {
		return;
	}

	if (!ReleaseDC(loader_hwnd, loader_hdc)) {
		return;
	}

	if (!DestroyWindow(loader_hwnd)) {
		return;
	}

	if (!UnregisterClass("ContextLoader", hinst)) {
		return;
	}
}

HGLRC CreateModernContext(HDC hdc, int samples) {
	InitModernContext();

	HGLRC context = 0;

	if (mglChoosePixelFormat && mglCreateContextAttribs) {
		while (true) {
			int attributes[] = {
				WGL_SUPPORT_OPENGL, 1,
				WGL_DRAW_TO_WINDOW, 1,
				WGL_DOUBLE_BUFFER, 1,
				WGL_ACCELERATION, WGL_FULL_ACCELERATION,
				WGL_PIXEL_TYPE, WGL_TYPE_RGBA,
				WGL_COLOR_BITS, 24,
				WGL_DEPTH_BITS, 24,
				WGL_SAMPLES, samples,
				0, 0,
			};

			int pixelformat = 0;
			unsigned numFormats = 0;

			if (!mglChoosePixelFormat(hdc, attributes, 0, 1, &pixelformat, &numFormats)) {
				continue;
			}

			if (numFormats) {
				if (!SetPixelFormat(hdc, pixelformat, &pfd)) {
					continue;
				}

				int attriblist[] = {
					WGL_CONTEXT_PROFILE_MASK, WGL_CONTEXT_CORE_PROFILE_BIT,
					0, 0,
				};

				context = mglCreateContextAttribs(hdc, 0, attriblist);
				break;
			}

			if (!samples) {
				break;
			}
			
			samples /= 2;
		}
	}

	if (!context) {
		int pixelformat = ChoosePixelFormat(hdc, &pfd);

		if (!pixelformat) {
			return 0;
		}

		if (!SetPixelFormat(hdc, pixelformat, &pfd)) {
			return 0;
		}

		context = wglCreateContext(hdc);
	}

	return context;
}
