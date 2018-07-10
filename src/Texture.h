#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/glew.h>

struct Texture
{
    GLuint handle;
    GLenum wrapS;
    GLenum wrapT;
    GLenum magFilter;
    GLenum minFilter;

    int width;
    int height;

    inline Texture()
        : handle(0)
        , wrapS(GL_REPEAT)
        , wrapT(GL_REPEAT)
        , magFilter(GL_LINEAR)
        , minFilter(GL_LINEAR)
        , width(0)
        , height(0)
    {
    }

public:
    static void Create(Texture& texture);
    static void Delete(Texture& texture);

    static bool Load(const char* path, Texture* texture);
    static bool Unload(const char* path);

    static void Apply(Texture& texture);
    static void SetPixels(Texture& texture, int width, int height, const void* pixels, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA);
};

#endif /* __TEXTUER_H__ */