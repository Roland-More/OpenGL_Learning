#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <string>

#include "texture_loader.h"


class PBRMaterial 
{
public:
    PBRMaterial(std::string pathToMaterial) 
    {
        albedoTexture = TextureFromFile((pathToMaterial + "/albedo.png").c_str());
        normalTexture = TextureFromFile((pathToMaterial + "/normal.png").c_str());
        metallicTexture = TextureFromFile((pathToMaterial + "/metallic.png").c_str());
        roughnessTexture = TextureFromFile((pathToMaterial + "/roughness.png").c_str());
        aoTexture = TextureFromFile((pathToMaterial + "/ao.png").c_str());
    }

    unsigned int albedoTexture;
    unsigned int normalTexture;
    unsigned int metallicTexture;
    unsigned int roughnessTexture;
    unsigned int aoTexture;
};

#endif
