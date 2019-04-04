#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>
#include <riku/gmath.h>

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

    static void Install(Shader& shader);
    static void Uninstall(Shader& shader);
    
    static void SetUniform(Shader& shader, const char* name, const float2& value);
    static void SetUniform(Shader& shader, const char* name, const float3& value);
    static void SetUniform(Shader& shader, const char* name, const float4& value);
    static void SetUniform(Shader& shader, const char* name, const float2x2& value);
    static void SetUniform(Shader& shader, const char* name, const float3x3& value);
    static void SetUniform(Shader& shader, const char* name, const float4x4& value);
};

#endif /* __SHADER_H__ */