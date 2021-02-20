import glnext
import glwindow
from glnext_compiler import glsl
from miniglm import add, cross, mul, norm, rotate, sub


class Camera:
    def __init__(self, position, target, fov, aspect):
        self.position = position
        self.target = target
        self.fov = fov
        self.aspect = aspect

    def turn(self, horizontal, vertical):
        forward = sub(self.target, self.position)
        forward = mul(rotate(-horizontal, (0.0, 0.0, 1.0)), forward)
        forward = mul(rotate(-vertical, norm(cross(forward, (0.0, 0.0, 1.0)))), forward)
        self.target = add(self.position, forward)

    def rotate(self, horizontal, vertical):
        forward = sub(self.target, self.position)
        forward = mul(rotate(-horizontal, (0.0, 0.0, 1.0)), forward)
        forward = mul(rotate(-vertical, norm(cross(forward, (0.0, 0.0, 1.0)))), forward)
        self.position = sub(self.target, forward)

    def move(self, local, z):
        f, s, u = local
        forward = norm(sub(self.target, self.position))
        sideward = norm(cross((0.0, 0.0, 1.0), forward))
        upward = norm(cross(forward, sideward))
        move = add(add(add(mul(forward, f), mul(sideward, s)), mul(upward, u)), (0.0, 0.0, z))
        self.position = add(self.position, move)
        self.target = add(self.target, move)

    def zoom(self, level):
        self.position = add(self.target, mul(sub(self.position, self.target), 0.9 ** level))

    def lens(self, level):
        self.fov *= 0.975 ** level

    def pack(self):
        return glnext.camera(self.position, self.target, fov=self.fov, aspect=self.aspect)


cube_mesh = glnext.pack([
    -1.0, -1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 1.0, 0.0, 0.0, 1.0,
    -1.0, 1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    1.0, 1.0, -1.0, 1.0, 0.0, 0.0,
    1.0, -1.0, 1.0, 0.0, -1.0, 0.0,
    -1.0, -1.0, -1.0, 0.0, -1.0, 0.0,
    1.0, -1.0, -1.0, 0.0, -1.0, 0.0,
    -1.0, 1.0, -1.0, 0.0, 0.0, -1.0,
    1.0, -1.0, -1.0, 0.0, 0.0, -1.0,
    -1.0, -1.0, -1.0, 0.0, 0.0, -1.0,
    -1.0, 1.0, 1.0, 0.0, 1.0, 0.0,
    1.0, 1.0, -1.0, 0.0, 1.0, 0.0,
    -1.0, 1.0, -1.0, 0.0, 1.0, 0.0,
    -1.0, -1.0, 1.0, -1.0, 0.0, 0.0,
    -1.0, 1.0, -1.0, -1.0, 0.0, 0.0,
    -1.0, -1.0, -1.0, -1.0, 0.0, 0.0,
    -1.0, -1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, -1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, -1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    1.0, -1.0, 1.0, 0.0, -1.0, 0.0,
    -1.0, -1.0, 1.0, 0.0, -1.0, 0.0,
    -1.0, -1.0, -1.0, 0.0, -1.0, 0.0,
    -1.0, 1.0, -1.0, 0.0, 0.0, -1.0,
    1.0, 1.0, -1.0, 0.0, 0.0, -1.0,
    1.0, -1.0, -1.0, 0.0, 0.0, -1.0,
    -1.0, 1.0, 1.0, 0.0, 1.0, 0.0,
    1.0, 1.0, 1.0, 0.0, 1.0, 0.0,
    1.0, 1.0, -1.0, 0.0, 1.0, 0.0,
    -1.0, -1.0, 1.0, -1.0, 0.0, 0.0,
    -1.0, 1.0, 1.0, -1.0, 0.0, 0.0,
    -1.0, 1.0, -1.0, -1.0, 0.0, 0.0,
])

camera = Camera((4.0, 3.0, 2.0), (0.0, 0.0, 0.0), fov=60.0, aspect=1.777)

wnd = glwindow.window()

instance = glnext.instance(surface=True)
fbo = instance.framebuffer(wnd.size)

ubo = instance.buffer('uniform_buffer', 64)

pipeline = fbo.render(
    vertex_shader=glsl('''
        #version 450
        #pragma shader_stage(vertex)

        layout (binding = 0) uniform Buffer {
            mat4 mvp;
        };

        layout (location = 0) in vec3 in_vert;
        layout (location = 1) in vec3 in_norm;

        layout (location = 0) out vec3 out_norm;

        void main() {
            gl_Position = mvp * vec4(in_vert, 1.0);
            gl_Position.y *= -1.0;
            out_norm = in_norm;
        }
    '''),
    fragment_shader=glsl('''
        #version 450
        #pragma shader_stage(fragment)

        layout (binding = 0) uniform Buffer {
            mat4 mvp;
        };

        layout (location = 0) in vec3 in_norm;

        layout (location = 0) out vec4 out_color;

        void main() {
            vec3 color = vec3(1.0, 1.0, 1.0);
            vec3 sight = -vec3(mvp[0].w, mvp[1].w, mvp[2].w);
            float lum = dot(normalize(sight), normalize(in_norm)) * 0.7 + 0.3;
            out_color = vec4(lum, lum, lum, 1.0);
        }
    '''),
    vertex_format='3f 3f',
    vertex_count=36,
    bindings=[
        {
            'binding': 0,
            'type': 'uniform_buffer',
            'buffer': ubo,
        },
    ],
)

pipeline.update(vertex_buffer=cube_mesh)

instance.surface(wnd.handle, fbo.output[0])
wnd.show(True)

fly = False

while wnd.visible:
    wheel = wnd.mouse[2]
    glwindow.update()
    if fly:
        mx, my = wnd.mouse[:2]
        camera.turn(mx * 0.005, my * 0.005)
        camera.lens(wheel)
        speed = 0.2 if wnd.key_down('shift') else 0.05
        if wnd.key_down('w'):
            camera.move((speed, 0.0, 0.0), 0.0)
        if wnd.key_down('s'):
            camera.move((-speed, 0.0, 0.0), 0.0)
        if wnd.key_down('a'):
            camera.move((0.0, speed, 0.0), 0.0)
        if wnd.key_down('d'):
            camera.move((0.0, -speed, 0.0), 0.0)
        if wnd.key_down('q'):
            camera.move((0.0, 0.0, 0.0), -speed)
        if wnd.key_down('e'):
            camera.move((0.0, 0.0, 0.0), speed)
        if wnd.key_pressed('mouse1'):
            wnd.grab_mouse(False)
            fly = False
    else:
        camera.zoom(wheel)
        if wnd.key_down('shift') and wnd.grab_mouse(wnd.key_down('mouse3')):
            mx, my = wnd.mouse[:2]
            camera.move((0.0, mx * 0.01, my * 0.01), 0.0)
        if not wnd.key_down('shift') and wnd.grab_mouse(wnd.key_down('mouse3')):
            mx, my = wnd.mouse[:2]
            camera.rotate(mx * 0.01, my * 0.01)
        if wnd.key_down('control') and wnd.key_pressed('tilde'):
            wnd.grab_mouse(True)
            fly = True
    ubo.write(camera.pack())
    instance.run()
