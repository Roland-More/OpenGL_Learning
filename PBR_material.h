#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <string>
#include <filesystem>

#include "texture_loader.h"


const int EXTENSION_COUNT = 2;


class PBRMaterial 
{
public:
    unsigned int albedoTexture;
    unsigned int normalTexture;
    unsigned int metallicTexture;
    unsigned int roughnessTexture;
    unsigned int aoTexture;

    PBRMaterial(const std::string pathToMaterial) 
    {
        albedoTexture = LoadTextureWithAnyExtension(pathToMaterial, "albedo");
        normalTexture = LoadTextureWithAnyExtension(pathToMaterial, "normal");
        metallicTexture = LoadTextureWithAnyExtension(pathToMaterial, "metallic");
        roughnessTexture = LoadTextureWithAnyExtension(pathToMaterial, "roughness");
        aoTexture = LoadTextureWithAnyExtension(pathToMaterial, "ao");
    }

private:
    unsigned int LoadTextureWithAnyExtension(const std::string& folder, const std::string& name) 
    {
        constexpr const char* extensions[EXTENSION_COUNT] = {".png", ".tga"};

        for (int i = 0; i < EXTENSION_COUNT; i++) {
            std::filesystem::path file = folder + "/" + name + extensions[i];
            if (std::filesystem::exists(file)) {
                return TextureFromFile(file.string().c_str());
            }
        }

        return 0;
    }
};

#endif
