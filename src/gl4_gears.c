/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * === History ===
 *
 * Michael Clark
 *   - Conversion from fixed function to programmable shaders
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef HAVE_GLAD
#include <glad/glad.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "gl2_util.h"

#define USE_SPIRV 0

#if USE_SPIRV
static const char* frag_shader_filename = "shaders/gears.frag.spv";
static const char* vert_shader_filename = "shaders/gears.vert.spv";
#else
static const char* frag_shader_filename = "shaders/gears.frag";
static const char* vert_shader_filename = "shaders/gears.vert";
#endif

static GLfloat view_dist = -40.0f;
static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLfloat angle = 0.f;
static int animation = 1;

static GLuint program;
static GLuint ubo[3], vao[3], vbo[3], ibo[3];
static vertex_buffer vb[3];
static index_buffer ib[3];
static mat4x4 gm[3], m, v, p, mvp;

typedef struct UBO_t {
    mat4x4 projection;
    mat4x4 model;
    mat4x4 view;
    vec4 lightpos;
} UBO_t;

static struct UBO_t UBO[3];

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
static void draw(void)
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

    for(size_t i = 0; i < 3; i++) {
        memcpy(UBO[i].model, &gm[i], sizeof(gm[i]));
        memcpy(UBO[i].view, v, sizeof(v));
    }

    for(size_t i = 0; i < 3; i++) {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo[i]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO[i]), &UBO[i]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(size_t i = 0; i < 3; i++) {
        glBindVertexArray(vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[i]);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[i]);
        glDrawElements(GL_TRIANGLES, (GLsizei)ib[i].count, GL_UNSIGNED_INT, (void*)0);
    }
}

/*
 * OpenGL reshape
 */
void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    glViewport(0, 0, (GLint) width, (GLint) height);
    mat4x4_frustum(p, -1.0, 1.0, -h, h, 5.0, 60.0);
    for(size_t i = 0; i < 3; i++) {
        memcpy(UBO[i].projection, p, sizeof(p));
    }
}

/*
 * animation update
 */
static void animate(void)
{
    if (animation) {
        angle = 100.f * (float) glfwGetTime();
    }
}

static void prelink(void)
{
    GLuint blockIndex = glGetUniformBlockIndex(program, "UBO.ubo");
    glUniformBlockBinding(program, blockIndex, 0);
    glBindFragDataLocation(program, 0, "outFragColor");
}

/*
 * OpenGL initialization
 */
static void init(void)
{
    GLuint vsh, fsh;

    /* shader program */
    vsh = compile_shader(GL_VERTEX_SHADER, vert_shader_filename);
    fsh = compile_shader(GL_FRAGMENT_SHADER, frag_shader_filename);
    program = link_program_ex(vsh, fsh, prelink);

    /* create gear vertex and index buffers */
    for (size_t i = 0; i < 3; i++) {
        vertex_buffer_init(&vb[i]);
        index_buffer_init(&ib[i]);
    }
    gear(&vb[0], &ib[0], 1.f, 4.f, 1.f, 20, 0.7f, (vec4f){0.8f, 0.1f, 0.f, 1.f});
    gear(&vb[1], &ib[1], 0.5f, 2.f, 2.f, 10, 0.7f, (vec4f){0.f, 0.8f, 0.2f, 1.f});
    gear(&vb[2], &ib[2], 1.3f, 2.f, 0.5f, 10, 0.7f,(vec4f){0.2f, 0.2f, 1.f, 1.f});

    /* create vertex array, vertex buffer and index buffer objects */
    glGenVertexArrays(3, vao);
    for (size_t i = 0; i < 3; i++) {
        glBindVertexArray(vao[i]);
        vertex_buffer_create(&vbo[i], GL_ARRAY_BUFFER, vb[i].data, vb[i].count * sizeof(vertex));
        vertex_buffer_create(&ibo[i], GL_ELEMENT_ARRAY_BUFFER, ib[i].data, ib[i].count * sizeof(uint));
        vertex_array_pointer("a_pos", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,pos));
        vertex_array_pointer("a_normal", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,norm));
        vertex_array_pointer("a_uv", 2, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,uv));
        vertex_array_pointer("a_color", 4, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,col));
    }

    glGenBuffers(3, ubo);
    for(size_t i = 0; i < 3; i++) {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo[i]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO[i]), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    /* set light position uniform */
    glUseProgram(program);
    vec4 lightpos = { 5.f, 5.f, 10.f, 0.f };
    for(size_t i = 0; i < 3; i++) {
        memcpy(UBO[i].lightpos, lightpos, sizeof(lightpos));
    }

    /* enable OpenGL capabilities */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

/*
 * keyboard dispatch
 */
void key( GLFWwindow* window, int k, int s, int action, int mods )
{
    if( action != GLFW_PRESS ) return;

    float shiftz = (mods & GLFW_MOD_SHIFT ? -1.0 : 1.0);

    switch (k) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_X: animation = !animation; break;
    case GLFW_KEY_Z: view_rotz += 5.0 * shiftz; break;
    case GLFW_KEY_C: view_dist += 5.0 * shiftz; break;
    case GLFW_KEY_W: view_rotx += 5.0; break;
    case GLFW_KEY_S: view_rotx -= 5.0; break;
    case GLFW_KEY_A: view_roty += 5.0; break;
    case GLFW_KEY_D: view_roty -= 5.0; break;
    default: return;
    }
}

/*
 * main program
 */
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow(512, 512, "GL2 Gears", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwGetFramebufferSize(window, &width, &height);

#ifdef HAVE_GLAD
    gladLoadGL();
#endif

    init();
    reshape(window, width, height);

    while(!glfwWindowShouldClose(window)) {
        draw();
        animate();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

