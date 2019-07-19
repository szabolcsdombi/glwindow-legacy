import glwindow
import moderngl as mgl
import numpy as np

wnd = glwindow.window((840, 480))
ctx = mgl.create_context()

prog = ctx.program(
    vertex_shader='''
        #version 140
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
    ''',
    fragment_shader='''
        #version 140
        in vec4 frag_color;
        out vec4 color;
        void main() {
            color = vec4(frag_color);
        }
    ''',
)

scale = prog['scale']
rotation = prog['rotation']

scale.value = (0.5, 16/9 * 0.5)

vertices = np.array([
    1.0, 0.0,
    1.0, 0.0, 0.0, 0.5,

    -0.5, 0.86,
    0.0, 1.0, 0.0, 0.5,

    -0.5, -0.86,
    0.0, 0.0, 1.0, 0.5,
])

vbo = ctx.buffer(vertices.astype('f4').tobytes())
vao = ctx.simple_vertex_array(prog, vbo, 'vert', 'vert_color')

while wnd.update():
    ctx.clear(1.0, 1.0, 1.0)
    wnd.grab_mouse(wnd.key_down('mouse1'))
    if wnd.key_down('mouse1'):
        ctx.clear(0.0, 0.0, 0.0)
    ctx.enable(mgl.BLEND)
    rotation.value = wnd.time
    vao.render(instances=10)
