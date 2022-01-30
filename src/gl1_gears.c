/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * === History ===
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

#ifdef HAVE_GLAD
#include <glad/glad.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GLFW/glfw3.h>

static GLfloat view_dist = -40.0f;
static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.f;
static int animation = 1;

/*
 * Create a gear wheel.
 *
 *   Input:  inner_radius - radius of hole at center
 *                  outer_radius - radius at center of teeth
 *                  width - width of gear teeth - number of teeth
 *                  tooth_depth - depth of tooth
 */

static void
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
    GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.f;
    r2 = outer_radius + tooth_depth / 2.f;

    da = 2.f * (float) M_PI / teeth / 4.f;

    glShadeModel(GL_FLAT);

    glNormal3f(0.f, 0.f, 1.f);

    /* draw front face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        if (i < teeth) {
            glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
            glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
        }
    }
    glEnd();

    /* draw front sides of teeth */
    glBegin(GL_QUADS);
    da = 2.f * (float) M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;

        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
    }
    glEnd();

    glNormal3f(0.0, 0.0, -1.0);

    /* draw back face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        if (i < teeth) {
            glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
            glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        }
    }
    glEnd();

    /* draw back sides of teeth */
    glBegin(GL_QUADS);
    da = 2.f * (float) M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;

        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
    }
    glEnd();

    /* draw outward faces of teeth */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;

        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
        u = r2 * (float) cos(angle + da) - r1 * (float) cos(angle);
        v = r2 * (float) sin(angle + da) - r1 * (float) sin(angle);
        len = (float) sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
        u = r1 * (float) cos(angle + 3 * da) - r2 * (float) cos(angle + 2 * da);
        v = r1 * (float) sin(angle + 3 * da) - r2 * (float) sin(angle + 2 * da);
        glNormal3f(v, -u, 0.f);
        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
        glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
    }

    glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), width * 0.5f);
    glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), -width * 0.5f);

    glEnd();

    glShadeModel(GL_SMOOTH);

    /* draw inside radius cylinder */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        glNormal3f(-(float) cos(angle), -(float) sin(angle), 0.f);
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
    }
    glEnd();

}

/*
 * OpenGL draw
 */
static void draw(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
    glTranslatef(-3.0, -2.0, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.1f, -2.f, 0.f);
    glRotatef(-2.f * angle - 9.f, 0.f, 0.f, 1.f);
    glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.1f, 4.2f, 0.f);
    glRotatef(-2.f * angle - 25.f, 0.f, 0.f, 1.f);
    glCallList(gear3);
    glPopMatrix();

    glPopMatrix();
}

/*
 * OpenGL reshape
 */
void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;

    glViewport(0, 0, (GLint) width, (GLint) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, view_dist);
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

/*
 * OpenGL initialization
 */
static void init(void)
{
    static GLfloat pos[4] = {5.f, 5.f, 10.f, 0.f};
    static GLfloat red[4] = {0.8f, 0.1f, 0.f, 1.f};
    static GLfloat green[4] = {0.f, 0.8f, 0.2f, 1.f};
    static GLfloat blue[4] = {0.2f, 0.2f, 1.f, 1.f};

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    /* make the gears */
    gear1 = glGenLists(1);
    glNewList(gear1, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    gear(1.f, 4.f, 1.f, 20, 0.7f);
    glEndList();

    gear2 = glGenLists(1);
    glNewList(gear2, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    gear(0.5f, 2.f, 2.f, 10, 0.7f);
    glEndList();

    gear3 = glGenLists(1);
    glNewList(gear3, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    gear(1.3f, 2.f, 0.5f, 10, 0.7f);
    glEndList();

    glEnable(GL_NORMALIZE);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(512, 512, "GL1 Gears", NULL, NULL);
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

