import math

import glwindow
import moderngl as mgl

wnd = glwindow.window((840, 480))
ctx = mgl.create_context()

while wnd.update():
    ctx.clear(math.sin(wnd.time) * 0.5 + 0.5, 0.5, 0.0)
