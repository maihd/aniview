#include "Shader.h"
#include "System.h"

#include <riku/fs.h>
#include <riku/string.h>
#include <riku/dictionary.h>

static Dictionary<String, Shader> shaders;

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
    FileHandle file = fs::open(path, FileOpen::Read);
    if (file)
    {
        fs::seek(file, FileSeek::End, 0);
        int length = fs::tell(file);
        fs::seek(file, FileSeek::Set, 0);

        char* source = (char*)memory::alloc(length + 1);
        if (fs::read(file, source, length) < 0)
        {
            free(source);
            return 0;
        }
        else
        {
            fs::close(file);

            source[length] = 0;
            GLuint shader = CreateGLShader(type, source);

            memory::dealloc(source);
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
    if (shaders.try_get(path, &shader))
    {
        if (outShader)
        {
            *outShader = shader;
        }
        return true;
    }

    char vshaderPath[1024];
    char fshaderPath[1024];
    string::format(vshaderPath, sizeof(vshaderPath), "%s.vert", path);
    string::format(fshaderPath, sizeof(fshaderPath), "%s.frag", path);

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
    if (shaders.try_get(path, &shader))
    {
        glDeleteProgram(shader.handle);
        shaders.remove(path);
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

void Shader::SetUniform(Shader& shader, const char* name, const float2& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform2f(location, value.x, value.y);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const float3& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const float4& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const float2x2& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix2fv(location, 1, GL_FALSE, (const float*)&value);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const float3x3& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, (const float*)&value);
    }
}

void Shader::SetUniform(Shader& shader, const char* name, const float4x4& value)
{
    GLint location = glGetUniformLocation(shader.handle, name);
    if (location >= 0)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, (const float*)&value);
    }
}