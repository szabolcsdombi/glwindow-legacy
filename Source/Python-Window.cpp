#include "Python.h"
#include "Window.h"

PyObject * InitializeWindow(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":InitializeWindow")) {
		return 0;
	}
	if (Window::InitializeWindow()) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * GetError(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetError")) {
		return 0;
	}

	return PyUnicode_FromWideChar(Window::GetError(), -1);
}

PyObject * BuildFullscreen(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":BuildFullscreen")) {
		return 0;
	}
	if (Window::BuildFullscreen()) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * GetFPS(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetFPS")) {
		return 0;
	}
	return PyFloat_FromDouble(Window::GetFPS());
}

PyObject * GetTime(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetTime")) {
		return 0;
	}
	return PyFloat_FromDouble(Window::GetTime());
}

PyObject * Update(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":Update")) {
		return 0;
	}
	if (Window::Update()) {
		Py_RETURN_TRUE;
	} else {
		Py_RETURN_FALSE;
	}
}

PyObject * SwapControl(PyObject * self, PyObject * args) {
	bool on;

	if (!PyArg_ParseTuple(args, "p:SwapControl", &on)) {
		return 0;
	}

	Window::SwapControl(on);
	Py_RETURN_NONE;
}

PyObject * GetKey(PyObject * self, PyObject * args) {
	unsigned char code;

	if (!PyArg_ParseTuple(args, "b:GetKey", &code)) {
		return 0;
	}

	return PyLong_FromLong(Window::GetKey(code));
}

PyObject * GetMouse(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetMouse")) {
		return 0;
	}

	Window::Mouse mouse = Window::GetMouse();
	return Py_BuildValue("iii", mouse.x, mouse.y, mouse.wheel);
}

PyObject * GetMouseDelta(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetMouseDelta")) {
		return 0;
	}

	Window::Mouse mouse = Window::GetMouseDelta();
	return Py_BuildValue("iii", mouse.x, mouse.y, mouse.wheel);
}

PyObject * GetSize(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":GetSize")) {
		return 0;
	}

	Window::WindowSize size = Window::GetSize();
	return Py_BuildValue("ii", size.width, size.height);
}

static PyMethodDef methods[] = {
	{"InitializeWindow", InitializeWindow, METH_VARARGS, 0},
	{"GetError", GetError, METH_VARARGS, 0},
	{"BuildFullscreen", BuildFullscreen, METH_VARARGS, 0},
	{"GetSize", GetSize, METH_VARARGS, 0},
	{"GetFPS", GetFPS, METH_VARARGS, 0},
	{"GetTime", GetTime, METH_VARARGS, 0},
	{"Update", Update, METH_VARARGS, 0},
	{"SwapControl", SwapControl, METH_VARARGS, 0},
	{"GetKey", GetKey, METH_VARARGS, 0},
	{"GetMouse", GetMouse, METH_VARARGS, 0},
	{"GetMouseDelta", GetMouseDelta, METH_VARARGS, 0},
	{"GetSize", GetSize, METH_VARARGS, 0},
	{0, 0},
};

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT, "PyWindow", 0, -1,
	methods, 0, 0, 0, 0
};

extern "C" {
	PyObject * PyInit_GLWindow();
}

PyObject * PyInit_GLWindow() {
	return PyModule_Create(&moduledef);
}
