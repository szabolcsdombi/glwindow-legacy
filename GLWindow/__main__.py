'''
    Sample program to test GLWindow.
'''

import struct

import GLWindow
import ModernGL


def main():
    '''
        Sample program to test GLWindow.
    '''

    wnd = GLWindow.create_window(480, 480, title='GLWindow Sample')
    ctx = ModernGL.create_context()

    prog = ctx.program([
        ctx.vertex_shader('''
            #version 330
            in vec2 vert;
            in vec4 vert_color;
            out vec4 frag_color;
            uniform vec2 scale;
            uniform float rotation;
            void main() {
                frag_color = vert_color;
                float r = rotation * (0.5 + gl_InstanceID * 0.05);
                mat2 rot = mat2(cos(r), sin(r), -sin(r), cos(r));
                gl_Position = vec4((rot * vert) * scale, 0.0, 1.0);
            }
        '''),
        ctx.fragment_shader('''
            #version 330
            in vec4 frag_color;
            out vec4 color;
            void main() {
                color = vec4(frag_color);
            }
        '''),
    ])

    scale = prog.uniforms['scale']
    rotation = prog.uniforms['rotation']

    vbo = ctx.buffer(struct.pack(
        '18f',
        1.0, 0.0, 1.0, 0.0, 0.0, 0.5,
        -0.5, 0.86, 0.0, 1.0, 0.0, 0.5,
        -0.5, -0.86, 0.0, 0.0, 1.0, 0.5,
    ))

    vao = ctx.simple_vertex_array(prog, vbo, ['vert', 'vert_color'])

    while wnd.update():
        width, height = wnd.size
        scale.value = (height / width * 0.75, 0.75)
        ctx.viewport = wnd.viewport
        ctx.clear(240, 240, 240)
        ctx.enable(ModernGL.BLEND)
        rotation.value = wnd.time
        vao.render(instances=10)


if __name__ == '__main__':
    main()
