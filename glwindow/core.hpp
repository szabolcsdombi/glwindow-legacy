#pragma once

struct RawData {
    int width, height;
    int glversion;
    bool key_down[110];
    bool old_key_down[110];
    wchar_t text_input[100];
    int text_input_size;
    double time;
    int mx, my, mw;
    int dmx, dmy;
    bool grab;
    bool old_grab;
    void * title;
    void * window;
};

extern bool create_window(void * arg);
extern bool update_window(void * arg);

