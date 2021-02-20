#include <Python.h>

#include <Windows.h>

#define WM_USER_NEW_WINDOW (WM_USER + 0)
#define WM_USER_GRAB_MOUSE (WM_USER + 1)
#define WM_USER_SHOW_WINDOW (WM_USER + 2)

struct Input {
    bool keys[256];
    int mouse_rx;
    int mouse_ry;
    int mouse_x;
    int mouse_y;
    int wheel;
    bool visible;
    bool grab;
    POINT mouse_restore;
};

struct Window {
    PyObject_HEAD

    int width;
    int height;
    const char * title;

    Input input[3];
    bool grab_mouse[2];

    CRITICAL_SECTION lock;
    HWND hwnd;
};

PyTypeObject * Window_type;
PyObject * window_list;
PyObject * keymap;

HINSTANCE hinst;
HANDLE thread;
HANDLE ready;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    Window * wnd = hwnd ? (Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA) : NULL;
    if (!wnd) {
        if (umsg == WM_CREATE) {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
        }
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    switch (umsg) {
        case WM_CLOSE: {
            ShowWindow(hwnd, SW_HIDE);
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].visible = false;
            LeaveCriticalSection(&wnd->lock);
            return 0;
        }
        case WM_KEYDOWN: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[wparam & 0xff] = true;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_KEYUP: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[wparam & 0xff] = false;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_LBUTTONDOWN: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_LBUTTON] = true;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_LBUTTONUP: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_LBUTTON] = false;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_RBUTTONDOWN: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_RBUTTON] = true;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_RBUTTONUP: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_RBUTTON] = false;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_MBUTTONDOWN: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_MBUTTON] = true;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_MBUTTONUP: {
            EnterCriticalSection(&wnd->lock);
            wnd->input[0].keys[VK_MBUTTON] = false;
            LeaveCriticalSection(&wnd->lock);
            break;
        }
        case WM_MOUSEWHEEL: {
            wnd->input[0].wheel += (short)(wparam >> 16 & 0xffff) / 120;
            break;
        }
        case WM_MOUSEMOVE: {
            if (!wnd->input[0].grab) {
                wnd->input[0].mouse_x = lparam & 0xffff;
                wnd->input[0].mouse_y = lparam >> 16 & 0xffff;
            }
            break;
        }
        case WM_INPUT: {
            RAWINPUT raw;
            UINT size = sizeof(raw);
            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
            if (raw.header.dwType == RIM_TYPEMOUSE && !(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
                wnd->input[0].mouse_rx += raw.data.mouse.lLastX;
                wnd->input[0].mouse_ry += raw.data.mouse.lLastY;
            }
            break;
        }
        case WM_USER_SHOW_WINDOW: {
            ShowWindow(hwnd, !!lparam);
            wnd->input[0].visible = true;
            return 0;
        }
        case WM_USER_GRAB_MOUSE: {
            if (lparam) {
                RECT rect = {};
                GetClientRect(hwnd, &rect);
                ClientToScreen(hwnd, (POINT *)&rect.left);
                ClientToScreen(hwnd, (POINT *)&rect.right);
                GetCursorPos(&wnd->input[0].mouse_restore);
                ClipCursor(&rect);
            } else {
                SetCursorPos(wnd->input[0].mouse_restore.x, wnd->input[0].mouse_restore.y);
                ClipCursor(NULL);
            }
            wnd->input[0].grab = !!lparam;
            ShowCursor(!lparam);
            return 0;
        }
    }
    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void message_loop() {
    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    WNDCLASS wnd_class = {0, WindowProc, 0, 0, hinst, NULL, hcursor, NULL, NULL, "glwindow"};
    RegisterClass(&wnd_class);

    RAWINPUTDEVICE rid = {1, 2, 0, NULL};
    RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    SetEvent(ready);

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_USER_NEW_WINDOW) {
            Window * wnd = (Window *)msg.lParam;
            int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
            int sw = GetSystemMetrics(SM_CXSCREEN);
            int sh = GetSystemMetrics(SM_CYSCREEN);

            RECT rect = {};
            rect.right = wnd->width;
            rect.bottom = wnd->height;

            AdjustWindowRect(&rect, style, false);

            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            int x = (sw - w) / 2;
            int y = (sh - h) / 2;

            wnd->hwnd = CreateWindow("glwindow", wnd->title, style, x, y, w, h, NULL, NULL, hinst, wnd);

            SetEvent(ready);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

Window * glwindow_meth_window(PyObject * self, PyObject * vargs, PyObject * kwargs) {
    static char * keywords[] = {"size", "title", NULL};

    struct {
        int width = 1280;
        int height = 720;
        const char * title = NULL;
    } args;

    if (!PyArg_ParseTupleAndKeywords(vargs, kwargs, "|(II)z", keywords, &args.width, &args.height, &args.title)) {
        return NULL;
    }

    Window * res = PyObject_New(Window, Window_type);

    res->width = args.width;
    res->height = args.height;
    res->title = args.title;
    memset(res->input, 0, sizeof(res->input));
    memset(res->grab_mouse, 0, sizeof(res->grab_mouse));

    InitializeCriticalSection(&res->lock);
    PostThreadMessage(GetThreadId(thread), WM_USER_NEW_WINDOW, 0, (LPARAM)res);
    WaitForSingleObject(ready, INFINITE);

    PyList_Append(window_list, (PyObject *)res);
    return res;
}

PyObject * glwindow_meth_update(PyObject * self) {
    for (int i = 0; i < PyList_Size(window_list); ++i) {
        Window * wnd = (Window *)PyList_GetItem(window_list, i);
        EnterCriticalSection(&wnd->lock);
        memcpy(&wnd->input[2], &wnd->input[1], sizeof(Input));
        memcpy(&wnd->input[1], &wnd->input[0], sizeof(Input));
        wnd->input[0].mouse_rx = 0;
        wnd->input[0].mouse_ry = 0;
        wnd->input[0].wheel = 0;
        LeaveCriticalSection(&wnd->lock);
    }
    Py_RETURN_NONE;
}

PyObject * Window_meth_show(Window * self, PyObject * arg) {
    bool show = !!PyObject_IsTrue(arg);
    if (show && !self->input[1].visible) {
        SendMessage(self->hwnd, WM_USER_SHOW_WINDOW, 0, true);
        self->input[1].visible = true;
    }
    if (!show && self->input[1].visible) {
        SendMessage(self->hwnd, WM_USER_SHOW_WINDOW, 0, false);
        self->input[1].visible = false;
    }
    Py_RETURN_NONE;
}

PyObject * Window_meth_grab_mouse(Window * self, PyObject * grab) {
    self->grab_mouse[0] = !!PyObject_IsTrue(grab);
    if (self->grab_mouse[0] && !self->grab_mouse[1]) {
        SendMessage(self->hwnd, WM_USER_GRAB_MOUSE, 0, true);
    }
    if (!self->grab_mouse[0] && self->grab_mouse[1]) {
        SendMessage(self->hwnd, WM_USER_GRAB_MOUSE, 0, false);
    }
    self->grab_mouse[1] = self->grab_mouse[0];
    if (self->grab_mouse[0]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_pressed(Window * self, PyObject * key) {
    if (PyUnicode_CheckExact(key)) {
        key = PyDict_GetItem(keymap, key);
    }
    if (self->input[1].keys[PyLong_AsLong(key)] && !self->input[2].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_released(Window * self, PyObject * key) {
    if (PyUnicode_CheckExact(key)) {
        key = PyDict_GetItem(keymap, key);
    }
    if (!self->input[1].keys[PyLong_AsLong(key)] && self->input[2].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_meth_key_down(Window * self, PyObject * key) {
    if (PyUnicode_CheckExact(key)) {
        key = PyDict_GetItem(keymap, key);
    }
    if (self->input[1].keys[PyLong_AsLong(key)]) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_get_mouse(Window * self) {
    if (self->grab_mouse[0]) {
        return Py_BuildValue("iii", self->input[1].mouse_rx, self->input[1].mouse_ry, self->input[1].wheel);
    }
    return Py_BuildValue("iii", self->input[1].mouse_x, self->input[1].mouse_y, self->input[1].wheel);
}

PyObject * Window_get_visible(Window * self) {
    if (self->input[1].visible) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * Window_get_size(Window * self) {
    return Py_BuildValue("ii", self->width, self->height);
}

PyObject * Window_get_handle(Window * self) {
    return Py_BuildValue("KK", hinst, self->hwnd);
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

PyType_Spec Window_spec = {"mymodule.Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "glwindow", NULL, -1, module_methods};

void add_key(const char * name, int code) {
    PyObject * keycode = PyLong_FromLong(code);
    PyDict_SetItemString(keymap, name, keycode);
    Py_DECREF(keycode);
}

extern "C" PyObject * PyInit_glwindow() {
    PyObject * module = PyModule_Create(&module_def);

    Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    PyModule_AddObject(module, "Window", (PyObject *)Window_type);

    ready = CreateEvent(NULL, false, false, NULL);
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)message_loop, NULL, 0, NULL);
    WaitForSingleObject(ready, INFINITE);

    window_list = PyList_New(0);
    keymap = PyDict_New();

    add_key("mouse1", VK_LBUTTON);
    add_key("mouse2", VK_RBUTTON);
    add_key("mouse3", VK_MBUTTON);
    add_key("control", VK_CONTROL);
    add_key("alt", VK_MENU);
    add_key("shift", VK_SHIFT);
    add_key("enter", VK_RETURN);
    add_key("space", VK_SPACE);
    add_key("escape", VK_ESCAPE);
    add_key("tab", VK_TAB);
    add_key("backspace", VK_BACK);
    add_key("up", VK_UP);
    add_key("down", VK_DOWN);
    add_key("left", VK_LEFT);
    add_key("right", VK_RIGHT);
    add_key("plus", VK_OEM_PLUS);
    add_key("minus", VK_OEM_MINUS);
    add_key("slash", VK_OEM_2);
    add_key("backslash", VK_OEM_5);
    add_key("period", VK_OEM_PERIOD);
    add_key("comma", VK_OEM_COMMA);
    add_key("tilde", VK_OEM_3);
    add_key("insert", VK_INSERT);
    add_key("delete", VK_DELETE);
    add_key("home", VK_HOME);
    add_key("end", VK_END);
    add_key("pageup", VK_NEXT);
    add_key("pagedown", VK_END);
    add_key("capslock", VK_CAPITAL);
    add_key("f1", VK_F1);
    add_key("f2", VK_F2);
    add_key("f3", VK_F3);
    add_key("f4", VK_F4);
    add_key("f5", VK_F5);
    add_key("f6", VK_F6);
    add_key("f7", VK_F7);
    add_key("f8", VK_F8);
    add_key("f9", VK_F9);
    add_key("f10", VK_F10);
    add_key("f11", VK_F11);
    add_key("f12", VK_F12);

    for (char c = 'a'; c <= 'z'; ++c) {
        char name[2] = {c, 0};
        add_key(name, c - 32);
    }

    for (char c = '0'; c <= '9'; ++c) {
        char name[2] = {c, 0};
        add_key(name, c);
    }

    PyModule_AddObject(module, "keymap", keymap);

    return module;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        hinst = hinstDLL;
    }
    return true;
}
