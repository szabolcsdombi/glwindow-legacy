#include <Python.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

struct Input {
    bool keys[256];
    int mouse_rx;
    int mouse_ry;
    int mouse_x;
    int mouse_y;
    int wheel;
    bool visible;
    bool grab;
    int mouse_restore_x;
    int mouse_restore_y;
};

struct XWindow {
    PyObject_HEAD

    int width;
    int height;

    Input input[3];
    bool grab_mouse[2];

    Window window;
    Cursor hidden_cursor;
    Atom delete_message;
};

PyTypeObject * Window_type;
PyObject * window_list;
PyObject * keymap;

Display * display;
XContext window_ctx;

XWindow * glwindow_meth_window(PyObject * self, PyObject * vargs, PyObject * kwargs) {
    static char * keywords[] = {"size", "title", NULL};

    struct {
        int width = 1280;
        int height = 720;
        const char * title = NULL;
    } args;

    if (!PyArg_ParseTupleAndKeywords(vargs, kwargs, "|(II)z", keywords, &args.width, &args.height, &args.title)) {
        return NULL;
    }

    XWindow * res = PyObject_New(XWindow, Window_type);

    res->width = args.width;
    res->height = args.height;
    memset(res->input, 0, sizeof(res->input));
    memset(res->grab_mouse, 0, sizeof(res->grab_mouse));

    res->window = XCreateSimpleWindow(
        display,
        RootWindow(display, DefaultScreen(display)),
        100,
        100,
        args.width,
        args.height,
        4,
        0,
        0
    );

    res->delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, res->window, &res->delete_message, 1);

    XColor black = {};
    char empty_icon[1024] = {};
    Pixmap empty_bitmap = XCreateBitmapFromData(display, res->window, empty_icon, 8, 8);
    res->hidden_cursor = XCreatePixmapCursor(display, empty_bitmap, empty_bitmap, &black, &black, 0, 0);

    XSizeHints * sh = XAllocSizeHints();
    sh->flags = PMinSize | PMaxSize;
    sh->min_width = args.width;
    sh->max_width = args.width;
    sh->min_height = args.height;
    sh->max_height = args.height;
    XSetWMSizeHints(display, res->window, sh, XA_WM_NORMAL_HINTS);
    XFree(sh);

    XStoreName(display, res->window, args.title);

    int events = ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;
    XSelectInput(display, res->window, events);

    XSaveContext(display, res->window, window_ctx, (XPointer)res);

    PyList_Append(window_list, (PyObject *)res);
    return res;
}

PyObject * glwindow_meth_update(PyObject * self) {
    int event_count = XPending(display);
    for (int i = 0; i < event_count; ++i) {
        XEvent event;
        XNextEvent(display, &event);
        if (XFilterEvent(&event, None)) {
            continue;
        }

        switch (event.type) {
            case KeyPress: {
                XWindow * window;
                XFindContext(display, event.xkey.window, window_ctx, (XPointer *)&window);
                window->input[0].keys[event.xkey.keycode & 0xff] = true;
                break;
            }

            case KeyRelease: {
                XWindow * window;
                XFindContext(display, event.xkey.window, window_ctx, (XPointer *)&window);
                window->input[0].keys[event.xkey.keycode & 0xff] = false;
                break;
            }

            case ButtonPress: {
                XWindow * window;
                XFindContext(display, event.xbutton.window, window_ctx, (XPointer *)&window);
                window->input[0].keys[event.xbutton.button & 0xff] = true;
                break;
            }

            case ButtonRelease: {
                XWindow * window;
                XFindContext(display, event.xbutton.window, window_ctx, (XPointer *)&window);
                window->input[0].keys[event.xbutton.button & 0xff] = false;
                break;
            }

            case ClientMessage: {
                XWindow * window;
                XFindContext(display, event.xclient.window, window_ctx, (XPointer *)&window);
                if ((Atom)event.xclient.data.l[0] == window->delete_message) {
                    XUnmapWindow(display, event.xclient.window);
                    window->input[0].visible = false;
                }
                break;
            }
        }
    }

    for (int i = 0; i < PyList_Size(window_list); ++i) {
        XWindow * wnd = (XWindow *)PyList_GetItem(window_list, i);
        memcpy(&wnd->input[2], &wnd->input[1], sizeof(Input));
        memcpy(&wnd->input[1], &wnd->input[0], sizeof(Input));

        int temp;
        unsigned mask;
        Window temp_window;
        XQueryPointer(display, wnd->window, &temp_window, &temp_window, &temp, &temp, &wnd->input[1].mouse_x, &wnd->input[1].mouse_y, &mask);

        if (wnd->grab_mouse[0]) {
            wnd->input[1].mouse_rx = wnd->input[1].mouse_x - wnd->width / 2;
            wnd->input[1].mouse_ry = wnd->input[1].mouse_y - wnd->height / 2;
            XWarpPointer(display, None, wnd->window, 0, 0, 0, 0, wnd->width / 2, wnd->height / 2);
        }
    }

    Py_RETURN_NONE;
}

PyObject * Window_meth_show(XWindow * self, PyObject * arg) {
    bool show = !!PyObject_IsTrue(arg);
    if (show && !self->input[1].visible) {
        XMapWindow(display, self->window);
    }
    if (!show && self->input[1].visible) {
        XUnmapWindow(display, self->window);
    }
    self->input[1].visible = show;
    self->input[0].visible = show;
    Py_RETURN_NONE;
}

PyObject * Window_meth_grab_mouse(XWindow * self, PyObject * grab) {
    self->grab_mouse[0] = !!PyObject_IsTrue(grab);
    if (self->grab_mouse[0] && !self->grab_mouse[1]) {
        int temp;
        unsigned mask;
        Window temp_window;
        XQueryPointer(display, self->window, &temp_window, &temp_window, &temp, &temp, &self->input[0].mouse_restore_x, &self->input[0].mouse_restore_y, &mask);
        XDefineCursor(display, self->window, self->hidden_cursor);
        XWarpPointer(display, None, self->window, 0, 0, 0, 0, self->width / 2, self->height / 2);
        self->input[1].mouse_rx = 0;
        self->input[1].mouse_ry = 0;
    }
    if (!self->grab_mouse[0] && self->grab_mouse[1]) {
        XUndefineCursor(display, self->window);
        XWarpPointer(display, None, self->window, 0, 0, 0, 0, self->input[0].mouse_restore_x, self->input[0].mouse_restore_y);
        XFlush(display);
    }
    self->grab_mouse[1] = self->grab_mouse[0];
    if (self->grab_mouse[0]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_pressed(XWindow * self, PyObject * key) {
    if (!PyUnicode_CheckExact(key)) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    key = PyDict_GetItem(keymap, key);
    if (!key) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    if (self->input[1].keys[PyLong_AsLong(key)] && !self->input[2].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_released(XWindow * self, PyObject * key) {
    if (!PyUnicode_CheckExact(key)) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    key = PyDict_GetItem(keymap, key);
    if (!key) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    if (!self->input[1].keys[PyLong_AsLong(key)] && self->input[2].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_down(XWindow * self, PyObject * key) {
    if (!PyUnicode_CheckExact(key)) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    key = PyDict_GetItem(keymap, key);
    if (!key) {
        PyErr_Format(PyExc_ValueError, "key");
        return NULL;
    }
    if (self->input[1].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_get_mouse(XWindow * self) {
    if (self->grab_mouse[0]) {
        return Py_BuildValue("iii", self->input[1].mouse_rx, self->input[1].mouse_ry, self->input[1].wheel);
    }
    return Py_BuildValue("iii", self->input[1].mouse_x, self->input[1].mouse_y, self->input[1].wheel);
}

PyObject * Window_get_visible(XWindow * self) {
    if (self->input[1].visible) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_get_size(XWindow * self) {
    return Py_BuildValue("ii", self->width, self->height);
}

PyObject * Window_get_handle(XWindow * self) {
    return Py_BuildValue("KK", display, self->window);
}

PyMethodDef module_methods[] = {
    {"window", (PyCFunction)glwindow_meth_window, METH_VARARGS | METH_KEYWORDS, NULL},
    {"update", (PyCFunction)glwindow_meth_update, METH_NOARGS, NULL},
    {},
};

void default_dealloc(PyObject * self) {
    Py_TYPE(self)->tp_free(self);
}

PyMethodDef Window_methods[] = {
    {"show", (PyCFunction)Window_meth_show, METH_O, NULL},
    {"grab_mouse", (PyCFunction)Window_meth_grab_mouse, METH_O, NULL},
    {"key_pressed", (PyCFunction)Window_meth_key_pressed, METH_O, NULL},
    {"key_released", (PyCFunction)Window_meth_key_released, METH_O, NULL},
    {"key_down", (PyCFunction)Window_meth_key_down, METH_O, NULL},
    {},
};

PyGetSetDef Window_getset[] = {
    {"mouse", (getter)Window_get_mouse, NULL, NULL, NULL},
    {"visible", (getter)Window_get_visible, NULL, NULL, NULL},
    {"size", (getter)Window_get_size, NULL, NULL, NULL},
    {"handle", (getter)Window_get_handle, NULL, NULL, NULL},
    {},
};

PyType_Slot Window_slots[] = {
    {Py_tp_methods, Window_methods},
    {Py_tp_getset, Window_getset},
    {Py_tp_dealloc, default_dealloc},
    {},
};

PyType_Spec Window_spec = {"mymodule.Window", sizeof(XWindow), 0, Py_TPFLAGS_DEFAULT, Window_slots};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "glwindow", NULL, -1, module_methods};

void add_key(const char * name, int code) {
    PyObject * keycode = PyLong_FromLong(code);
    PyDict_SetItemString(keymap, name, keycode);
    Py_DECREF(keycode);
}

void add_key(const char * name, const char * keysym) {
    PyObject * keycode = PyLong_FromLong(XKeysymToKeycode(display, XStringToKeysym(keysym)));
    PyDict_SetItemString(keymap, name, keycode);
    Py_DECREF(keycode);
}

extern "C" PyObject * PyInit_glwindow() {
    PyObject * module = PyModule_Create(&module_def);

    Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    PyModule_AddObject(module, "Window", (PyObject *)Window_type);

    display = XOpenDisplay(NULL);

    if (!display) {
        display = XOpenDisplay(":0");
    }

    if (!display) {
        return NULL;
    }

    window_ctx = XUniqueContext();

    window_list = PyList_New(0);
    keymap = PyDict_New();

    add_key("mouse1", 1);
    add_key("mouse2", 3);
    add_key("mouse3", 2);
    add_key("control", "Control_L");
    add_key("alt", "Alt_L");
    add_key("shift", "Shift_L");
    add_key("enter", "Return");
    add_key("space", "space");
    add_key("escape", "Escape");
    add_key("tab", "Tab");
    add_key("backspace", "BackSpace");
    add_key("up", "Up");
    add_key("down", "Down");
    add_key("left", "Left");
    add_key("right", "Right");
    add_key("plus", "equal");
    add_key("minus", "minus");
    add_key("slash", "slash");
    add_key("backslash", "backslash");
    add_key("period", "period");
    add_key("comma", "comma");
    add_key("tilde", "grave");
    add_key("insert", "Insert");
    add_key("delete", "Delete");
    add_key("home", "Home");
    add_key("end", "End");
    add_key("pageup", "Prior");
    add_key("pagedown", "Next");
    add_key("capslock", "Caps_Lock");
    add_key("f1", "F1");
    add_key("f2", "F2");
    add_key("f3", "F3");
    add_key("f4", "F4");
    add_key("f5", "F5");
    add_key("f6", "F6");
    add_key("f7", "F7");
    add_key("f8", "F8");
    add_key("f9", "F9");
    add_key("f10", "F10");
    add_key("f11", "F11");
    add_key("f12", "F12");

    for (char c = 'a'; c <= 'z'; ++c) {
        char name[2] = {c, 0};
        add_key(name, name);
    }

    for (char c = '0'; c <= '9'; ++c) {
        char name[2] = {c, 0};
        add_key(name, name);
    }

    PyModule_AddObject(module, "keymap", keymap);

    return module;
}
