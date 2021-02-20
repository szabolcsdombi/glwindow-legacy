import glnext
import glwindow
import miniglm
from glnext_compiler import glsl

window1 = glwindow.window((512, 512), '8 bits')
window2 = glwindow.window((512, 512), '10 bits')

instance = glnext.instance(surface=True)

framebuffer1 = instance.framebuffer((512, 512), '4p')
framebuffer2 = instance.framebuffer((512, 512), '4h')

uniform_buffer = instance.buffer('uniform_buffer', 8)

for fbo in [framebuffer1, framebuffer2]:
    fbo.render(
        vertex_shader=glsl('''
            #version 450
            #pragma shader_stage(vertex)

            layout (binding = 0) uniform Buffer {
                float rotation;
                float scale;
            };

            layout (location = 0) out vec4 out_color;

            vec2 positions[3] = vec2[](
                vec2(-0.43, -0.25),
                vec2(0.43, -0.25),
                vec2(0.0, 0.5)
            );

            vec4 colors[3] = vec4[](
                vec4(1.0, 0.0, 0.0, 1.0),
                vec4(0.0, 1.0, 0.0, 1.0),
                vec4(0.0, 0.0, 1.0, 1.0)
            );

            void main() {
                mat2 rot = mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation));
                gl_Position = vec4(rot * positions[gl_VertexIndex] * scale, 0.0, 1.0);
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
        bindings=[
            {
                'binding': 0,
                'type': 'uniform_buffer',
                'buffer': uniform_buffer,
            }
        ]
    )

staging = instance.staging([
    {
        'offset': 0,
        'type': 'input_buffer',
        'buffer': uniform_buffer,
    }
])

instance.surface(window1.handle, framebuffer1.output[0])
instance.surface(window2.handle, framebuffer2.output[0])

rotation = 0.0
scale = 1.0

window1.show(True)
window2.show(True)

while window1.visible and window2.visible:
    glwindow.update()

    for wnd in [window1, window2]:
        mx, my, mw = wnd.mouse
        scale *= 1.1 ** mw
        if wnd.grab_mouse(wnd.key_down('mouse1')):
            mx, my, mw = wnd.mouse
            rotation += mx * 0.01

    staging.mem[:] = glnext.pack([rotation, scale])
    instance.run()
