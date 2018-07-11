#include "Texture.h"

#include <Windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "memwise/membuf.h"
#include "memwise/table.hpp"

static table_t<const char*, Texture> textures(membuf_heap());

void Texture::Create(Texture& texture)
{
    if (!texture.handle)
    {
        glGenTextures(1, &texture.handle);
    }
}

void Texture::Delete(Texture& texture)
{
    if (!texture.handle)
    {
        glDeleteTextures(1, &texture.handle);
        texture.handle = 0;
    }
}

void Texture::Apply(Texture& texture)
{
    if (texture.handle)
    {
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetPixels(Texture& texture, int width, int height, const void* pixels, GLenum format, GLenum internalFormat)
{
    if (texture.handle && width > 0 && height > 0 && pixels)
    {
        texture.width  = width;
        texture.height = height;

        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

bool Texture::Load(const char* path, Texture* outTexture, GLenum format, GLenum internalFormat)
{
    if (table::tryget(textures, path, *outTexture))
    {
        return true;
    }

    int w, h, n;
    void* pixels = stbi_load(path, &w, &h, &n, 0);
    if (pixels)
    {
        Texture texture;
        Texture::Create(texture);
        Texture::Apply(texture);
        Texture::SetPixels(texture, w, h, pixels, format, internalFormat);

        if (outTexture)
        {
            outTexture[0] = texture;
        }
        table::set(textures, path, texture);
        stbi_image_free(pixels);

        return true;
    }
    else
    {
        return false;
    }
}

bool Texture::Unload(const char* path)
{
    Texture texture;
    if (table::tryget(textures, path, texture))
    {
        Texture::Delete(texture);
        table::remove(textures, path);
        return true;
    }
    else
    {
        return false;
    }
}


#include "spine/extension.h"
GLenum Filter_SpineToGL(spAtlasFilter filter)
{
    switch (filter)
    {
    case SP_ATLAS_LINEAR:
        return GL_LINEAR;

    case SP_ATLAS_NEAREST:
        return GL_NEAREST;
    }

    return GL_LINEAR;
}

GLenum Wrap_SpineToGL(spAtlasWrap wrap)
{
    switch (wrap)
    {
    case SP_ATLAS_REPEAT:
        return GL_REPEAT;

    case SP_ATLAS_CLAMPTOEDGE:
        return GL_CLAMP;
    }

    return GL_REPEAT;
}

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
    GLenum format, internalFormat;
    switch (self->format)
    {
    case SP_ATLAS_RGB888:
        format         = GL_RGB;
        internalFormat = GL_RGBA;
        break;

    case SP_ATLAS_RGBA8888:
        format         = GL_RGBA;
        internalFormat = GL_RGBA;
        break;

    case SP_ATLAS_RGBA4444:
        format         = GL_RGBA4;
        internalFormat = GL_RGBA;
        break;
    }

    Texture texture;
    if (Texture::Load(path, &texture, format, internalFormat))
    {             
        texture.wrapS = Wrap_SpineToGL(self->uWrap);
        texture.wrapT = Wrap_SpineToGL(self->vWrap);
        texture.minFilter = Filter_SpineToGL(self->minFilter);
        texture.magFilter = Filter_SpineToGL(self->magFilter);
        Texture::Apply(texture);

        Texture* savedTexture = new Texture();
        savedTexture[0] = texture;
        self->rendererObject = savedTexture;
    }
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    Texture* texture = (Texture*)self->rendererObject;
    delete texture;
}

char* _spUtil_readFile(const char* path, int* length)
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
        DWORD number = GetFileSize(file, NULL);
        char* buffer = (char*)malloc((number + 1) * sizeof(char));

        if (ReadFile(file, buffer, number, NULL, NULL))
        {
        }
        CloseHandle(file);

        if (length)
        {
            *length = number;
        }
        buffer[number] = 0;
        return buffer;
    }
    else
    {
        if (length)
        {
            *length = 0;
        }
        return NULL;
    }
}