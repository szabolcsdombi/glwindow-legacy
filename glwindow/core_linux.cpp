#include <Python.h>

#include <X11/Xatom.h>
#include <GL/glx.h>
#include <GL/gl.h>

#include "core.hpp"

#define GLX_CONTEXT_MAJOR_VERSION 0x2091
#define GLX_CONTEXT_MINOR_VERSION 0x2092
#define GLX_CONTEXT_PROFILE_MASK 0x9126
#define GLX_CONTEXT_CORE_PROFILE_BIT 0x0001

typedef GLXContext (* GLXCREATECONTEXTATTRIBSARBPROC)(Display * display, GLXFBConfig config, GLXContext context, Bool direct, const int * attribs);

void get_cursor_pos(Display * display, Window window, int * mx, int * my) {
    int temp;
    unsigned mask;
    Window temp_window;
    XQueryPointer(display, window, &temp_window, &temp_window, &temp, &temp, mx, my, &mask);
}

char empty_icon[1024] = {};

struct MyWindow {
    Display * display;
    Window window;
    GLXContext context;
    Atom delete_message;
    Cursor hidden_cursor;
    int events;
};

int keyid(int code) {
	switch (code) {
        case 37: return 1;
        case 64: return 2;
        case 50: return 3;
        case 36: return 4;
        case 65: return 5;
        case 9: return 6;
        case 23: return 7;
        case 22: return 8;
        case 111: return 9;
        case 116: return 10;
        case 113: return 11;
        case 114: return 12;
        case 21: return 13;
        case 20: return 14;
        case 61: return 15;
        case 51: return 16;
        case 60: return 17;
        case 59: return 18;
        case 49: return 19;
        case 118: return 20;
        case 119: return 21;
        case 110: return 22;
        case 115: return 23;
        case 112: return 24;
        case 117: return 25;
        case 66: return 26;

        case 38: return 27;
        case 56: return 28;
        case 54: return 29;
        case 40: return 30;
        case 26: return 31;
        case 41: return 32;
        case 42: return 33;
        case 43: return 34;
        case 31: return 35;
        case 44: return 36;
        case 45: return 37;
        case 46: return 38;
        case 58: return 39;
        case 57: return 40;
        case 32: return 41;
        case 33: return 42;
        case 24: return 43;
        case 27: return 44;
        case 39: return 45;
        case 28: return 46;
        case 30: return 47;
        case 55: return 48;
        case 25: return 49;
        case 53: return 50;
        case 29: return 51;
        case 52: return 52;

        case 19: return 53;
        case 10: return 54;
        case 11: return 55;
        case 12: return 56;
        case 13: return 57;
        case 14: return 58;
        case 15: return 59;
        case 16: return 60;
        case 17: return 61;
        case 18: return 62;

        case 67: return 63;
        case 68: return 64;
        case 69: return 65;
        case 70: return 66;
        case 71: return 67;
        case 72: return 68;
        case 73: return 69;
        case 74: return 70;
        case 75: return 71;
        case 76: return 72;
        case 95: return 73;
        case 96: return 74;

		default: return 0;
	}
}

bool create_window(void * arg) {
    RawData * data = (RawData *)arg;
    MyWindow * window = new MyWindow();

    window->display = XOpenDisplay(NULL);

    if (!window->display) {
        PyErr_BadInternalCall();
        return false;
    }

    int glx_major, glx_minor;
    glXQueryVersion(window->display, &glx_major, &glx_minor);

    int elements = 0;

    int glx_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None,
    };

    GLXFBConfig * fbc = glXChooseFBConfig(window->display, DefaultScreen(window->display), glx_attribs, &elements);

    if (!fbc) {
        XCloseDisplay(window->display);
        PyErr_BadInternalCall();
        return false;
    }

    static int attribute_list[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        None,
    };

    XVisualInfo * vi = glXChooseVisual(window->display, DefaultScreen(window->display), attribute_list);

    if (!vi) {
        XCloseDisplay(window->display);
        PyErr_BadInternalCall();
        return false;
    }

    Window root_window = RootWindow(window->display, vi->screen);

    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(window->display, root_window, vi->visual, AllocNone);
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;

    window->window = XCreateWindow(window->display, root_window, 0, 0, data->width, data->height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

    if (!window->window) {
        XCloseDisplay(window->display);
        PyErr_BadInternalCall();
        return false;
    }

    XSizeHints * sh = XAllocSizeHints();
    sh->flags = PMinSize | PMaxSize;
    sh->min_width = data->width;
    sh->max_width = data->width;
    sh->min_height = data->height;
    sh->max_height = data->height;
    XSetWMSizeHints(window->display, window->window, sh, XA_WM_NORMAL_HINTS);
    XFree(sh);

    XStoreName(window->display, window->window, "glwindow");
    XMapWindow(window->display, window->window);

    GLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = (GLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

    if (!glXCreateContextAttribsARB) {
        PyErr_BadInternalCall();
        return false;
    }

    int attribs[] = {
        GLX_CONTEXT_PROFILE_MASK, GLX_CONTEXT_CORE_PROFILE_BIT,
        GLX_CONTEXT_MAJOR_VERSION, data->glversion / 100 % 10,
        GLX_CONTEXT_MINOR_VERSION, data->glversion / 10 % 10,
        None,
    };

    window->context = glXCreateContextAttribsARB(window->display, fbc[0], NULL, true, attribs);

    if (!window->context) {
        window->context = glXCreateContext(window->display, vi, NULL, true);
    }

    XSync(window->display, False);

    if (!window->context) {
        XDestroyWindow(window->display, window->window);
        XCloseDisplay(window->display);
        PyErr_BadInternalCall();
        return false;
    }

    int make_current = glXMakeCurrent(window->display, window->window, window->context);

    if (!make_current) {
        glXDestroyContext(window->display, window->context);
        XDestroyWindow(window->display, window->window);
        XCloseDisplay(window->display);
        PyErr_BadInternalCall();
        return false;
    }

    window->events = ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | StructureNotifyMask;
    XSelectInput(window->display, window->window, window->events);

    window->delete_message = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(window->display, window->window, &window->delete_message, 1);

    XColor black = {};
    Pixmap empty_bitmap = XCreateBitmapFromData(window->display, window->window, empty_icon, 8, 8);
    window->hidden_cursor = XCreatePixmapCursor(window->display, empty_bitmap, empty_bitmap, &black, &black, 0, 0);

    data->window = window;
    return true;
}

bool update_window(void * arg) {
    RawData * data = (RawData *)arg;
    MyWindow * window = (MyWindow *)data->window;

	if (data->grab != data->old_grab) {
		if (data->grab) {
            int temp;
            unsigned mask;
            Window temp_window;
            XQueryPointer(window->display, window->window, &temp_window, &temp_window, &temp, &temp, &data->mx, &data->my, &mask);
            XDefineCursor(window->display, window->window, window->hidden_cursor);
            XWarpPointer(window->display, None, window->window, 0, 0, 0, 0, data->width / 2, data->height / 2);
        } else {
            XUndefineCursor(window->display, window->window);
            XWarpPointer(window->display, None, window->window, 0, 0, 0, 0, data->mx, data->my);
            XFlush(window->display);
        }
    }

    if (data->grab) {
        XWarpPointer(window->display, None, window->window, 0, 0, 0, 0, data->width / 2, data->height / 2);
    }

    bool alive = true;
    glXSwapBuffers(window->display, window->window);

    XEvent event;
    while (XPending(window->display)) {
        XNextEvent(window->display, &event);
        if (XFilterEvent(&event, None)) {
            continue;
        }

        switch (event.type) {
            case KeyPress:
                data->key_down[keyid(((XKeyEvent *)&event)->keycode)] = true;
                break;

            case KeyRelease:
                data->key_down[keyid(((XKeyEvent *)&event)->keycode)] = false;
                break;

            case ButtonPress:
                switch (((XButtonEvent *)&event)->button) {
                    case 1: data->key_down[101] = true; break;
                    case 2: data->key_down[103] = true; break;
                    case 3: data->key_down[102] = true; break;
                }
                break;

            case ButtonRelease:
                switch (((XButtonEvent *)&event)->button) {
                    case 1: data->key_down[101] = false; break;
                    case 2: data->key_down[103] = false; break;
                    case 3: data->key_down[102] = false; break;
                }
                break;

            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == window->delete_message) {
                    alive = false;
                }
                break;
        }
    }

    int mx, my;
    int temp;
    unsigned mask;
    Window temp_window;
    XQueryPointer(window->display, window->window, &temp_window, &temp_window, &temp, &temp, &mx, &my, &mask);

	if (data->grab) {
		data->dmx = mx - data->width / 2;
		data->dmy = my - data->height / 2;
	} else {
		data->mx = mx;
		data->my = my;
	}

    return alive;
}
