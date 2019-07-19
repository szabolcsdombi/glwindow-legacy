#include <Python.h>
#include <structmember.h>

#include "core.hpp"
#include "keys.hpp"

#define Py_RETURN_BOOL(cond) if (cond) { Py_RETURN_TRUE; } else { Py_RETURN_FALSE; }

struct Window {
    PyObject_HEAD

    RawData raw;

    PyObject * data;
    PyObject * size;
    PyObject * text_input;

    PyObject * update_window_args;
    PyObject * update_window;
};

PyTypeObject * Window_type;
PyObject * keymap;

PyObject * default_create_window;
PyObject * default_update_window;

PyObject * glwindow_meth_default_create_window(PyObject * self, PyObject * arg) {
    if (!create_window(PyMemoryView_GET_BUFFER(arg)->buf)) {
        return NULL;
    }
    Py_INCREF(arg);
    return arg;
}

PyObject * glwindow_meth_default_update_window(PyObject * self, PyObject * arg) {
    if (!update_window(PyMemoryView_GET_BUFFER(arg)->buf)) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

Window * glwindow_meth_window(PyObject * self, PyObject * args, PyObject * kwargs) {
    static char * keywords[] = {"size", "title", "glversion", "window_handler", NULL};

    int width = 1280;
    int height = 720;
    int glversion = 330;
    PyObject * title = NULL;
    PyObject * create_window = default_create_window;
    PyObject * update_window = default_update_window;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|(ii)si(OO)", keywords, &width, &height, &PyUnicode_Type, &title, &glversion, &create_window, &update_window)) {
        return NULL;
    }

    Window * window = PyObject_New(Window, Window_type);
    memset(&window->raw, 0, sizeof(RawData));

    window->raw.width = width;
    window->raw.height = height;
    window->raw.glversion = glversion;

    #ifdef __linux__
    window->raw.title = strdup(title ? PyUnicode_AsUTF8(title) : "glwindow");
    #else
    window->raw.title = title ? PyUnicode_AsWideCharString(title, NULL) : L"glwindow";
    #endif

    window->size = Py_BuildValue("ii", window->raw.width, window->raw.height);
    window->data = PyMemoryView_FromMemory((char *)&window->raw, sizeof(RawData), PyBUF_WRITE);
    window->text_input = PyUnicode_FromString("");

    PyObject * update_window_args = PyObject_CallFunction(create_window, "O", window->data);
    if (!update_window_args) {
        return NULL;
    }

    window->update_window_args = Py_BuildValue("(O)", update_window_args);
    window->update_window = update_window;
    return window;
}

PyObject * Window_meth_update(Window * self) {
    memcpy(self->raw.old_key_down, self->raw.key_down, sizeof(self->raw.key_down));
    PyObject * res = PyObject_CallObject(self->update_window, self->update_window_args);
    if (self->raw.text_input_size || PyUnicode_GET_LENGTH(self->text_input)) {
        Py_DECREF(self->text_input);
        self->text_input = PyUnicode_FromWideChar(self->raw.text_input, self->raw.text_input_size);
    }
    self->raw.old_grab = self->raw.grab;
    self->raw.text_input_size = 0;
    return res;
}

PyObject * Window_meth_key_pressed(Window * self, PyObject * key) {
    PyObject * keycode = PyDict_GetItem(keymap, key);
    if (!keycode) {
        PyErr_BadInternalCall();
        return NULL;
    }
    int code = PyLong_AS_LONG(keycode);
    Py_RETURN_BOOL(self->raw.key_down[code] && !self->raw.old_key_down[code]);
}

PyObject * Window_meth_key_released(Window * self, PyObject * key) {
    PyObject * keycode = PyDict_GetItem(keymap, key);
    if (!keycode) {
        PyErr_BadInternalCall();
        return NULL;
    }
    int code = PyLong_AS_LONG(keycode);
    Py_RETURN_BOOL(!self->raw.key_down[code] && self->raw.old_key_down[code]);
}

PyObject * Window_meth_key_down(Window * self, PyObject * key) {
    PyObject * keycode = PyDict_GetItem(keymap, key);
    if (!keycode) {
        PyErr_BadInternalCall();
        return NULL;
    }
    int code = PyLong_AS_LONG(keycode);
    Py_RETURN_BOOL(self->raw.key_down[code]);
}

PyObject * Window_meth_key_up(Window * self, PyObject * key) {
    PyObject * keycode = PyDict_GetItem(keymap, key);
    if (!keycode) {
        PyErr_BadInternalCall();
        return NULL;
    }
    int code = PyLong_AS_LONG(keycode);
    Py_RETURN_BOOL(!self->raw.key_down[code]);
}

PyObject * Window_meth_grab_mouse(Window * self, PyObject * arg) {
    self->raw.grab = !!PyObject_IsTrue(arg);
    if (!self->raw.grab) {
        self->raw.dmx = 0;
        self->raw.dmy = 0;
    }
    Py_RETURN_NONE;
}

PyObject * Window_get_mouse(Window * self) {
    if (self->raw.grab) {
        return Py_BuildValue("ii", self->raw.dmx, self->raw.dmy);
    } else {
        return Py_BuildValue("ii", self->raw.mx, self->raw.my);
    }
}

void Window_dealloc(Window * self) {
    Py_TYPE(self)->tp_free(self);
}

PyMethodDef Window_methods[] = {
    {"update", (PyCFunction)Window_meth_update, METH_NOARGS, NULL},
    {"key_pressed", (PyCFunction)Window_meth_key_pressed, METH_O, NULL},
    {"key_released", (PyCFunction)Window_meth_key_released, METH_O, NULL},
    {"key_down", (PyCFunction)Window_meth_key_down, METH_O, NULL},
    {"key_up", (PyCFunction)Window_meth_key_up, METH_O, NULL},
    {"grab_mouse", (PyCFunction)Window_meth_grab_mouse, METH_O, NULL},
    {},
};

PyGetSetDef Window_getset[] = {
    {"mouse", (getter)Window_get_mouse, NULL, NULL, NULL},
    {},
};

PyMemberDef Window_members[] = {
    {"data", T_OBJECT_EX, offsetof(Window, data), READONLY, NULL},
    {"size", T_OBJECT_EX, offsetof(Window, size), READONLY, NULL},
    {"time", T_DOUBLE, offsetof(Window, raw.time), READONLY, NULL},
    {"text_input", T_OBJECT_EX, offsetof(Window, text_input), READONLY, NULL},
    {},
};

PyType_Slot Window_slots[] = {
    {Py_tp_methods, Window_methods},
    {Py_tp_getset, Window_getset},
    {Py_tp_members, Window_members},
    {Py_tp_dealloc, Window_dealloc},
    {},
};

PyType_Spec Window_spec = {"glwindow.Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};

PyMethodDef module_methods[] = {
    {"window", (PyCFunction)glwindow_meth_window, METH_VARARGS | METH_KEYWORDS, NULL},
    {"default_create_window", (PyCFunction)glwindow_meth_default_create_window, METH_O, NULL},
    {"default_update_window", (PyCFunction)glwindow_meth_default_update_window, METH_O, NULL},
    {},
};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "glwindow", NULL, -1, module_methods};

extern "C" PyObject * PyInit_glwindow() {
    PyObject * module = PyModule_Create(&module_def);

    keymap = PyDict_New();
    for (NamedKey * it = named_keys; it->name; ++it) {
        PyObject * keyname = PyUnicode_FromString(it->name);
        PyObject * keycode = PyLong_FromLong(it->code);
        PyDict_SetItem(keymap, keyname, keycode);
        Py_DECREF(keyname);
        Py_DECREF(keycode);
    }

    default_create_window = PyObject_GetAttrString(module, "default_create_window");
    default_update_window = PyObject_GetAttrString(module, "default_update_window");
    Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    PyModule_AddObject(module, "Window", (PyObject *)Window_type);
    return module;
}
