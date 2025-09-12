#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>

#include "shader.h"
#include "model_flags.h"


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, bool hasTangents)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh(hasTangents);
    }

    void Draw(Shader &shader, unsigned int flags)
    {
        if (flags & ModelLoad_CustomTex)
        {
            // Nothing
        }
        else if (flags & ModelLoad_PBR)
        {
            for (unsigned int i = 0; i < textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding

                std::string name = textures[i].type;
                shader.setInt((name).c_str(), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }
        }
        else
        {
            unsigned int diffuseNr = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr = 1;
            for (unsigned int i = 0; i < textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                // retrieve texture number (the N in diffuse_textureN)
                std::string number;
                std::string name = textures[i].type;
                if (name == "diffuse")
                    number = std::to_string(diffuseNr++);
                else if (name == "specular")
                    number = std::to_string(specularNr++);
                else if (name == "normal")
                    number = std::to_string(normalNr++);

                shader.setInt((name + number).c_str(), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    unsigned int getVAO()
    {
        return VAO;
    }

private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh(bool hasTangents)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                    &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        if (hasTangents)
        {
            // vertex Tangent vectors
            glEnableVertexAttribArray(3);	
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        }

        glBindVertexArray(0);
    }
};

#endif
