/*
 * Copyright (c) 2020-2025 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

/*
 * vertex buffer, index buffer and shader loading interface
 */

typedef unsigned uint;

typedef union { int vec[2]; struct { int x, y;       }; struct { int r, g;       }; } vec2i;
typedef union { int vec[3]; struct { int x, y, z;    }; struct { int r, g, b;    }; } vec3i;
typedef union { int vec[4]; struct { int x, y, z, w; }; struct { int r, g, b, a; }; } vec4i;

typedef union { uint vec[2]; struct { uint x, y;       }; struct { uint r, g;       }; } vec2ui;
typedef union { uint vec[3]; struct { uint x, y, z;    }; struct { uint r, g, b;    }; } vec3ui;
typedef union { uint vec[4]; struct { uint x, y, z, w; }; struct { uint r, g, b, a; }; } vec4ui;

typedef union { float vec[2]; struct { float x, y;       }; struct { float r, g;       }; } vec2f;
typedef union { float vec[3]; struct { float x, y, z;    }; struct { float r, g, b;    }; } vec3f;
typedef union { float vec[4]; struct { float x, y, z, w; }; struct { float r, g, b, a; }; } vec4f;

typedef union { double vec[2]; struct { double x, y;       }; struct { double r, g;       }; } vec2d;
typedef union { double vec[3]; struct { double x, y, z;    }; struct { double r, g, b;    }; } vec3d;
typedef union { double vec[4]; struct { double x, y, z, w; }; struct { double r, g, b, a; }; } vec4d;

typedef struct
{
    const char *name;
    GLuint val;
} attr_val;

typedef struct
{
    attr_val *arr;
    GLuint count;
    GLuint size;
} attr_list;

typedef struct
{
    void *data;
    size_t length;
} buffer;

typedef struct
{
    vec3f pos;
    vec3f norm;
    vec2f uv;
    vec4f col;
} vertex;

typedef struct
{
    size_t stride;
    size_t capacity;
    size_t count;
    char *data;
} array_buffer;

typedef array_buffer vertex_buffer;
typedef array_buffer index_buffer;

typedef enum
{
    primitive_topology_triangles,
    primitive_topology_triangle_strip,
    primitive_topology_quads,
    primitive_topology_quad_strip,
} primitive_type;

static GLuint compile_shader(GLenum type, const char *filename);
static GLuint link_program(const GLuint *shaders, GLuint numshaders,
    GLuint (*bindfn)(GLuint prog));
static void vertex_buffer_create(GLuint *obj, GLenum target,
    void *data, size_t size);
static void vertex_array_pointer(const char *attr, GLint size,
    GLenum type, GLboolean norm, size_t stride, size_t offset);
static void vertex_array_1f(const char *attr, float v1);
static void uniform_1i(const char *uniform, GLint i);
static void uniform_3f(const char *uniform, GLfloat v1, GLfloat v2, GLfloat v3);
static void uniform_matrix_4fv(const char *uniform, const GLfloat *mat);

static void array_buffer_init(array_buffer *sb,
    size_t stride, size_t capacity);
static void array_buffer_destroy(array_buffer *sb);
static void* array_buffer_data(array_buffer *sb);
static size_t array_buffer_size(array_buffer *sb);
static size_t array_buffer_stride(array_buffer *sb);
static uint array_buffer_count(array_buffer *sb);
static void array_buffer_resize(array_buffer *sb, size_t count);
static uint array_buffer_add(array_buffer *sb, void *data);

static void vertex_buffer_init(vertex_buffer *vb);
static void vertex_buffer_destroy(vertex_buffer *vb);
static void* vertex_buffer_data(vertex_buffer *vb);
static size_t vertex_buffer_size(vertex_buffer *vb);
static uint vertex_buffer_count(vertex_buffer *vb);
static uint vertex_buffer_add(vertex_buffer *vb, vertex vertex);

static void index_buffer_init(index_buffer *ib);
static void index_buffer_destroy(index_buffer *ib);
static void* index_buffer_data(index_buffer *ib);
static size_t index_buffer_size(index_buffer *ib);
static uint index_buffer_count(index_buffer *ib);
static void index_buffer_add(index_buffer *ib,
    const uint *data, uint count, uint addend);
static void index_buffer_add_primitves(index_buffer *ib,
    primitive_type type, uint count, uint addend);

/*
 * vertex, index and generic array buffer implementation
 */

enum { VERTEX_BUFFER_INITIAL_COUNT = 16 };
enum { INDEX_BUFFER_INITIAL_COUNT = 64 };

static void array_buffer_init(array_buffer *sb, size_t stride, size_t capacity)
{
    sb->stride = stride;
    sb->capacity = capacity;
    sb->count = 0;
    sb->data = (char*)malloc(sb->stride * sb->capacity);
}

static void array_buffer_destroy(array_buffer *sb)
{
    free(sb->data);
    sb->data = NULL;
}

static uint array_buffer_count(array_buffer *sb)
{
    return (uint)sb->count;
}

static void* array_buffer_data(array_buffer *sb)
{
    return sb->data;
}

static size_t array_buffer_stride(array_buffer *sb)
{
    return sb->stride;
}

static size_t array_buffer_size(array_buffer *sb)
{
    return sb->count * sb->stride;
}

static void array_buffer_resize(array_buffer *sb, size_t count)
{
    size_t capacity = sb->capacity;
    while (count > capacity) capacity <<= 1;
    if (capacity > sb->capacity) {
        sb->data = (char*)realloc(sb->data, sb->stride * capacity);
        sb->capacity = capacity;
    }
    sb->count = count;
}

static uint array_buffer_add(array_buffer *sb, void *data)
{
    size_t idx = sb->count;
    array_buffer_resize(sb, idx + 1);
    memcpy(sb->data + (idx * sb->stride), data, sb->stride);
    return (uint)idx;
}

static void vertex_buffer_init(vertex_buffer *vb)
{
    array_buffer_init(vb, sizeof(vertex), VERTEX_BUFFER_INITIAL_COUNT);
}

static void vertex_buffer_destroy(vertex_buffer *vb)
{
    array_buffer_destroy(vb);
}

static uint vertex_buffer_count(vertex_buffer *vb)
{
    return array_buffer_count(vb);
}

static void* vertex_buffer_data(vertex_buffer *vb)
{
    return array_buffer_data(vb);
}

static size_t vertex_buffer_size(vertex_buffer *vb)
{
    return array_buffer_size(vb);
}

static uint vertex_buffer_add(vertex_buffer *vb, vertex v)
{
    return array_buffer_add(vb, &v);
}

static void vertex_buffer_dump(vertex_buffer *vb)
{
    size_t count = vb->count;
    printf("vertex_buffer_%p = {\n", vb);
    for (size_t i = 0; i < count; i++) {
        vertex *v = ((vertex*)vb->data) + i;
        printf("  [%7zu] = { "
            ".pos = {%5.3f,%5.3f,%5.3f}, "
            ".norm = {%5.3f,%5.3f,%5.3f}, "
            ".uv = {%5.3f,%5.3f}, "
            ".col = {%5.3f,%5.3f,%5.3f,%5.3f} }\n",
            i, v->pos.vec[0], v->pos.vec[1], v->pos.vec[2],
            v->norm.vec[0], v->norm.vec[1], v->norm.vec[2],
            v->uv.vec[0], v->uv.vec[1],
            v->col.vec[0], v->col.vec[1], v->col.vec[2], v->col.vec[3]);
    }
    printf("}\n");
}

static void index_buffer_init(index_buffer *ib)
{
    array_buffer_init(ib, sizeof(uint), INDEX_BUFFER_INITIAL_COUNT);
}

static void index_buffer_destroy(index_buffer *ib)
{
    array_buffer_destroy(ib);
}

static uint index_buffer_count(index_buffer *ib)
{
    return array_buffer_count(ib);
}

static void* index_buffer_data(index_buffer *ib)
{
    return array_buffer_data(ib);
}

static size_t index_buffer_size(index_buffer *ib)
{
    return array_buffer_size(ib);
}

static void index_buffer_add(index_buffer *ib,
    const uint *data, uint count, uint addend)
{
    if (ib->count + count >= ib->capacity) {
        do { ib->capacity <<= 1; }
        while (ib->count + count > ib->capacity);
        ib->data = realloc(ib->data, sizeof(uint) * ib->capacity);
    }
    for (uint i = 0; i < count; i++) {
        ((uint*)ib->data)[ib->count++] = data[i] + addend;
    }
}

static void index_buffer_add_primitves(index_buffer *ib,
    primitive_type type, uint count, uint addend)
{
    static const uint tri[] = {0,1,2};
    static const uint tri_strip[] = {0,1,2,2,1,3};
    static const uint quads[] = {0,1,2,0,2,3};

    switch (type) {
    case primitive_topology_triangles:
        for (size_t i = 0; i < count; i++) {
            index_buffer_add(ib, tri, 3, addend);
            addend += 3;
        }
        break;
    case primitive_topology_triangle_strip:
        assert((count&1) == 0);
        for (size_t i = 0; i < count; i += 2) {
            index_buffer_add(ib, tri_strip, 6, addend);
            addend += 2;
        }
        break;
    case primitive_topology_quads:
        for (size_t i = 0; i < count; i++) {
            index_buffer_add(ib, quads, 6, addend);
            addend += 4;
        }
        break;
    case primitive_topology_quad_strip:
        for (size_t i = 0; i < count; i++) {
            index_buffer_add(ib, tri_strip, 6, addend);
            addend += 2;
        }
        break;
    }
}

static void index_buffer_dump(index_buffer *ib)
{
    static const int width = 12;
    size_t count = ib->count;
    printf("index_buffer_%p = {\n", ib);
    size_t i;
    for (i = 0; i < count; i++) {
        if (i % width == 0) printf("  [%7zu] = ", i);
        printf("%7u", ((uint*)ib->data)[i]);
        if (i % width == width-1) printf("\n");
    }
    if (i % width != 0) printf("\n");
    printf("}\n");
}

static attr_list attrs;
static attr_list uniforms;

enum { ATTR_LIST_INITIAL_SIZE = 16, ATTR_NOT_FOUND = 0xffffffff };

static GLuint attr_list_index(attr_list *list, const char*name)
{
    for (GLuint i = 0; i < list->count; i++)
        if (strcmp(list->arr[i].name, name) == 0) return i;
    return ATTR_NOT_FOUND;
}

static GLuint attr_list_value(attr_list *list, const char*name)
{
    GLuint idx = attr_list_index(list, name);
    return idx == ATTR_NOT_FOUND ? idx : list->arr[idx].val;
}

static GLuint attr_list_set(attr_list *list, const char*name, GLuint val)
{
    GLuint idx = attr_list_index(list, name);
    if (idx != ATTR_NOT_FOUND) goto found;
    if (list->size == 0) {
        list->size = ATTR_LIST_INITIAL_SIZE;
        list->arr = (attr_val*)malloc(list->size * sizeof(attr_val));
    }
    if (list->count == list->size) {
        list->count <<= 1;
        list->arr = (attr_val*)realloc(list->arr, list->size * sizeof(attr_val));
    }
    idx = list->count++;
    list->arr[idx].name = strdup(name);
found:
    return (list->arr[idx].val = val);
}

/*
 * shader utilties
 */

static buffer load_file(const char *filename)
{
    FILE *f;
    struct stat statbuf;
    char *buf;
    size_t nread;

    if ((f = fopen(filename, "r")) == NULL) {
        printf("load_file: open: %s: %s\n",
            filename, strerror(errno));
        exit(1);
    }
    if (fstat(fileno(f), &statbuf) < 0) {
        printf("load_file: stat: %s: %s\n",
            filename, strerror(errno));
        exit(1);
    }
    buf = (char*)malloc(statbuf.st_size);
    if ((nread = fread(buf, 1, statbuf.st_size, f)) != statbuf.st_size) {
        printf("load_file: fread: %s: expected %zu got %zu\n",
            filename, (size_t)statbuf.st_size, nread);
        exit(1);
    }
    return (buffer){buf, (size_t)statbuf.st_size};
}

/*
 * code in this header assumes OpenGL 3.2 and OpenGL ES 3.1 as dependencies
 * that can be statically linked. given the code support multiple loaders,
 * OpenGL 4.x functions are resolved here and the scope is currently limited
 * to linking and loading of shaders themselves, which are runtime linked.
 */

typedef void (*func_4_1_glShaderBinary)
(GLsizei, const GLuint *, GLenum, const void *, GLsizei);
typedef void (*func_4_3_glGetProgramResourceName)
(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
typedef void (*func_4_6_glSpecializeShader)
(GLuint, const GLchar *, GLuint, const GLuint *, const GLuint *);

static func_4_1_glShaderBinary           muglShaderBinary;
static func_4_3_glGetProgramResourceName muglGetProgramResourceName;
static func_4_6_glSpecializeShader       muglSpecializeShader;

#if defined (OSMESA_MAJOR_VERSION)
#define muglGetProcAddress OSMesaGetProcAddress
#elif defined (GLFW_VERSION_MAJOR)
#define muglGetProcAddress glfwGetProcAddress
#elif defined (GLX_VERSION)
#define muglGetProcAddress glXGetProcAddress
#elif defined (WGL_VERSION_1_0)
#define muglGetProcAddress wglGetProcAddress
#elif defined (EGL_VERSION)
#define muglGetProcAddress eglGetProcAddress
#endif

static void muglInit()
{
    static int initialized = 0;

    if (initialized) return;

    muglShaderBinary = (func_4_1_glShaderBinary)
        muglGetProcAddress("glShaderBinary");
    muglGetProgramResourceName = (func_4_3_glGetProgramResourceName)
        muglGetProcAddress("glGetProgramResourceName");
    muglSpecializeShader = (func_4_6_glSpecializeShader)
        muglGetProcAddress("glSpecializeShader");

    initialized++;
}

static GLuint compile_shader(GLenum type, const char *filename)
{
    GLint length, status;
    GLuint shader;
    buffer buf;
    int is_spirv;

    buf = load_file(filename);
    length = (GLint)buf.length;
    if (!length) {
        printf("failed to load shader: %s\n", filename);
        exit(1);
    }
    shader = glCreateShader(type);

    is_spirv = strstr(filename, ".spv") == filename + strlen(filename) - 4;
    if (is_spirv) {
        muglInit();
        muglShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
            (const void *)buf.data, length);
        muglSpecializeShader(shader, (const GLchar*)"main", 0, NULL, NULL);
    } else {
        glShaderSource(shader, (GLsizei)1,
            (const GLchar * const *)&buf.data, &length);
        glCompileShader(shader);
    }

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        char *logbuf = (char*)malloc(length + 1);
        glGetShaderInfoLog(shader, length, &length, logbuf);
        printf("shader compile log: %s\n", logbuf);
        free(logbuf);
    }

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        printf("failed to compile shader: %s\n", filename);
        exit(1);
    }

    return shader;
}

static void reflect_gl2(GLuint program, GLint *numattrs, GLint *numuniforms)
{
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, numattrs);
    for (GLint i = 0; i < *numattrs; i++)  {
        char namebuf[128];
        muglGetProgramResourceName(program, GL_PROGRAM_INPUT, i, sizeof(namebuf), NULL, namebuf);
        attr_list_set(&attrs, namebuf, i);
    }

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, numuniforms);
    for (GLint i = 0; i < *numuniforms; i++) {
        char namebuf[128];
        muglGetProgramResourceName(program, GL_UNIFORM, i, sizeof(namebuf), NULL, namebuf);
        attr_list_set(&uniforms, namebuf, glGetUniformLocation(program, namebuf));
    }
}

static void reflect_gl4(GLuint program, GLint *numattrs, GLint *numuniforms)
{
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, numattrs);
    for (GLint i = 0; i < *numattrs; i++)  {
        GLint namelen=-1, size=-1;
        GLenum type = GL_ZERO;
        char namebuf[128];
        glGetActiveAttrib(program, i, sizeof(namebuf)-1, &namelen, &size,
                          &type, namebuf);
        namebuf[namelen] = 0;
        attr_list_set(&attrs, namebuf, i);
    }

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, numuniforms);
    for (GLint i = 0; i < *numuniforms; i++) {
        GLint namelen=-1, size=-1;
        GLenum type = GL_ZERO;
        char namebuf[128];
        glGetActiveUniform(program, i, sizeof(namebuf)-1, &namelen, &size,
                           &type, namebuf);
        namebuf[namelen] = 0;
        attr_list_set(&uniforms, namebuf, glGetUniformLocation(program, namebuf));
    }
}

static GLuint link_program(const GLuint *shaders, GLuint numshaders,
    GLuint (*bindfn)(GLuint prog))
{
    GLuint program;
    GLint status, numattrs, numuniforms;

    program = glCreateProgram();
    for (size_t i = 0; i < numshaders; i++) {
        glAttachShader(program, shaders[i]);
    }

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        printf("failed to link shader program\n");
        exit(1);
    }

    muglInit();
    if (muglGetProgramResourceName) {
        reflect_gl2(program, &numattrs, &numuniforms);
    } else {
        reflect_gl4(program, &numattrs, &numuniforms);
    }

    /*
     * Note: OpenGL by default binds attributes to locations counting
     * from zero upwards. This is problematic with at least the Nvidia
     * driver, where zero has a special meaning. So after linking, we
     * call bindfn which can go through reflected attributes and assign
     * them new indices, say starting from 1 counting upwards. We then
     * re-link the program. bindfn is provided for this purpose. e.g.
     *
     * static GLuint rebind(GLuint program) {
     *   for (size_t i = 0; i < attrs.count; i++) {
     *     glBindAttribLocation(program, (attrs.arr[i].val = n++),
     *       attrs.arr[i].name);
     *   }
     * }
     */
    if (bindfn && bindfn(program) == GL_TRUE) {
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            printf("failed to relink shader program\n");
            exit(1);
        }
    }

    /*
     * Note: support statically linked locations in SPIR-V modules
     * requires us to accept the locations assigned by the driver,
     * so after fetching names, instead of explicitly rebinding,
     * we find the locations assigned by the driver. This is to work
     * around issues where attempting to re-assign indices fails.
     */
    for (size_t i = 0; i < attrs.count; i++) {
        attrs.arr[i].val = glGetAttribLocation(program, attrs.arr[i].name);
    }

    for (size_t i = 0; i < numshaders; i++) {
        glDeleteShader(shaders[i]);
    }

    for (size_t i = 0; i < attrs.count; i++) {
        printf("attr %s = %d\n", attrs.arr[i].name, attrs.arr[i].val);
    }
    for (size_t i = 0; i < uniforms.count; i++) {
        printf("uniform %s = %d\n", uniforms.arr[i].name, uniforms.arr[i].val);
    }

    return program;
}

static void buffer_object_create_offset(GLuint *obj, GLenum target,
    array_buffer *ab, size_t offset, size_t count)
{
    size_t size = array_buffer_stride(ab) * count;
    char *data = (char*)array_buffer_data(ab) + array_buffer_stride(ab) * offset;
    glGenBuffers(1, obj);
    glBindBuffer(target, *obj);
    glBufferData(target, size, (void*)data, GL_STATIC_DRAW);
    glBindBuffer(target, *obj);
}

static void buffer_object_create(GLuint *obj, GLenum target, array_buffer *ab)
{
    buffer_object_create_offset(obj, target, ab, 0, array_buffer_count(ab));
}

static void vertex_array_pointer(const char *attr, GLint size,
    GLenum type, GLboolean norm, size_t stride, size_t offset)
{
    GLuint val;
    if ((val = attr_list_value(&attrs, attr)) != ATTR_NOT_FOUND) {
        glEnableVertexAttribArray(val);
        glVertexAttribPointer(val, size, type, norm, (GLsizei)stride, (const void*)offset);
    }
}

static void vertex_array_1f(const char *attr, float v1)
{
    GLuint val;
    if ((val = attr_list_value(&attrs, attr)) != ATTR_NOT_FOUND) {
        glDisableVertexAttribArray(val);
        glVertexAttrib1f(val, v1);
    }
}

static void uniform_1i(const char *uniform, GLint i)
{
    GLuint val;
    if ((val = attr_list_value(&uniforms, uniform)) != ATTR_NOT_FOUND) {
        glUniform1i(val, i);
    }
}

static void uniform_3f(const char *uniform, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GLuint val;
    if ((val = attr_list_value(&uniforms, uniform)) != ATTR_NOT_FOUND) {
        glUniform3f(val, v1, v2, v3);
    }
}

static void uniform_matrix_4fv(const char *uniform, const GLfloat *mat)
{
    GLuint val;
    if ((val = attr_list_value(&uniforms, uniform)) != ATTR_NOT_FOUND) {
        glUniformMatrix4fv(val, 1, GL_FALSE, mat);
    }
}
