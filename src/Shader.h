#ifndef __SHADER_H__
#define __SHADER_H__

#include <assert.h>
#include <GL/glew.h>

#include "vmath/vmath.h"

struct Shader
{
public:
    GLuint handle;

    inline Shader()
        : handle(0)
    {
    }

    inline ~Shader()
    {
        handle = 0;
    }

public:
    static bool Load(const char* path, Shader* shader);
    static bool Unload(const char* path);

    static void UseShader(Shader& shader);
    
    static void SetUniform(Shader& shader, const char* name, const vec2& value);
    static void SetUniform(Shader& shader, const char* name, const vec3& value);
    static void SetUniform(Shader& shader, const char* name, const vec4& value);
    static void SetUniform(Shader& shader, const char* name, const mat2& value);
    static void SetUniform(Shader& shader, const char* name, const mat3& value);
    static void SetUniform(Shader& shader, const char* name, const mat4& value);
};

#endif /* __SHADER_H__ */