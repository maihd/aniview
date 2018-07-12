#include "Shader.h"
#include "System.h"

#include "memwise/membuf.h"
#include "memwise//table.hpp"

#include <stdio.h>
#include <Windows.h>

static table_t<const char*, Shader> shaders(membuf_heap());

static GLuint CreateGLShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog) - 1, NULL, infoLog);
        System::Print("Shader error: %s\n", infoLog);

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint CreateGLShaderFromFile(GLenum type, const char* path)
{
    HANDLE file = CreateFileA(
        path,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (file != INVALID_HANDLE_VALUE)
    {
        DWORD length = GetFileSize(file, NULL);
        char* source = (char*)malloc(sizeof(char) * (length + 1));

        if (!ReadFile(file, source, length, NULL, NULL))
        {
            free(source);
            return 0;
        }
        else
        {
            CloseHandle(file);
            source[length] = 0;

            GLuint shader = CreateGLShader(type, source);

            free(source);
            return shader;
        }
    }
    else
    {
        return 0;
    }
}

bool Shader::Load(const char* path, Shader* outShader)
{
    Shader shader;
    if (table::tryget(shaders, path, shader))
    {
        if (outShader)
        {
            *outShader = shader;
        }
        return true;
    }

    char vshaderPath[1024];
    char fshaderPath[1024];
    sprintf(vshaderPath, "%s.vert", path);
    sprintf(fshaderPath, "%s.frag", path);

    GLuint vshader = CreateGLShaderFromFile(GL_VERTEX_SHADER, vshaderPath);
    GLuint fshader = CreateGLShaderFromFile(GL_FRAGMENT_SHADER, fshaderPath);

    if (!vshader || !fshader)
    {
        glDeleteShader(fshader);
        glDeleteShader(vshader);
        return false;
    }

    shader.handle = glCreateProgram();
    glAttachShader(shader.handle, vshader);
    glAttachShader(shader.handle, fshader);
    glLinkProgram(shader.handle);

    glDeleteShader(fshader);
    glDeleteShader(vshader);

    GLint status;
    glGetProgramiv(shader.handle, GL_LINK_STATUS, &status);
    if (!status)
    {
        char infoLog[1024];
        glGetProgramInfoLog(shader.handle, sizeof(infoLog) - 1, NULL, infoLog);
        System::Print("Shader error: %s\n", infoLog);

        glDeleteProgram(shader.handle);
        return false;
    }
    else
    {
        if (outShader)
        {
            *outShader = shader;
        }
        return true;
    }
}

bool Shader::Unload(const char* path)
{
    Shader shader;
    if (table::tryget(shaders, path, shader))
    {
        glDeleteProgram(shader.handle);
        table::remove(shaders, path);
        return true;
    }
    else
    {
        return false;
    }
}

void Shader::Install(Shader& shader)
{
    if (shader.handle)
    {
        glUseProgram(shader.handle);
    }
}

void Shader::Uninstall(Shader& shader)
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (shader.handle == program)
    {
        glUseProgram(0);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const vec2& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform2f(location, value.x, value.y);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const vec3& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const vec4& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const mat2& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix2fv(location, 1, GL_FALSE, value);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const mat3& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const mat4& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}