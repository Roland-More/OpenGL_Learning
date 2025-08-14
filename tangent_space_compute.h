#ifndef TANGENT_SPACE_COMPUTE_H
#define TANGENT_SPACE_COMPUTE_H

#include <glm/glm.hpp>

struct tangentVectors {
    glm::vec3 tangent1;
    glm::vec3 bitangent1; 
    glm::vec3 tangent2;
    glm::vec3 bitangent2;
};

constexpr tangentVectors computeWallTangentSpace()
{
    // Calculating Tangent space vectors

    // positions
    glm::vec3 pos1(-1.0,  1.0, 0.0);
    glm::vec3 pos2(-1.0, -1.0, 0.0);
    glm::vec3 pos3( 1.0, -1.0, 0.0);
    glm::vec3 pos4( 1.0,  1.0, 0.0);
    // texture coordinates
    glm::vec2 uv1(0.0, 1.0);
    glm::vec2 uv2(0.0, 0.0);
    glm::vec2 uv3(1.0, 0.0);
    glm::vec2 uv4(1.0, 1.0);
    // normal vector
    glm::vec3 nm(0.0, 0.0, 1.0);

    // Triangle 1: 
    // edges and delta UV coordinates
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;
    // Final calculation
    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent1(
        f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
        f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
        f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
    );
    glm::vec3 bitangent1(
        f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
        f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
        f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)
    );

    // Triangle 2:
    // edges and delta UV coordinates
    edge1 = pos1 - pos3;
    edge2 = pos4 - pos3;
    deltaUV1 = uv1 - uv3;
    deltaUV2 = uv4 - uv3;
    // Final calculation
    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent2(
        f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
        f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
        f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
    );
    glm::vec3 bitangent2(
        f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
        f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
        f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)
    );

    return {tangent1, bitangent1, tangent2, bitangent2};
}

#endif
