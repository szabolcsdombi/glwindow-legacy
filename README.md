# glwindow

```
pip install glwindow
```

- [Documentation](https://glwindow.readthedocs.io/)
- [glwindow on Github](https://github.com/glnext/glwindow/)
- [glwindow on PyPI](https://pypi.org/project/glwindow/)

A simple window with **your** own main loop.

## Examples

```py
import glwindow

window = glwindow.window()

while window.visible:
    glwindow.update()
```

**Multiple Windows**

```py
import glwindow

windows = [
    glwindow.window((640, 480)),
    glwindow.window((640, 480)),
    glwindow.window((640, 480)),
]

while any(x.visible for x in windows):
    glwindow.update()
```

**Keyboard Input**

```py
import glwindow

window = glwindow.window()

while window.visible:
    glwindow.update()
    if window.key_pressed('escape'):
        break
```

**Vulkan Surface**

```py
import glnext
import glwindow
from glnext_compiler import glsl

instance = glnext.instance(surface=True)
task = instance.task()

size = (512, 512)
framebuffer = task.framebuffer(size)

pipeline = framebuffer.render(
    vertex_shader=glsl('''
        #version 450
        #pragma shader_stage(vertex)

        layout (location = 0) out vec4 out_color;

        vec2 positions[3] = vec2[](
            vec2(-0.5, -0.5),
            vec2(0.5, -0.5),
            vec2(0.0, 0.5)
        );

        vec4 colors[3] = vec4[](
            vec4(1.0, 0.0, 0.0, 1.0),
            vec4(0.0, 1.0, 0.0, 1.0),
            vec4(0.0, 0.0, 1.0, 1.0)
        );

        void main() {
            gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
            out_color = colors[gl_VertexIndex];
            gl_Position.y *= -1.0;
        }
    '''),
    fragment_shader=glsl('''
        #version 450
        #pragma shader_stage(fragment)

        layout (location = 0) in vec4 in_color;
        layout (location = 0) out vec4 out_color;

        void main() {
            out_color = in_color;
        }
    '''),
    vertex_count=3,
)

wnd = glwindow.window(size, title='Hello World!')

instance.surface(wnd.handle, framebuffer.output[0])

while wnd.visible:
    glwindow.update()
    task.run()
    instance.present()
```
