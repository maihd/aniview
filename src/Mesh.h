#ifndef __MESH_H__
#define __MESH_H__

#include <GL/glew.h>

#include "vmath/vmath.h"

struct Mesh
{
public:
    struct Vertex
    {
        vec2 pos;
        vec2 uv;
        vec4 color;
    };

public:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    inline Mesh()
        : vao(0)
        , vbo(0)
        , ebo(0)
    {
    }

public:
    static void Create(Mesh& mesh);
    static void Delete(Mesh& mesh);

    static void SetIndices(Mesh& mesh, int count, const unsigned* indices, GLenum usage);
    static void SetVertices(Mesh& mesh, int count, const Vertex* vertices, GLenum usage);
};

#endif /* __MESH_H__ */