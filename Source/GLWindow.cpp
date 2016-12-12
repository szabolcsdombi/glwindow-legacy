#include "Python.h"
#include "structmember.h"

#define UNICODE

#include <Windows.h>

#include "ModernContext.hpp"

enum KeyState {
	KEY_UP,
	KEY_PRESSED,
	KEY_DOWN,
	KEY_RELEASED,
};

const int input_limit = 64 * 1024;
wchar_t input_buffer[input_limit + 1];
int input_counter;

long long counter_frequecy;
long long counter_first;
long long counter_last;

double time_delta;
double elapsed;

bool window_alive;
bool key_down[256];

int window_width;
int window_height;

int mx;
int my;
int mw;

HGLRC hglrc;
HWND hwnd;
HDC hdc;

KeyState key_state[256];

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CHAR: {
			if (input_counter < input_limit) {
				input_buffer[input_counter++] = (wchar_t)wParam;
			}
			break;
		}
		case WM_MOUSEWHEEL: {
			mw += GET_WHEEL_DELTA_WPARAM(wParam);
			break;
		}
		case WM_MOUSEMOVE: {
			POINT cursor;
			GetCursorPos(&cursor);
			int cx = window_width / 2;
			int cy = window_height / 2;
			if (cursor.x != cx || cursor.y != cy) {
				mx += cursor.x - cx;
				my += cursor.y - cy;
				SetCursorPos(cx, cy);
			}
			break;
		}
		case WM_LBUTTONDOWN: {
			key_down[1] = true;
			break;
		}
		case WM_LBUTTONUP: {
			key_down[1] = false;
			break;
		}
		case WM_RBUTTONDOWN: {
			key_down[2] = true;
			break;
		}
		case WM_RBUTTONUP: {
			key_down[2] = false;
			break;
		}
		case WM_MBUTTONDOWN: {
			key_down[3] = true;
			break;
		}
		case WM_MBUTTONUP: {
			key_down[3] = false;
			break;
		}
		case WM_KEYDOWN: {
			key_down[wParam & 0xFF] = true;
			break;
		}
		case WM_KEYUP: {
			key_down[wParam & 0xFF] = false;
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(hwnd);
			return(0);
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return(0);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

PyObject * Init(PyObject * self, PyObject * args) {
	int samples = 16;
	int dbg_border = 0;

	if (!PyArg_ParseTuple(args, "|II", &dbg_border, &samples)) {
		return 0;
	}

	QueryPerformanceFrequency((LARGE_INTEGER *)&counter_frequecy);

	HMODULE hinst = GetModuleHandle(0);

	if (!hinst) {
		return 0;
	}

	WNDCLASSW wndClass = {
		CS_OWNDC,						// style
		WindowProc,						// lpfnWndProc
		0,								// cbClsExtra
		0,								// cbWndExtra
		hinst,							// hInstance
		0,								// hIcon
		0,								// hCursor
		0,								// hbrBackground
		0,								// lpszMenuName
		L"GLWindow",					// lpszClassName
	};

	if (!RegisterClass(&wndClass)) {
		return 0;
	}

	window_width = GetSystemMetrics(SM_CXSCREEN);
	window_height = GetSystemMetrics(SM_CYSCREEN);

	hwnd = CreateWindowEx(
		0,								// exStyle
		L"GLWindow",					// lpClassName
		0,								// lpWindowName
		WS_POPUP,						// dwStyle
		0 + dbg_border,					// x
		0 + dbg_border,					// y
		window_width - dbg_border * 2,	// nWidth
		window_height - dbg_border * 2,	// nHeight
		0,								// hWndParent
		0,								// hMenu
		hinst,							// hInstance
		0								// lpParam
	);

	if (!hwnd) {
		return 0;
	}

	hdc = GetDC(hwnd);

	if (!hdc) {
		return 0;
	}

	hglrc = CreateModernContext(hdc, samples);

	if (!hglrc) {
		return 0;
	}

	if (!wglMakeCurrent(hdc, hglrc)) {
		return 0;
	}

	window_alive = true;
	Py_RETURN_NONE;
}

PyObject * Update(PyObject * self, PyObject * args) {
	if (!window_alive) {
		Py_RETURN_FALSE;
	}

	static bool window_visible = false;

	if (!window_visible) {
		QueryPerformanceCounter((LARGE_INTEGER *)&counter_first);
		
		int cx = window_width / 2;
		int cy = window_height / 2;
		SetCursorPos(cx, cy);
		ShowCursor(false);

		ShowWindow(hwnd, SW_SHOW);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);
		SetFocus(hwnd);

		window_visible = true;
	}

	input_buffer[input_counter] = 0;
	input_counter = 0;

	mx = 0;
	my = 0;
	mw = 0;

	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
		if (msg.message == WM_QUIT) {
			window_alive = false;
			Py_RETURN_FALSE;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SwapBuffers(hdc);

	for (int i = 0; i < 256; ++i) {
		switch (key_state[i]) {
			case KEY_PRESSED:
			case KEY_DOWN:
				key_state[i] = key_down[i] ? KEY_DOWN : KEY_RELEASED;
				break;

			case KEY_RELEASED:
			case KEY_UP:
				key_state[i] = key_down[i] ? KEY_PRESSED : KEY_UP;
				break;
		}
	}

	long long counter_prev = counter_last;
	QueryPerformanceCounter((LARGE_INTEGER *)&counter_last);
	time_delta = (double)(counter_last - counter_prev) / counter_frequecy;
	elapsed = (double)(counter_last - counter_first) / counter_frequecy;
	Py_RETURN_TRUE;
}

PyObject * Destroy(PyObject * self, PyObject * args) {
	Py_RETURN_NONE;
}

PyObject * GetSize(PyObject * self, PyObject * args) {
	PyObject * size = PyTuple_New(2);
	PyTuple_SET_ITEM(size, 0, PyLong_FromLong(window_width));
	PyTuple_SET_ITEM(size, 1, PyLong_FromLong(window_height));
	return size;
}

PyObject * GetTime(PyObject * self, PyObject * args) {
	return PyFloat_FromDouble(elapsed);
}

PyObject * GetTimeDelta(PyObject * self, PyObject * args) {
	return PyFloat_FromDouble(time_delta);
}

PyObject * GetMouse(PyObject * self, PyObject * args) {
	PyObject * mouse = PyTuple_New(3);
	PyTuple_SET_ITEM(mouse, 0, PyLong_FromLong(mx));
	PyTuple_SET_ITEM(mouse, 1, PyLong_FromLong(my));
	PyTuple_SET_ITEM(mouse, 2, PyLong_FromLong(mw));
	return mouse;
}

PyObject * GetText(PyObject * self, PyObject * args) {
	PyObject * text = 0;
	return text;
}

PyObject * KeyPressed(PyObject * self, PyObject * args) {
	if (PyTuple_Size(args) != 1) {
		return 0;
	}
	PyObject * arg = PyTuple_GET_ITEM(args, 0);
	PyObject * type = PyObject_Type(arg);
	int keycode = 0;
	if (type == (PyObject *)&PyUnicode_Type) {
		keycode = PyUnicode_ReadChar(arg, 0);
	} else if (type == (PyObject *)&PyLong_Type) {
		keycode = PyLong_AsLong(arg) & 0xFF;
	} else {
		return 0;
	}
	if (key_state[keycode] == KEY_PRESSED) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * KeyDown(PyObject * self, PyObject * args) {
	if (PyTuple_Size(args) != 1) {
		return 0;
	}
	PyObject * arg = PyTuple_GET_ITEM(args, 0);
	PyObject * type = PyObject_Type(arg);
	int keycode = 0;
	if (type == (PyObject *)&PyUnicode_Type) {
		keycode = PyUnicode_ReadChar(arg, 0);
	} else if (type == (PyObject *)&PyLong_Type) {
		keycode = PyLong_AsLong(arg) & 0xFF;
	} else {
		return 0;
	}
	if (key_state[keycode] != KEY_UP) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * KeyReleased(PyObject * self, PyObject * args) {
	if (PyTuple_Size(args) != 1) {
		return 0;
	}
	PyObject * arg = PyTuple_GET_ITEM(args, 0);
	PyObject * type = PyObject_Type(arg);
	int keycode = 0;
	if (type == (PyObject *)&PyUnicode_Type) {
		keycode = PyUnicode_ReadChar(arg, 0);
	} else if (type == (PyObject *)&PyLong_Type) {
		keycode = PyLong_AsLong(arg) & 0xFF;
	} else {
		return 0;
	}
	if (key_state[keycode] == KEY_RELEASED) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * KeyUp(PyObject * self, PyObject * args) {
	if (PyTuple_Size(args) != 1) {
		return 0;
	}
	PyObject * arg = PyTuple_GET_ITEM(args, 0);
	PyObject * type = PyObject_Type(arg);
	int keycode = 0;
	if (type == (PyObject *)&PyUnicode_Type) {
		keycode = PyUnicode_ReadChar(arg, 0);
	} else if (type == (PyObject *)&PyLong_Type) {
		keycode = PyLong_AsLong(arg) & 0xFF;
	} else {
		return 0;
	}
	if (key_state[keycode] == KEY_UP) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyMethodDef methods[] = {
	{"Init", Init, METH_VARARGS, 0},
	{"Update", Update, METH_VARARGS, 0},
	{"Destroy", Destroy, METH_VARARGS, 0},
	{"GetSize", GetSize, METH_VARARGS, 0},
	{"GetTime", GetTime, METH_VARARGS, 0},
	{"GetTimeDelta", GetTimeDelta, METH_VARARGS, 0},
	{"GetMouse", GetMouse, METH_VARARGS, 0},
	{"GetText", GetText, METH_VARARGS, 0},
	{"KeyPressed", KeyPressed, METH_VARARGS, 0},
	{"KeyDown", KeyDown, METH_VARARGS, 0},
	{"KeyReleased", KeyReleased, METH_VARARGS, 0},
	{"KeyUp", KeyUp, METH_VARARGS, 0},
	{0, 0},
};

PyModuleDef moduledef = {PyModuleDef_HEAD_INIT, "GLWindow", 0, -1, methods, 0, 0, 0, 0};

extern "C" {
	PyObject * PyInit_GLWindow();
}

PyObject * PyInit_GLWindow() {
	return PyModule_Create(&moduledef);
}
