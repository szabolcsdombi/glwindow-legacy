#include <Python.h>

#define UNICODE
#include <Windows.h>
#include <GL/gl.h>

// extern "C" void WINAPI glViewport(int x, int y, int width, int height);

#include "ModernContext.hpp"

typedef void (WINAPI * wglSwapIntervalProc)(int interval);
typedef int (WINAPI * wglGetSwapIntervalProc)();

wglSwapIntervalProc wglSwapInterval;
wglGetSwapIntervalProc wglGetSwapInterval;

enum KeyState {
	KEY_UP,
	KEY_PRESSED,
	KEY_DOWN,
	KEY_RELEASED,
};

struct Window {
	PyObject_HEAD

	HANDLE mutex;
	HANDLE thread;
	HINSTANCE hinst;
	HWND hwnd;
	HDC hdc;
	HGLRC hrc;

	int width;
	int height;

	long long timer_counter;
	long long timer_last_counter;
	long long timer_frequency;
	double elapsed;

	bool show;
	bool disable_hotkeys;
	bool grab_mouse;
	bool show_fps;

	bool key_down[256];
	KeyState key_state[256];

	wchar_t text_input[2][256];
	int text_input_prefix;
	int text_input_size;
	int text_cursor;

	int mouse_delta_x;
	int mouse_delta_y;

	int frames;
	int seconds;
};

Window * window;

bool created;
bool barrier;
bool destroyed;

PyObject * Window_tp_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
	Window * self = (Window *)type->tp_alloc(type, 0);

	if (self) {
	}

	return (PyObject *)self;
}

void Window_tp_dealloc(Window * self) {
	Py_TYPE(self)->tp_free((PyObject *)self);
}

int Window_tp_init(Window * self, PyObject * args, PyObject * kwargs) {
	PyErr_Format(PyExc_NotImplementedError, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
	return -1;
}

void Window_Show(Window * self) {
	ShowWindow(self->hwnd, SW_SHOW);
	SetForegroundWindow(self->hwnd);
	SetActiveWindow(self->hwnd);
	SetFocus(self->hwnd);
	self->show = true;
}

PyObject * Window_fullscreen(Window * self) {
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	WaitForSingleObject(window->mutex, INFINITE);
	self->width = width;
	self->height = height;
	ReleaseMutex(window->mutex);

	SetWindowLong(self->hwnd, GWL_EXSTYLE, 0);
	SetWindowLong(self->hwnd, GWL_STYLE, WS_POPUP);
	SetWindowPos(self->hwnd, HWND_TOP, 0, 0, width, height, 0);

	if (self->show) {
		Window_Show(self);
	}

	glViewport(0, 0, width, height);

	Py_RETURN_NONE;
}

PyObject * Window_windowed(Window * self, PyObject * args) {
	int width;
	int height;

	int args_ok = PyArg_ParseTuple(
		args,
		"II",
		&width,
		&height
	);

	if (!args_ok) {
		return 0;
	}

	int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU; // | WS_THICKFRAME;
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);

	RECT rect = {};
	rect.right = width;
	rect.bottom = height;

	AdjustWindowRect(&rect, style, 0);

	int adjusted_width = rect.right - rect.left;
	int adjusted_height = rect.bottom - rect.top;

	WaitForSingleObject(window->mutex, INFINITE);
	self->width = adjusted_width;
	self->height = adjusted_height;
	ReleaseMutex(window->mutex);

	int x = (sw - width) / 2;
	int y = (sh - height) / 2;

	if (y < 0) {
		y = 0;
	}

	SetWindowLong(self->hwnd, GWL_EXSTYLE, WS_EX_DLGMODALFRAME);
	SetWindowLong(self->hwnd, GWL_STYLE, style);
	SetWindowPos(self->hwnd, HWND_TOP, x, y, adjusted_width, adjusted_height, 0);

	if (self->show) {
		Window_Show(self);
	}

	glViewport(0, 0, width, height);

	Py_RETURN_NONE;
}

PyObject * Window_update(Window * self) {
	SwapBuffers(self->hdc);

	if (!self->show) {
		Window_Show(self);
	}

	long long now;
	QueryPerformanceCounter((LARGE_INTEGER *)&now);
	double frame_time = (double)(now - self->timer_last_counter) / self->timer_frequency;
	int frame_time_ms = (int)(frame_time * 1000.0);

	if (frame_time_ms < 5) {
		Sleep(5 - frame_time_ms);
		QueryPerformanceCounter((LARGE_INTEGER *)&now);
	}

	self->timer_last_counter = now;
	self->elapsed = (double)(now - self->timer_counter) / self->timer_frequency;

	int seconds = (int)self->elapsed;

	if (self->seconds < seconds) {
		self->seconds = seconds;

		if (self->show_fps) {
			wchar_t title[256];
			wsprintf(title, L"FPS: %d", self->frames);
			SetWindowText(self->hwnd, title);
		}

		self->frames = 0;
	}

	self->frames += 1;

	if (!self->thread) {
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
			if (msg.message == WM_QUIT) {
				destroyed = true;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (!destroyed) {
		for (int i = 0; i < 256; ++i) {
			switch (self->key_state[i]) {
				case KEY_PRESSED:
				case KEY_DOWN:
					self->key_state[i] = self->key_down[i] ? KEY_DOWN : KEY_RELEASED;
					break;

				case KEY_RELEASED:
				case KEY_UP:
					self->key_state[i] = self->key_down[i] ? KEY_PRESSED : KEY_UP;
					break;
			}
		}

		WaitForSingleObject(window->mutex, INFINITE);
		self->text_input_size = self->text_cursor;
		self->text_input_prefix = (self->text_input_prefix + 1) % 2;
		self->text_cursor = 0;
		ReleaseMutex(window->mutex);

		if (self->hwnd == GetForegroundWindow()) {
			if (self->grab_mouse) {
				RECT rect;
				GetWindowRect(self->hwnd, &rect);
				int cx = (rect.left + rect.right) / 2;
				int cy = (rect.top + rect.bottom) / 2;
				POINT mouse;
				GetCursorPos(&mouse);
				self->mouse_delta_x = mouse.x - cx;
				self->mouse_delta_y = cy - mouse.y;
				SetCursorPos(cx, cy);
			} else {
				self->mouse_delta_x = 0;
				self->mouse_delta_y = 0;
			}

			if (!self->disable_hotkeys && self->key_state[VK_CONTROL] != KEY_UP && self->key_state[VK_SHIFT] != KEY_UP) {
				if (self->key_state['Q'] == KEY_PRESSED) {
					DestroyWindow(self->hwnd);
				}

				if (self->key_state['1'] == KEY_PRESSED) {
					Py_XDECREF(Window_fullscreen(self));
				}
				if (self->key_state['2'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(1920);
					PyObject * height = PyLong_FromLong(1080);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['3'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(1600);
					PyObject * height = PyLong_FromLong(900);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['4'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(1366);
					PyObject * height = PyLong_FromLong(768);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['5'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(1280);
					PyObject * height = PyLong_FromLong(720);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['6'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(1024);
					PyObject * height = PyLong_FromLong(768);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['7'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(800);
					PyObject * height = PyLong_FromLong(600);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['8'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(600);
					PyObject * height = PyLong_FromLong(400);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['9'] == KEY_PRESSED) {
					PyObject * width = PyLong_FromLong(320);
					PyObject * height = PyLong_FromLong(240);
					PyObject * args = PyTuple_Pack(2, width, height);
					Py_XDECREF(Window_windowed(self, args));
					Py_DECREF(args);
				}
				if (self->key_state['F'] == KEY_PRESSED) {
					self->show_fps = !self->show_fps;
				}
			}
		}
	}

	if (PyErr_Occurred()) {
		return 0;
	}

	return PyBool_FromLong(!destroyed);
}

PyObject * Window_make_current(Window * self) {
	PyErr_Format(PyExc_NotImplementedError, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
	return 0;
}

PyObject * Window_grab_mouse(Window * self, PyObject * args, PyObject * kwargs) {
	int grab;

	int args_ok = PyArg_ParseTuple(
		args,
		"p",
		&grab
	);

	if (!args_ok) {
		return 0;
	}

	bool grab_bool = grab ? true : false;

	if (grab_bool ^ self->grab_mouse) {
		RECT rect;
		GetWindowRect(self->hwnd, &rect);
		int cx = (rect.left + rect.right) / 2;
		int cy = (rect.top + rect.bottom) / 2;
		SetCursorPos(cx, cy);

		ShowCursor(!grab_bool);

		self->grab_mouse = grab_bool;
	}

	Py_RETURN_NONE;
}

PyObject * Window_key_pressed(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * key;

	int args_ok = PyArg_ParseTuple(
		args,
		"O",
		&key
	);

	if (!args_ok) {
		return 0;
	}

	int keycode = 0;

	if (Py_TYPE(key) == &PyUnicode_Type) {

		keycode = PyUnicode_ReadChar(key, 0);

	} else if (Py_TYPE(key) == &PyLong_Type) {

		keycode = PyLong_AsLong(key) & 0xFF;

	} else {

		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;

	}

	return PyBool_FromLong(self->key_state[keycode] == KEY_PRESSED);
}

PyObject * Window_key_down(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * key;

	int args_ok = PyArg_ParseTuple(
		args,
		"O",
		&key
	);

	if (!args_ok) {
		return 0;
	}

	int keycode = 0;

	if (Py_TYPE(key) == &PyUnicode_Type) {

		keycode = PyUnicode_ReadChar(key, 0);

	} else if (Py_TYPE(key) == &PyLong_Type) {

		keycode = PyLong_AsLong(key) & 0xFF;

	} else {

		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;

	}

	return PyBool_FromLong(self->key_state[keycode] != KEY_UP);
}

PyObject * Window_key_released(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * key;

	int args_ok = PyArg_ParseTuple(
		args,
		"O",
		&key
	);

	if (!args_ok) {
		return 0;
	}

	int keycode = 0;

	if (Py_TYPE(key) == &PyUnicode_Type) {

		keycode = PyUnicode_ReadChar(key, 0);

	} else if (Py_TYPE(key) == &PyLong_Type) {

		keycode = PyLong_AsLong(key) & 0xFF;

	} else {

		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;

	}

	return PyBool_FromLong(self->key_state[keycode] == KEY_RELEASED);
}

PyObject * Window_key_up(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * key;

	int args_ok = PyArg_ParseTuple(
		args,
		"O",
		&key
	);

	if (!args_ok) {
		return 0;
	}

	int keycode = 0;

	if (Py_TYPE(key) == &PyUnicode_Type) {

		keycode = PyUnicode_ReadChar(key, 0);

	} else if (Py_TYPE(key) == &PyLong_Type) {

		keycode = PyLong_AsLong(key) & 0xFF;

	} else {

		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;

	}

	return PyBool_FromLong(self->key_state[keycode] == KEY_UP);
}

PyObject * Window_set_icon(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * filename;

	int args_ok = PyArg_ParseTuple(
		args,
		"O!",
		&PyUnicode_Type,
		&filename
	);

	if (!args_ok) {
		return 0;
	}

	wchar_t * filename_str = PyUnicode_AsWideCharString(filename, 0);
 	HICON hicon = (HICON)LoadImage(0, filename_str, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
	PyMem_Free(filename_str);

	if (!hicon) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	SetClassLongPtr(self->hwnd, GCLP_HICON, (LONG_PTR)hicon);
	Py_RETURN_NONE;
}

PyObject * Window_set_small_icon(Window * self, PyObject * args, PyObject * kwargs) {
	PyObject * filename;

	int args_ok = PyArg_ParseTuple(
		args,
		"O!",
		&PyUnicode_Type,
		&filename
	);

	if (!args_ok) {
		return 0;
	}

	wchar_t * filename_str = PyUnicode_AsWideCharString(filename, 0);
 	HICON hicon = (HICON)LoadImage(0, filename_str, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
	PyMem_Free(filename_str);

	if (!hicon) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	SetClassLongPtr(self->hwnd, GCLP_HICONSM, (LONG_PTR)hicon);
	Py_RETURN_NONE;
}

PyMethodDef Window_tp_methods[] = {
	{"fullscreen", (PyCFunction)Window_fullscreen, METH_NOARGS, 0},
	{"windowed", (PyCFunction)Window_windowed, METH_VARARGS, 0},
	{"update", (PyCFunction)Window_update, METH_NOARGS, 0},
	{"make_current", (PyCFunction)Window_make_current, METH_NOARGS, 0},
	{"key_pressed", (PyCFunction)Window_key_pressed, METH_VARARGS, 0},
	{"key_down", (PyCFunction)Window_key_down, METH_VARARGS, 0},
	{"key_released", (PyCFunction)Window_key_released, METH_VARARGS, 0},
	{"key_up", (PyCFunction)Window_key_up, METH_VARARGS, 0},
	{"grab_mouse", (PyCFunction)Window_grab_mouse, METH_VARARGS, 0},
	{"set_icon", (PyCFunction)Window_set_icon, METH_VARARGS, 0},
	{"set_small_icon", (PyCFunction)Window_set_small_icon, METH_VARARGS, 0},
	{0},
};

PyObject * Window_get_mouse(Window * self, void * closure) {
	RECT rect;
	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(self->hwnd, &mouse);
	GetClientRect(self->hwnd, &rect);
	PyObject * x = PyLong_FromLong(mouse.x);
	PyObject * y = PyLong_FromLong(rect.bottom - rect.top - mouse.y - 1);
	return PyTuple_Pack(2, x, y);
}

PyObject * Window_get_mouse_delta(Window * self, void * closure) {
	PyObject * x = PyLong_FromLong(self->mouse_delta_x);
	PyObject * y = PyLong_FromLong(self->mouse_delta_y);
	return PyTuple_Pack(2, x, y);
}

PyObject * Window_get_size(Window * self, void * closure) {
	RECT rect;
	GetClientRect(self->hwnd, &rect);
	PyObject * width = PyLong_FromLong(rect.right - rect.left);
	PyObject * height = PyLong_FromLong(rect.bottom - rect.top);
	return PyTuple_Pack(2, width, height);
}

PyObject * Window_get_viewport(Window * self, void * closure) {
	RECT rect;
	GetClientRect(self->hwnd, &rect);
	PyObject * x = PyLong_FromLong(rect.left);
	PyObject * y = PyLong_FromLong(rect.top);
	PyObject * width = PyLong_FromLong(rect.right - rect.left);
	PyObject * height = PyLong_FromLong(rect.bottom - rect.top);
	return PyTuple_Pack(4, x, y, width, height);
}

int Window_set_title(Window * self, PyObject * value, void * closure) {
	if (Py_TYPE(value) != &PyUnicode_Type) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	wchar_t * title_str = PyUnicode_AsWideCharString(value, 0);
	SetWindowText(self->hwnd, title_str);
	PyMem_Free(title_str);

	return 0;
}

PyObject * Window_get_vsync(Window * self, void * closure) {
	if (wglGetSwapInterval) {
		return PyBool_FromLong(wglGetSwapInterval());
	}
	Py_RETURN_FALSE;
}

int Window_set_vsync(Window * self, PyObject * value, void * closure) {
	if (value == Py_True) {
		if (wglSwapInterval) {
			wglSwapInterval(1);
		}
	} else if (value == Py_False) {
		if (wglSwapInterval) {
			wglSwapInterval(0);
		}
	} else {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	return 0;
}

PyObject * Window_get_time(Window * self, void * closure) {
	return PyFloat_FromDouble(self->elapsed);
}

PyObject * Window_get_text_input(Window * self, void * closure) {
	WaitForSingleObject(window->mutex, INFINITE);
	wchar_t * text = self->text_input[(self->text_input_prefix + 1) % 2];
	ReleaseMutex(window->mutex);

	return PyUnicode_FromUnicode(text, self->text_input_size);
}

PyGetSetDef Window_tp_getseters[] = {
	{(char *)"mouse", (getter)Window_get_mouse, 0, 0, 0},
	{(char *)"mouse_delta", (getter)Window_get_mouse_delta, 0, 0, 0},
	{(char *)"size", (getter)Window_get_size, 0, 0, 0},
	{(char *)"viewport", (getter)Window_get_viewport, 0, 0, 0},
	{(char *)"title", 0, (setter)Window_set_title, 0, 0},
	{(char *)"vsync", (getter)Window_get_vsync, (setter)Window_set_vsync, 0, 0},
	{(char *)"time", (getter)Window_get_time, 0, 0, 0},
	{(char *)"text_input", (getter)Window_get_text_input, 0, 0, 0},
	{0},
};

PyTypeObject Window_Type = {
	PyVarObject_HEAD_INIT(0, 0)
	"glwnd.Window",                                         // tp_name
	sizeof(Window),                                         // tp_basicsize
	0,                                                      // tp_itemsize
	(destructor)Window_tp_dealloc,                          // tp_dealloc
	0,                                                      // tp_print
	0,                                                      // tp_getattr
	0,                                                      // tp_setattr
	0,                                                      // tp_reserved
	0,                                                      // tp_repr
	0,                                                      // tp_as_number
	0,                                                      // tp_as_sequence
	0,                                                      // tp_as_mapping
	0,                                                      // tp_hash
	0,                                                      // tp_call
	0,                                                      // tp_str
	0,                                                      // tp_getattro
	0,                                                      // tp_setattro
	0,                                                      // tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,               // tp_flags
	0,                                                      // tp_doc
	0,                                                      // tp_traverse
	0,                                                      // tp_clear
	0,                                                      // tp_richcompare
	0,                                                      // tp_weaklistoffset
	0,                                                      // tp_iter
	0,                                                      // tp_iternext
	Window_tp_methods,                                      // tp_methods
	0,                                                      // tp_members
	Window_tp_getseters,                                    // tp_getset
	0,                                                      // tp_base
	0,                                                      // tp_dict
	0,                                                      // tp_descr_get
	0,                                                      // tp_descr_set
	0,                                                      // tp_dictoffset
	(initproc)Window_tp_init,                               // tp_init
	0,                                                      // tp_alloc
	Window_tp_new,                                          // tp_new
};

Window * Window_New() {
	Window * self = (Window *)Window_tp_new(&Window_Type, 0, 0);
	return self;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!window) {
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg) {
		case WM_CLOSE: {
			DestroyWindow(window->hwnd);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_LBUTTONDOWN: {
			window->key_down[1] = true;
			break;
		}
		case WM_LBUTTONUP: {
			window->key_down[1] = false;
			break;
		}
		case WM_RBUTTONDOWN: {
			window->key_down[2] = true;
			break;
		}
		case WM_RBUTTONUP: {
			window->key_down[2] = false;
			break;
		}
		case WM_MBUTTONDOWN: {
			window->key_down[3] = true;
			break;
		}
		case WM_MBUTTONUP: {
			window->key_down[3] = false;
			break;
		}
		case WM_KEYDOWN: {
			window->key_down[wParam & 0xFF] = true;
			break;
		}
		case WM_KEYUP: {
			window->key_down[wParam & 0xFF] = false;
			break;
		}
		case WM_CHAR: {
			WaitForSingleObject(window->mutex, INFINITE);
			if (window->text_cursor < 256 - 1) {
				window->text_input[window->text_input_prefix][window->text_cursor++] = wParam & 0xFFFF;
			}
			ReleaseMutex(window->mutex);
			break;
		}
		case WM_GETMINMAXINFO: {
			WaitForSingleObject(window->mutex, INFINITE);
			MINMAXINFO * info = (MINMAXINFO *)lParam;
			info->ptMinTrackSize.x = window->width;
			info->ptMinTrackSize.y = window->height;
			info->ptMaxTrackSize.x = window->width;
			info->ptMaxTrackSize.y = window->height;
			ReleaseMutex(window->mutex);
			return 0;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: {
			static bool sys_alt = false;
			if (wParam == VK_MENU) {
				sys_alt = (uMsg == WM_SYSKEYDOWN);
			} else if (sys_alt && uMsg == WM_SYSKEYDOWN && wParam == VK_F4) {
				DestroyWindow(window->hwnd);
			}
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

struct CreateWindowParameters {
	PyObject * width;
	PyObject * height;
	int samples;
	int fullscreen;
	PyObject * title;
};

void Window_Create(CreateWindowParameters * parameters) {

	window->hinst = GetModuleHandle(0);

	if (!window->hinst) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	WNDCLASSW wnd_class = {
		CS_OWNDC,                             // style
		WindowProc,                           // lpfnWndProc
		0,                                    // cbClsExtra
		0,                                    // cbWndExtra
		window->hinst,                        // hInstance
		0,                                    // hIcon
		(HCURSOR)LoadCursor(0, IDC_ARROW),    // hCursor
		0,                                    // hbrBackground
		0,                                    // lpszMenuName
		L"GLWindow",                          // lpszClassName
	};

	if (!RegisterClass(&wnd_class)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	window->hwnd = CreateWindowEx(
		0,                           // exStyle
		L"GLWindow",                 // lpClassName
		0,                           // lpWindowName
		0,                           // dwStyle
		0,                           // x
		0,                           // y
		0,                           // nWidth
		0,                           // nHeight
		0,                           // hWndParent
		0,                           // hMenu
		window->hinst,               // hInstance
		0                            // lpParam
	);

	if (!window->hwnd) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	window->hdc = GetDC(window->hwnd);

	if (!window->hdc) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	window->hrc = CreateModernContext(window->hdc, parameters->samples);

	if (!window->hrc) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	if (parameters->fullscreen) {

		Py_XDECREF(Window_fullscreen(window));

	} else if (!parameters->fullscreen && parameters->width != Py_None && parameters->height != Py_None) {

		Py_INCREF(parameters->width);
		Py_INCREF(parameters->height);
		PyObject * args = PyTuple_Pack(2, parameters->width, parameters->height);
		Py_XDECREF(Window_windowed(window, args));
		Py_DECREF(args);

		if (PyErr_Occurred()) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			barrier = true;
			return;
		}

	} else if (!parameters->fullscreen && parameters->width == Py_None && parameters->height == Py_None) {

		PyObject * width = PyLong_FromLong(1280);
		PyObject * height = PyLong_FromLong(720);
		PyObject * args = PyTuple_Pack(2, width, height);
		Py_XDECREF(Window_windowed(window, args));
		Py_DECREF(args);

	} else {

		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;

	}

	if (parameters->title != Py_None && Window_set_title(window, parameters->title, 0)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		barrier = true;
		return;
	}

	barrier = true;
}

void Window_CreateThreaded(CreateWindowParameters * parameters) {

	Window_Create(parameters);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	destroyed = true;
}

Window * meth_create_window(PyObject * self, PyObject * args) {
	PyObject * width;
	PyObject * height;
	int samples;
	int fullscreen;
	PyObject * title;
	int threaded;

	int args_ok = PyArg_ParseTuple(
		args,
		"OOIpOp",
		&width,
		&height,
		&samples,
		&fullscreen,
		&title,
		&threaded
	);

	if (!args_ok) {
		return 0;
	}

	if (created) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if (samples & (samples - 1)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	CreateWindowParameters parameters = {
		width,
		height,
		samples,
		fullscreen,
		title,
	};

	window = Window_New();
	Py_INCREF(window);

	window->mutex = CreateMutex(0, false, 0);
	window->thread = 0;
	window->hinst = 0;
	window->hwnd = 0;
	window->hdc = 0;
	window->hrc = 0;

	window->width = 0;
	window->height = 0;

	window->timer_counter = 0;
	window->timer_last_counter = 0;
	window->timer_frequency = 0;
	window->elapsed = 0.0;

	window->show = false;
	window->disable_hotkeys = false;
	window->grab_mouse = false;
	window->show_fps = false;

	for (int i = 0; i < 256; ++i) {
		window->key_down[i] = false;
	}

	for (int i = 0; i < 256; ++i) {
		window->key_state[256] = KEY_UP;
	}

	for (int i = 0; i < 256; ++i) {
		window->text_input[0][256] = 0;
		window->text_input[1][256] = 0;
	}

	window->text_input_size = 0;
	window->text_cursor = 0;

	window->mouse_delta_x = 0;
	window->mouse_delta_y = 0;

	window->frames = 0;
	window->seconds = 0;

	if (threaded) {
		window->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Window_CreateThreaded, &parameters, 0, 0);

		while (!barrier) {
			Sleep(1);
		}
	} else {
		window->thread = 0;
		Window_Create(&parameters);
	}

	if (PyErr_Occurred()) {
		Py_DECREF(window);
		if (window->thread) {
			TerminateThread(window->thread, 0);
		}
		return 0;
	}

	if (!wglMakeCurrent(window->hdc, window->hrc)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		Py_DECREF(window);
		if (window->thread) {
			TerminateThread(window->thread, 0);
		}
		return 0;
	}

	wglSwapInterval = (wglSwapIntervalProc)wglGetProcAddress("wglSwapIntervalEXT");

	if (wglSwapInterval) {
		wglSwapInterval(1);
	}

	for (int i = 0; i < 4; ++i) {
		SwapBuffers(window->hdc);
	}

	QueryPerformanceFrequency((LARGE_INTEGER *)&window->timer_frequency);
	QueryPerformanceCounter((LARGE_INTEGER *)&window->timer_counter);
	window->timer_last_counter = window->timer_counter;

	created = true;

	return window;
}

PyObject * meth_get_window(PyObject * self, PyObject * args) {
	if (window) {
		Py_INCREF(window);
		return (PyObject *)window;
	} else {
		Py_RETURN_NONE;
	}
}

PyMethodDef methods[] = {
	{"create_window", (PyCFunction)meth_create_window, METH_VARARGS, 0},
	{"get_window", (PyCFunction)meth_get_window, METH_NOARGS, 0},
	{0},
};

#if PY_MAJOR_VERSION >= 3

PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"glwnd",
	0,
	-1,
	methods,
	0,
	0,
	0,
	0,
};

PyObject * InitializeGLWindow(PyObject * module) {
	if (PyType_Ready(&Window_Type) < 0) {
		PyErr_Format(PyExc_ImportError, "Cannot register Window in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	Py_INCREF(&Window_Type);

	PyModule_AddObject(module, "Window", (PyObject *)&Window_Type);

	return module;
}

extern "C" PyObject * PyInit_glwnd() {
	PyObject * module = PyModule_Create(&moduledef);
	return InitializeGLWindow(module);
}

#else

extern "C" PyObject * initglwnd() {
	PyObject * module = Py_InitModule("glwnd", methods);
	return InitializeGLWindow(module);
}

#endif
