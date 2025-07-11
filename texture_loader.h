#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

#include <glad/glad.h>

#include <stb/image_load.cpp>


// Generate textures and its object and bind it
unsigned int TextureFromFile(const char* filename, const std::string directory)
{
    unsigned int id;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Texture filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Load image
    int width, height, nrChannels;
    const std::string path = directory + '/' + filename;
    
    std::cout << "Loading image: " << path << '\n';
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    // Get the format based on the extension
    std::string file_ext = filename;
    file_ext = file_ext.substr(file_ext.find_last_of('.'));
    const int format = file_ext == ".png" ? GL_RGBA : GL_RGB;

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

unsigned int TextureFromFile(const char* path)
{
    unsigned int id;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Texture filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    // Get the format based on the extension
    std::string file_ext = path;
    file_ext = file_ext.substr(file_ext.find_last_of('.'));
    const int format = file_ext == ".png" ? GL_RGBA : GL_RGB;

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

#endif
