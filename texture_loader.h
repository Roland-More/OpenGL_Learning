#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

#include <glad/glad.h>

#include <stb/image_load.cpp>


enum Texture_filter {
    NEAREST = 0x2600,
    LINEAR = 0x2601,
    LINEAR_MIPMAP_LINEAR = 0x2703,
};

enum Input_format {
    RGB = 0x1907,
    RGBA = 0x1908,
    GAMMA_CORRECTED = 0x8C40,
    GAMMA_CORRECTED_ALPHA = 0x8C42
};

// Generate textures and its object and bind it
unsigned int TextureFromFile(const char* path)
{
    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    // Get the format based on the number of channels
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else
    {
        std::cout << "Unsupported texture format" << std::endl;
        stbi_image_free(data);
        return 0;
    }

    unsigned int id;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    // Texture filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Set the image as the openGL objects data and generate a mipmap for it

    // The first argument specifies the texture target
    // The second argument specifies the mipmap level for which we want to create a texture for
    // The third argument tells OpenGL in what kind of format we want to store the texture
    // The 4th and 5th argument sets the width and height of the resulting texture
    // The next argument should always be 0 (some legacy stuff).
    // The 7th and 8th argument specify the format and datatype of the source image
    // The last argument is the actual image data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free the image data after its loaded into GL object
    stbi_image_free(data);

    return id;
}

unsigned int TextureFromFile(const char* path, Input_format texformat)
{
    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    // Get the format based on the number of channels
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
    {
        format = GL_RGBA;
        if (texformat == GAMMA_CORRECTED)
            texformat = GAMMA_CORRECTED_ALPHA;
        else
            texformat = RGBA;
    }
    else
    {
        std::cout << "Unsupported texture format" << std::endl;
        stbi_image_free(data);
        return 0;
    }

    unsigned int id;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    // Texture filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Set the image as the openGL objects data and generate a mipmap for it

    // The first argument specifies the texture target
    // The second argument specifies the mipmap level for which we want to create a texture for
    // The third argument tells OpenGL in what kind of format we want to store the texture
    // The 4th and 5th argument sets the width and height of the resulting texture
    // The next argument should always be 0 (some legacy stuff).
    // The 7th and 8th argument specify the format and datatype of the source image
    // The last argument is the actual image data
    glTexImage2D(GL_TEXTURE_2D, 0, texformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free the image data after its loaded into GL object
    stbi_image_free(data);

    return id;
}

unsigned int TextureFromFile(const char* path, Texture_filter mag_filter)
{
    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    // Get the format based on the number of channels
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else
    {
        std::cout << "Unsupported texture format" << std::endl;
        stbi_image_free(data);
        return 0;
    }

    unsigned int id;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    // Texture filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Set the image as the openGL objects data and generate a mipmap for it

    // The first argument specifies the texture target
    // The second argument specifies the mipmap level for which we want to create a texture for
    // The third argument tells OpenGL in what kind of format we want to store the texture
    // The 4th and 5th argument sets the width and height of the resulting texture
    // The next argument should always be 0 (some legacy stuff).
    // The 7th and 8th argument specify the format and datatype of the source image
    // The last argument is the actual image data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free the image data after its loaded into GL object
    stbi_image_free(data);

    return id;
}

unsigned int loadCubemap(const char** faces)
{
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            int inputFormat = nrChannels == 3 ? GL_RGB : GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return id;
}  

unsigned int loadHdrTexture(const char* path)
{
    int width, height, nrComponents;
    float *data = stbi_loadf(path, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    return hdrTexture;
}

#endif
