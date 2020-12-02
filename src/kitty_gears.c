/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * === History ===
 *
 * Michael Clark
 *   - Conversion from fixed function to programmable shaders
 *   - Conversion to OSMesa and kitty
 *
 * Marcus Geelnard:
 *   - Conversion to GLFW
 *   - Time based rendering (frame rate independent)
 *   - Slightly modified camera that should work better for stereo viewing
 *
 * Camilla Löwy:
 *   - Removed FPS counter (this is not a benchmark)
 *   - Added a few comments
 *   - Enabled vsync
 *
 * Brian Paul
 *   - Orignal version
 */

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include "GL/osmesa.h"

#include "linmath.h"
#include "gl2_util.h"
#include "kitty_util.h"

static const char* frag_shader_filename = "shaders/gears.fsh";
static const char* vert_shader_filename = "shaders/gears.vsh";

static uint width = 256, height = 256;
static uint count = 1000;
static uint help = 0;
static uint millis = 10;
static uint animation = 1;
static uint running = 1;
static uint statistics = 0;
static uint compression = 0;
static size_t bytes_rendered = 0;
static size_t bytes_transferred = 0;

static GLfloat view_dist = -40.0f;
static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLfloat angle = 0.f;

static GLuint program;
static GLuint vao[3], vbo[3], ibo[3];
static vertex_buffer vb[3];
static index_buffer ib[3];
static mat4x4 gm[3], m, v, p, mvp;

/*
 * Create a gear wheel.
 *
 *   Input:  inner_radius - radius of hole at center
 *                  outer_radius - radius at center of teeth
 *                  width - width of gear teeth - number of teeth
 *                  tooth_depth - depth of tooth
 */

#define vertex_3f(x,y,z,u,v) \
    vertex_buffer_add(vb, (vertex){{x,y,z}, norm, {u,v}, col })
#define normal_3f(x,y,z) norm = (vec3f){x,y,z}

static inline void normalize2f(float v[2])
{
    float len = sqrtf(v[0]*v[0] + v[1]*v[1]);
    v[0] /= len;
    v[1] /= len;
}

static void gear(vertex_buffer *vb, index_buffer *ib,
                 float inner_radius, float outer_radius, float width,
                 int teeth, float tooth_depth, vec4f col)
{
    int i;
    float r0, r1, r2, nr0, nr1, nr2;
    float angle, da, ca0, sa0, ca1, sa1, ca2, sa2, ca3, sa3, ca4, sa4;
    float u, v, len;
    float tmp[2];
    vec3f norm;
    vec2f uv;
    uint idx;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.f;
    r2 = outer_radius + tooth_depth / 2.f;
    nr0 = r0/r2;
    nr1 = r1/r2;
    nr2 = 1.f;
    da = 2.f*(float) M_PI / teeth / 4.f;
    uv = (vec2f){ 0,0 };

    normal_3f(0.f, 0.f, 1.f);

    /* draw front face */
    idx = vertex_buffer_count(vb);
    for (i = 0; i <= teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        ca3 = cosf(angle+3*da);
        sa3 = sinf(angle+3*da);
        vertex_3f(r0*ca0, r0*sa0, width*0.5f, nr0*ca0, nr0*sa0);
        vertex_3f(r1*ca0, r1*sa0, width*0.5f, nr1*ca0, nr1*sa0);
        if (i < teeth) {
            vertex_3f(r0*ca0, r0*sa0, width*0.5f, nr0*ca0, nr0*sa0);
            vertex_3f(r1*ca3, r1*sa3, width*0.5f, nr1*ca3, nr1*sa3);
        }
    }
    index_buffer_add_primitves(ib, primitive_topology_quad_strip, teeth*2, idx);

    /* draw front sides of teeth */
    da = 2.f*(float) M_PI / teeth / 4.f;
    idx = vertex_buffer_count(vb);
    for (i = 0; i < teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        ca1 = cosf(angle+1*da);
        sa1 = sinf(angle+1*da);
        ca2 = cosf(angle+2*da);
        sa2 = sinf(angle+2*da);
        ca3 = cosf(angle+3*da);
        sa3 = sinf(angle+3*da);
        vertex_3f(r1*ca0, r1*sa0, width*0.5f, nr1*ca0, nr1*sa0);
        vertex_3f(r2*ca1, r2*sa1, width*0.5f, nr2*ca1, nr2*sa1);
        vertex_3f(r2*ca2, r2*sa2, width*0.5f, nr2*ca2, nr2*sa2);
        vertex_3f(r1*ca3, r1*sa3, width*0.5f, nr1*ca3, nr1*sa3);
    }
    index_buffer_add_primitves(ib, primitive_topology_quads, teeth, idx);

    normal_3f(0.f, 0.f, -1.f);

    /* draw back face */
    idx = vertex_buffer_count(vb);
    for (i = 0; i <= teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        ca3 = cosf(angle+3*da);
        sa3 = sinf(angle+3*da);
        vertex_3f(r1*ca0, r1*sa0, -width*0.5f, nr1*ca0, nr1*sa0);
        vertex_3f(r0*ca0, r0*sa0, -width*0.5f, nr0*ca0, nr0*sa0);
        if (i < teeth) {
            vertex_3f(r1*ca3, r1*sa3, -width*0.5f, nr1*ca3, nr1*sa3);
            vertex_3f(r0*ca0, r0*sa0, -width*0.5f, nr0*ca0, nr0*sa0);
        }
    }
    index_buffer_add_primitves(ib, primitive_topology_quad_strip, teeth*2, idx);

    /* draw back sides of teeth */
    da = 2.f*(float) M_PI / teeth / 4.f;
    idx = vertex_buffer_count(vb);
    for (i = 0; i < teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        ca1 = cosf(angle+1*da);
        sa1 = sinf(angle+1*da);
        ca2 = cosf(angle+2*da);
        sa2 = sinf(angle+2*da);
        ca3 = cosf(angle+3*da);
        sa3 = sinf(angle+3*da);
        vertex_3f(r1*ca3, r1*sa3, -width*0.5f, nr1*ca3, nr1*sa3);
        vertex_3f(r2*ca2, r2*sa2, -width*0.5f, nr2*ca2, nr2*sa2);
        vertex_3f(r2*ca1, r2*sa1, -width*0.5f, nr2*ca1, nr2*sa1);
        vertex_3f(r1*ca0, r1*sa0, -width*0.5f, nr1*ca0, nr1*sa0);
    }
    index_buffer_add_primitves(ib, primitive_topology_quads, teeth, idx);

    /* draw outward faces of teeth */
    idx = vertex_buffer_count(vb);
    for (i = 0; i < teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        ca1 = cosf(angle+1*da);
        sa1 = sinf(angle+1*da);
        ca2 = cosf(angle+2*da);
        sa2 = sinf(angle+2*da);
        ca3 = cosf(angle+3*da);
        sa3 = sinf(angle+3*da);
        ca4 = cosf(angle+4*da);
        sa4 = sinf(angle+4*da);
        tmp[0] = r2*ca1 - r1*ca0;
        tmp[1] = r2*sa1 - r1*sa0;
        normalize2f(tmp);
        normal_3f(tmp[1], -tmp[0], 0.f);
        vertex_3f(r1*ca0, r1*sa0,  width*0.5f, nr1*ca0, nr1*sa0);
        vertex_3f(r1*ca0, r1*sa0, -width*0.5f, nr1*ca0, nr1*sa0);
        vertex_3f(r2*ca1, r2*sa1, -width*0.5f, nr2*ca1, nr2*sa1);
        vertex_3f(r2*ca1, r2*sa1,  width*0.5f, nr2*ca1, nr2*sa1);
        normal_3f(ca0, sa0, 0.f);
        vertex_3f(r2*ca1, r2*sa1,  width*0.5f, nr2*ca1, nr2*sa1);
        vertex_3f(r2*ca1, r2*sa1, -width*0.5f, nr2*ca1, nr2*sa1);
        vertex_3f(r2*ca2, r2*sa2, -width*0.5f, nr2*ca2, nr2*sa2);
        vertex_3f(r2*ca2, r2*sa2,  width*0.5f, nr2*ca2, nr2*sa2);
        tmp[0] = r1*ca3 - r2*ca2;
        tmp[1] = r1*sa3 - r2*sa2;
        normalize2f(tmp);
        normal_3f(tmp[1], -tmp[0], 0.f);
        vertex_3f(r2*ca2, r2*sa2,  width*0.5f, nr2*ca2, nr2*sa2);
        vertex_3f(r2*ca2, r2*sa2, -width*0.5f, nr2*ca2, nr2*sa2);
        vertex_3f(r1*ca3, r1*sa3, -width*0.5f, nr1*ca3, nr1*sa3);
        vertex_3f(r1*ca3, r1*sa3,  width*0.5f, nr1*ca3, nr1*sa3);
        normal_3f(ca0, sa0, 0.f);
        vertex_3f(r1*ca3, r1*sa3,  width*0.5f, nr1*ca3, nr1*sa3);
        vertex_3f(r1*ca3, r1*sa3, -width*0.5f, nr1*ca3, nr1*sa3);
        vertex_3f(r1*ca4, r1*sa4, -width*0.5f, nr1*ca4, nr1*sa4);
        vertex_3f(r1*ca4, r1*sa4,  width*0.5f, nr1*ca4, nr1*sa4);
    }
    index_buffer_add_primitves(ib, primitive_topology_quads, teeth*4, idx);

    /* draw inside radius cylinder */
    idx = vertex_buffer_count(vb);
    for (i = 0; i <= teeth; i++) {
        angle = i*2.f*(float) M_PI / teeth;
        ca0 = cosf(angle);
        sa0 = sinf(angle);
        normal_3f(-ca0, -sa0, 0.f);
        vertex_3f(r0*ca0, r0*sa0, -width*0.5f, nr0*ca0, nr0*sa0);
        vertex_3f(r0*ca0, r0*sa0,  width*0.5f, nr0*ca0, nr0*sa0);
    }
    index_buffer_add_primitves(ib, primitive_topology_quad_strip, teeth, idx);
}

/*
 * OpenGL draw
 */
static void draw()
{
    /* create gear model and view matrices */
    mat4x4_translate(v, 0.0, 0.0, view_dist);
    mat4x4_rotate(v, v, 1.0, 0.0, 0.0, (view_rotx / 180) * M_PI);
    mat4x4_rotate(v, v, 0.0, 1.0, 0.0, (view_roty / 180) * M_PI);
    mat4x4_rotate(v, v, 0.0, 0.0, 1.0, (view_rotz / 180) * M_PI);

    mat4x4_translate(m, -3.0, -2.0, 0.0);
    mat4x4_rotate_Z(gm[0], m, (angle / 180) * M_PI);

    mat4x4_translate(m, 3.1f, -2.f, 0.f);
    mat4x4_rotate_Z(gm[1], m, ((-2.f * angle - 9.f) / 180) * M_PI);

    mat4x4_translate(m, -3.1f, 4.2f, 0.f);
    mat4x4_rotate_Z(gm[2], m, ((-2.f * angle - 25.f) / 180) * M_PI);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(size_t i = 0; i < 3; i++) {
        glBindVertexArray(vao[i]);
        uniform_matrix_4fv("u_model", (const GLfloat *)gm[i]);
        uniform_matrix_4fv("u_view", (const GLfloat *)v);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[i]);
        glBindVertexArray(vao[i]);
        glDrawElements(GL_TRIANGLES, (GLsizei)ib[i].count, GL_UNSIGNED_INT, (void*)0);
    }
}

/*
 * OpenGL reshape
 */
static void reshape(int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    glViewport(0, 0, (GLint) width, (GLint) height);
    mat4x4_frustum(p, -1.0, 1.0, -h, h, 5.0, 60.0);
    uniform_matrix_4fv("u_projection", (const GLfloat *)p);
}

/*
 * animation update
 */
static void animate()
{
    if (animation) {
        angle += 1;
    }
}

/*
 * OpenGL initialization
 */
static void init(void)
{
    GLuint shaders[2];

    /* shader program */
    shaders[0] = compile_shader(GL_VERTEX_SHADER, vert_shader_filename);
    shaders[1] = compile_shader(GL_FRAGMENT_SHADER, frag_shader_filename);
    program = link_program(shaders, 2, NULL);

    /* create gear vertex and index buffers */
    for (size_t i = 0; i < 3; i++) {
        vertex_buffer_init(&vb[i]);
        index_buffer_init(&ib[i]);
    }
    gear(&vb[0], &ib[0], 1.f, 4.f, 1.f, 20, 0.7f, (vec4f){0.8f, 0.1f, 0.f, 1.f});
    gear(&vb[1], &ib[1], 0.5f, 2.f, 2.f, 10, 0.7f, (vec4f){0.f, 0.8f, 0.2f, 1.f});
    gear(&vb[2], &ib[2], 1.3f, 2.f, 0.5f, 10, 0.7f,(vec4f){0.2f, 0.2f, 1.f, 1.f});

    /* create vertex array, vertex buffer and index buffer objects */
    for (size_t i = 0; i < 3; i++) {
        glGenVertexArrays(1, &vao[i]);
        glBindVertexArray(vao[i]);
        vertex_buffer_create(&vbo[i], GL_ARRAY_BUFFER, vb[i].data, vb[i].count * sizeof(vertex));
        vertex_buffer_create(&ibo[i], GL_ELEMENT_ARRAY_BUFFER, ib[i].data, ib[i].count * sizeof(uint));
        vertex_array_pointer("a_pos", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,pos));
        vertex_array_pointer("a_normal", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,norm));
        vertex_array_pointer("a_uv", 2, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,uv));
        vertex_array_pointer("a_color", 4, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,col));
    }

    /* set light position uniform */
    glUseProgram(program);
    uniform_3f("u_lightpos", 5.f, 5.f, 10.f);

    /* enable OpenGL capabilities */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

/*
 * keyboard dispatch
 */
static void keystroke(int key)
{
    switch (key) {
    case 'q': running = 0; break;
    case 'x': animation = !animation; break;
    case 'Z': view_rotz -= 5.0; break;
    case 'z': view_rotz += 5.0; break;
    case 'C': view_dist += 5.0; break;
    case 'c': view_dist += -5.0; break;
    case 'w': view_rotx += 5.0; break;
    case 's': view_rotx -= 5.0; break;
    case 'a': view_roty += 5.0; break;
    case 'd': view_roty -= 5.0; break;
    default: return;
    }
}

/*
 * help text
 */
static void print_help(int argc, char **argv)
{
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -s, --frame-size <width>x<height>  window or image size (default %dx%d)\n"
        "  -i, --frame-interval <integer>     interframe delay ms (default %d)\n"
        "  -c, --frame-count <integer>        output frame count limit (default %d)\n"
        "  -z, --compression                  enable zlib compression\n"
        "  -x, --statistics                   print statistics on quit\n"
        "  -h, --help                         command line help\n",
        argv[0], width, height, millis, count);
}

/*
 * command-line option parsing
 */

static int check_param(int cond, const char *param)
{
    if (cond) {
        printf("error: %s requires parameter\n", param);
    }
    return (help = cond);
}

static int match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

static void parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-s", "--frame-size")) {
            if (check_param(++i == argc, "--frame-size")) break;
            sscanf(argv[i++], "%dx%d", &width, &height);
        } else if (match_opt(argv[i], "-c", "--frame-count")) {
            if (check_param(++i == argc, "--frame-count")) break;
            count = atoi(argv[i++]);
        } else if (match_opt(argv[i], "-i", "--frame-interval")) {
            if (check_param(++i == argc, "--frame-interval")) break;
            millis = atoi(argv[i++]);
        } else if (match_opt(argv[i], "-z", "--compression")) {
            compression += 1;
            i++;
        } else if (match_opt(argv[i], "-9", "--zz")) {
            compression += 2;
            i++;
        } else if (match_opt(argv[i], "-x", "--statistics")) {
            statistics++;
            i++;
        } else if (match_opt(argv[i], "-h", "--help")) {
            help++;
            i++;
         } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help++;
            break;
        }
    }

    if (help) {
        print_help(argc, argv);
        exit(1);
    }
}

/*
 * kitty_gears main loop
 */
static int kitty_gears(int argc, char *argv[])
{
    OSMesaContext ctx;
    uint8_t *buffer;
    uint lh = height / 18;
    uint frame;
    size_t len;
    pos p;

    /* Create an RGBA-mode context */
    if (!(ctx = OSMesaCreateContextExt( OSMESA_RGBA, 16, 0, 0, NULL))) {
        fprintf(stderr, "OSMesaCreateContext failed!\n");
        return 0;
    }

    /* Allocate the image buffer */
    if (!(buffer = (uint8_t*)malloc(width * height * sizeof(uint)))) {
        fprintf(stderr, "Alloc image buffer failed!\n");
        return 0;
    }

    /* Bind the buffer to the context and make it current */
    if (!OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, width, height)) {
        fprintf(stderr, "OSMesaMakeCurrent failed!\n");
        return 0;
    }

    kitty_key_callback(keystroke);

    init();
    reshape(width, height);

    for (uint i=0; i < lh; i++) printf("\n");

    kitty_setup_termios();
    p = kitty_get_position();
    kitty_hide_cursor();

    /* loop displaying frames */
    for(frame = 0; frame < count && running; frame++)
    {
        draw();
        glFlush();

        /* flip buffer and output to kitty as base64 RGBA data*/
        uint iid = 2 + (frame&1);
        kitty_set_position(p.x, p.y-lh);
        kitty_flip_buffer_y((uint*)buffer, width, height);
        len = kitty_send_rgba('T', iid, compression, buffer, width, height);

        bytes_rendered += (width * height) << 2;
        bytes_transferred += len;

        kitty_poll_events(millis);
        animate();
    }

    /* drain kitty responses */
    kitty_poll_events(millis);

    /* restore cursor position then show cursor */
    kitty_show_cursor();
    kitty_set_position(p.x, p.y);
    kitty_restore_termios();
    printf("\n");

    /* print statistics */
    if (statistics) {
        printf("frames rendered = %u\n", frame);
        printf("data transfered = %zu (bytes)\n", bytes_transferred);
        printf("data rendered   = %zu (bytes)\n", bytes_rendered);
        if (bytes_transferred != bytes_rendered) {
            float factor = (float)bytes_rendered/(float)bytes_transferred;
            float efficiency = (1.f - 1.f/factor)*100.f;
            printf("efficiency      = %5.2f%% (%5.2fX)\n", efficiency, factor);
        }
    }

    /* release memory and exit */
    OSMesaDestroyContext(ctx);
    free(buffer);
    exit(EXIT_SUCCESS);
}

/*
 * entry point
 */
int main(int argc, char **argv)
{
    parse_options(argc, argv);
    kitty_gears(argc, argv);
    return 0;
}
