// TODO: Optimize Memory Usage -
// only use vertices with tangents when the model actually has any


#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "texture_loader.h"
#include "model_flags.h"


class Model 
{
public:
    Model(const char *path, unsigned int flags = ModelLoad_None)
    {
        this->flags = flags;
        loadModel(path);
    }
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader, flags);
    }

    size_t getNumMeshes()
    {
        return meshes.size();
    }
    
    size_t getMeshIndicesSize(int index)
    {
        return meshes[index].indices.size();
    }

    unsigned int getMeshVAO(int index)
    {
        return meshes[index].getVAO();
    }

private:
    unsigned int flags;
    // model data
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;

    void loadModel(std::string path)
    {
        unsigned int assimpFlags = aiProcess_Triangulate;

        if (flags & ModelLoad_FlipUVs)
            assimpFlags |= aiProcess_FlipUVs;
        if (flags & ModelLoad_Tangents)
            assimpFlags |= aiProcess_CalcTangentSpace;

        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, assimpFlags);

        // Check textures detected by Assimp
        // for (unsigned int m = 0; m < scene->mNumMaterials; m++) 
        // {
        //     aiMaterial* material = scene->mMaterials[m];

        //     for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++) {
        //         aiTextureType texType = static_cast<aiTextureType>(type);
        //         unsigned int texCount = material->GetTextureCount(texType);

        //         if (texCount > 0) {
        //             std::cout << "Material " << m 
        //                     << " has " << texCount 
        //                     << " textures of type " << texType 
        //                     << std::endl;

        //             for (unsigned int t = 0; t < texCount; t++) {
        //                 aiString path;
        //                 material->GetTexture(texType, t, &path);
        //                 std::cout << "   -> " << path.C_Str() << std::endl;
        //             }
        //         }
        //     }
        // }
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);

        std::cout << "Finished loading model: " << path << '\n';
    }

    void processNode(aiNode *node, const aiScene *scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
            meshes.push_back(processMesh(mesh, scene));			
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }  

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        const bool hasTangents = flags & ModelLoad_Tangents;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            // process vertex positions, normals, texture coordinates and if included, tangent vectors
            glm::vec3 vector;

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z; 
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            if (hasTangents)
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            }

            vertices.push_back(vertex);
        }

        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Skip materials if using custom textures
        if (flags & ModelLoad_CustomTex)
            return Mesh(vertices, indices, textures, hasTangents);
            
        // process material
        if (mesh->mMaterialIndex >= 0)
        {            
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            
            if (flags & ModelLoad_PBR)
            {
                std::vector<Texture> albedoMaps = loadMaterialTextures(material,
                                                    aiTextureType_BASE_COLOR, "albedoMap");
                textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
                std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                                    aiTextureType_NORMALS, "normalMap");
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
                std::vector<Texture> metallicMaps = loadMaterialTextures(material,
                                                    aiTextureType_METALNESS, "metallicMap");
                textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
                std::vector<Texture> roughnessMaps = loadMaterialTextures(material,
                                                    aiTextureType_DIFFUSE_ROUGHNESS, "roughnessMap");
                textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
                std::vector<Texture> aoMaps = loadMaterialTextures(material,
                                                    aiTextureType_AMBIENT_OCCLUSION, "aoMap");
                textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
            }
            else
            {
                std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                    aiTextureType_DIFFUSE, "diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                                    aiTextureType_SPECULAR, "specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

                if (ModelLoad_Tangents & flags)
                {
                    std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                                        aiTextureType_HEIGHT, "normal");
                    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
                }
            }
        }
        return Mesh(vertices, indices, textures, hasTangents);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                            std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                texture.type = typeName;

                const std::string fullPath = directory + "/" + str.C_Str();
                const Input_format format = flags & ModelLoad_GAMMA_CRCT ? GAMMA_CORRECTED : RGB;
                texture.id = TextureFromFile(fullPath.c_str(), format);

                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // add to loaded textures
            }
        }
        return textures;
    }
};

#endif
