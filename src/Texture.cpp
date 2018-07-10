#include "Texture.h"

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
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapS);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapT);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
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

bool Texture::Load(const char* path, Texture* outTexture)
{
    if (table::tryget(textures, path, *outTexture))
    {
        return true;
    }

    int w, h, n;
    void* pixels = stbi_load(path, &w, &h, &h, 0);
    if (pixels)
    {
        Texture texture;
        Texture::Create(texture);
        Texture::SetPixels(texture, w, h, pixels);

        if (outTexture)
        {
            outTexture[0] = texture;
        }
        table::set(textures, path, texture);

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
void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
    Texture texture;
    if (Texture::Load(path, &texture))
    {
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
    FILE* file = fopen(path, "r");
    if (file)
    {
        fseek(file, -1, SEEK_END);

        int   number = ftell(file);
        char* buffer = (char*)malloc(number);

        fread(buffer, sizeof(char), number, file);
        fclose(file);

        if (length)
        {
            *length = number;
        }
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