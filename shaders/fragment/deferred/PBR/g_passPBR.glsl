#version 330 core

layout (location = 0) out vec4 gPositionMetallic;
layout (location = 1) out vec4 gNormalRoughness;
layout (location = 2) out vec4 gAlbedoAo;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;


vec3 getNormalFromMap(); // inefficient tangent space calculation here


void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPositionMetallic.rgb = WorldPos;
    // also store the per-fragment normals into the gbuffer
    gNormalRoughness.rgb = getNormalFromMap();
    // and the diffuse per-fragment color
    gAlbedoAo.rgb = texture(albedoMap, TexCoords).rgb;

    // Store roughness, metalic and ao in the remaining spots in the buffers
    gPositionMetallic.a = texture(metallicMap,  TexCoords).r;
    gNormalRoughness.a  = texture(roughnessMap, TexCoords).r;
    gAlbedoAo.a         = texture(aoMap,        TexCoords).r;
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
