#include <Python.h>

#define UNICODE
#include <Windows.h>

#include "core.hpp"

#define WGL_CONTEXT_PROFILE_MASK 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT 0x0001
#define WGL_CONTEXT_MAJOR_VERSION 0x2091
#define WGL_CONTEXT_MINOR_VERSION 0x2092

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int * attribList);
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);

HINSTANCE hinst;

struct MyWindow {
    CRITICAL_SECTION lock;
    HANDLE ready;

    HWND hwnd;
    HDC hdc;
    HGLRC hrc;

    bool key_down[110];
    wchar_t text_input[100];
    int text_input_size;
    int mouse_wheel;

    long long freq;
    long long start;
    bool alive;
};

MyWindow * window;

PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER, 0, 24};

int keyid(int code) {
    if ('A' <= code && code <= 'Z') {
        return 27 + code - 'A';
    }
    if ('0' <= code && code <= '9') {
        return 53 + code - '0';
    }
    if (VK_F1 <= code && code <= VK_F12) {
        return 63 + code - VK_F1;
    }
    switch (code) {
        case VK_CONTROL: return 1;
        case VK_MENU: return 2;
        case VK_SHIFT: return 3;
        case VK_RETURN: return 4;
        case VK_SPACE: return 5;
        case VK_ESCAPE: return 6;
        case VK_TAB: return 7;
        case VK_BACK: return 8;
        case VK_UP: return 9;
        case VK_DOWN: return 10;
        case VK_LEFT: return 11;
        case VK_RIGHT: return 12;
        case VK_OEM_PLUS: return 13;
        case VK_OEM_MINUS: return 14;
        case VK_OEM_2: return 15;
        case VK_OEM_5: return 16;
        case VK_OEM_PERIOD: return 17;
        case VK_OEM_COMMA: return 18;
        case VK_OEM_3: return 19;
        case VK_INSERT: return 20;
        case VK_DELETE: return 21;
        case VK_HOME: return 22;
        case VK_END: return 23;
        case VK_PRIOR: return 24;
        case VK_NEXT: return 25;
        case VK_CAPITAL: return 26;
        default: return 0;
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE: {
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY: {
            EnterCriticalSection(&window->lock);
            window->alive = false;
            LeaveCriticalSection(&window->lock);
            PostQuitMessage(0);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            EnterCriticalSection(&window->lock);
            window->key_down[101] = true;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_LBUTTONUP: {
            EnterCriticalSection(&window->lock);
            window->key_down[101] = false;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_RBUTTONDOWN: {
            EnterCriticalSection(&window->lock);
            window->key_down[102] = true;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_RBUTTONUP: {
            EnterCriticalSection(&window->lock);
            window->key_down[102] = false;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_MBUTTONDOWN: {
            EnterCriticalSection(&window->lock);
            window->key_down[103] = true;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_MBUTTONUP: {
            EnterCriticalSection(&window->lock);
            window->key_down[103] = false;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_MOUSEWHEEL: {
            EnterCriticalSection(&window->lock);
            window->mouse_wheel += GET_WHEEL_DELTA_WPARAM(wParam);
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_KEYDOWN: {
            EnterCriticalSection(&window->lock);
            window->key_down[keyid((int)wParam)] = true;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_KEYUP: {
            EnterCriticalSection(&window->lock);
            window->key_down[keyid((int)wParam)] = false;
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_CHAR: {
            EnterCriticalSection(&window->lock);
            if (window->text_input_size < 100) {
                window->text_input[window->text_input_size++] = (wchar_t)wParam;
            }
            LeaveCriticalSection(&window->lock);
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            EnterCriticalSection(&window->lock);
            static bool sys_alt = false;
            if (wParam == VK_MENU) {
                sys_alt = (uMsg == WM_SYSKEYDOWN);
            } else if (sys_alt && uMsg == WM_SYSKEYDOWN && wParam == VK_F4) {
                DestroyWindow(hWnd);
            }
            LeaveCriticalSection(&window->lock);
            return 0;
        }
        case WM_USER: {
            ShowCursor(!wParam);
            return 0;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void window_thread(void * arg) {
    RawData * data = (RawData *)arg;

    if (!hinst) {
        return;
    }

    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    HCURSOR hicon1 = (HICON)LoadIcon(hinst, MAKEINTRESOURCE(10001));
    HCURSOR hicon2 = (HICON)LoadIcon(hinst, MAKEINTRESOURCE(10002));

    WNDCLASSEXW wnd_class = {sizeof(WNDCLASSEXW), CS_OWNDC, WindowProc, 0, 0, hinst, hicon1, hcursor, NULL, NULL, L"glwindow", hicon2};

    if (!RegisterClassEx(&wnd_class)) {
        return;
    }

    int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    RECT rect = {};
    rect.right = data->width;
    rect.bottom = data->height;

    AdjustWindowRect(&rect, style, false);

    int adjusted_width = rect.right - rect.left;
    int adjusted_height = rect.bottom - rect.top;

    int x = (sw - adjusted_width) / 2;
    int y = (sh - adjusted_height) / 2;

    window->hwnd = CreateWindowEx(WS_EX_APPWINDOW, L"glwindow", (LPCWSTR)data->title, style, x, y, data->width, data->height, NULL, NULL, hinst, data);

    if (!window->hwnd) {
        return;
    }

    window->hdc = GetDC(window->hwnd);

    if (!window->hdc) {
        return;
    }

    int pixelformat = ChoosePixelFormat(window->hdc, &pfd);
    if (!pixelformat) {
        return;
    }

    if (!SetPixelFormat(window->hdc, pixelformat, &pfd)) {
        return;
    }

   window->hrc = wglCreateContext(window->hdc);
    if (!window->hrc) {
        return;
    }

    if (data->glversion) {
        if (!wglMakeCurrent(window->hdc, window->hrc)) {
            return;
        }

        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        if (!wglCreateContextAttribsARB) {
            return;
        }

        if (!wglMakeCurrent(NULL, NULL)) {
            return;
        }

        if (!wglDeleteContext(window->hrc)) {
            return;
        }

        int attribs[] = {
            WGL_CONTEXT_PROFILE_MASK, WGL_CONTEXT_CORE_PROFILE_BIT,
            WGL_CONTEXT_MAJOR_VERSION, data->glversion / 100 % 10,
            WGL_CONTEXT_MINOR_VERSION, data->glversion / 10 % 10,
            0, 0,
        };

        window->hrc = wglCreateContextAttribsARB(window->hdc, NULL, attribs);
    }

    if (!window->hrc) {
        return;
    }

    data->window = window;
    SetEvent(window->ready);

    MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


bool create_window(void * arg) {
    window = new MyWindow();

    window->ready = CreateEvent(NULL, true, false, NULL);
    InitializeCriticalSection(&window->lock);
    window->alive = true;

    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)window_thread, arg, 0, NULL);

    HANDLE objects[2] = {
        window->ready,
        thread,
    };

    WaitForMultipleObjects(2, objects, false, INFINITE);

    if (WaitForSingleObject(window->ready, 0)) {
        PyErr_BadInternalCall();
        return false;
    }

    if (!wglMakeCurrent(window->hdc, window->hrc)) {
        PyErr_BadInternalCall();
        return false;
    }

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (!wglSwapIntervalEXT) {
        PyErr_BadInternalCall();
        return false;
    }

    wglSwapIntervalEXT(1);

    QueryPerformanceFrequency((LARGE_INTEGER *)&window->freq);
    QueryPerformanceCounter((LARGE_INTEGER *)&window->start);

    ShowWindow(window->hwnd, SW_SHOW);
	SetForegroundWindow(window->hwnd);
	SetActiveWindow(window->hwnd);
	SetFocus(window->hwnd);
    return true;
}

bool update_window(void * arg) {
    RawData * data = (RawData *)arg;

    EnterCriticalSection(&window->lock);

    if (!window->alive) {
        LeaveCriticalSection(&window->lock);
        return false;
    }

    SwapBuffers(window->hdc);

    if (data->grab != data->old_grab) {
        if (data->grab) {
            POINT point;
            GetCursorPos(&point);
            data->mx = point.x;
            data->my = point.y;
        } else {
            SetCursorPos(data->mx, data->my);
        }
    }

    memcpy(data->key_down, window->key_down, 110);
    memcpy(data->text_input, window->text_input, window->text_input_size * 2);
    data->text_input_size = window->text_input_size;
    data->mw = window->mouse_wheel;

    LeaveCriticalSection(&window->lock);

    if (data->grab != data->old_grab) {
        SendMessage(window->hwnd, WM_USER, data->grab, 0);
    }

    POINT point;
    GetCursorPos(&point);

    RECT rect;
    GetWindowRect(window->hwnd, &rect);

    if (data->grab) {
        SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
        data->dmx = point.x - (rect.left + rect.right) / 2;
        data->dmy = point.y - (rect.top + rect.bottom) / 2;
    } else {
        data->mx = point.x - rect.left;
        data->my = point.y - rect.top;
    }

    long long now;
    QueryPerformanceCounter((LARGE_INTEGER *)&now);
    data->time = (double)(now - window->start) / window->freq;
    return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        hinst = hinstDLL;
    }
    return true;
}
