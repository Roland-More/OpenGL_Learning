#include <iostream>
#include <string>
#include <cmath>

#include <glad/glad.h> // Glad sa importuje pred glfw
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "texture_loader.h"
#include "render_shapes.h"
#include "PBR_material.h"

#include "debugging.h"

#include "text_rendering.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Update viewportu
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // cursor movement tracking
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // scrolling tracking
void processInput(GLFWwindow* window);


inline float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}


// Settings
float SCR_WIDTH = 800.0f;
float SCR_HEIGHT = 600.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


int main()
{
    // Inicializacia glfw
    // ------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Give buffer pixels more sample points for MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Vytvorenie okna
    // ---------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Serus", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Nastavenie funkcie na udpate velkosti viewportu pre resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // setting the function for cursor movement tracking
    glfwSetCursorPosCallback(window, mouse_callback);

    // setting the function for scrolling tracking
    glfwSetScrollCallback(window, scroll_callback);

    // Enable cursor capturing + hide it
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // just testing
    loadFont();

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable culling
    // glEnable(GL_CULL_FACE);

    // Enable MSAA (Multi Sample Anti-Aliasing)
    glEnable(GL_MULTISAMPLE);

    // Enable seamless cubemap sampling for prefiltered cubemaps with mipmaps
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Load shader porgrams
    // --------------------
    Shader gPassPBRShader("shaders/vertex/lighting/3d_PBR.glsl", "shaders/fragment/deferred/PBR/g_passPBR.glsl");
    Shader lPassPBRShader("shaders/vertex/2d_tex.glsl", "shaders/fragment/deferred/PBR/l_passtex_IBL.glsl");

    // Shader PBRShader("shaders/vertex/lighting/3d_PBR.glsl", "shaders/fragment/PBR/surface_model_IBL.glsl");

    Shader skyboxShader("shaders/vertex/cubemap.glsl", "shaders/fragment/cubemap/skyboxhrd.glsl");

    Shader equirectangularShader("shaders/vertex/equirectangular.glsl", "shaders/fragment/equirectangular.glsl");
    Shader irradianceShader("shaders/vertex/equirectangular.glsl", "shaders/fragment/cubemap/cubemap_convolute.glsl");
    Shader prefilterShader("shaders/vertex/equirectangular.glsl", "shaders/fragment/cubemap/cubemap_prefilterconv.glsl");
    Shader brdfShader("shaders/vertex/2d_tex.glsl", "shaders/fragment/cubemap/cubemap_brdfconv.glsl");

    // Set up uniforms and buffer data
    const glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };

    const glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    const int nrRows    = 7;
    const int nrColumns = 7;
    const float spacing = 2.5f;

    // Load textures
    // -------------
    const unsigned int MATERIAL_COUNT = 3;
    const PBRMaterial materials[MATERIAL_COUNT] = {
        PBRMaterial("resources/textures/PBR_materials/carbon-fiber"),
        PBRMaterial("resources/textures/PBR_materials/rusted_iron"),
        PBRMaterial("resources/textures/PBR_materials/gold-scuffed"),
    };

    const unsigned int hdrTexture = loadHdrTexture("resources/textures/equirectangular/ibl_hdr_radiance.png");

    // Load models
    // -----------
    const PBRMaterial gunMaterial = PBRMaterial("resources/textures/PBR_materials/gun");
    Model gun("resources/models/Cerberus_gun/Cerberus_LP.FBX", ModelLoad_CustomTex);

    // Set up framebuffers
    // -------------------

    // geometry pass framebuffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPositionMetallic, gNormalRougness, gAlbedoAo;
    
    // - position + metallic color buffer
    glGenTextures(1, &gPositionMetallic);
    glBindTexture(GL_TEXTURE_2D, gPositionMetallic);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionMetallic, 0);
    
    // - normal + roughness color buffer
    glGenTextures(1, &gNormalRougness);
    glBindTexture(GL_TEXTURE_2D, gNormalRougness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalRougness, 0);
    
    // - color + ao color buffer
    glGenTextures(1, &gAlbedoAo);
    glBindTexture(GL_TEXTURE_2D, gAlbedoAo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoAo, 0);
    
    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    unsigned int gDepth;
    glGenRenderbuffers(1, &gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);

    // Cubemap framebuffer
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // - create a cubemap to render to
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
                    512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Irradiance cubemap - convoluting the envCubemap (diffuse irradiance)
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, 
                    GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Prefiltered environment cubemap (specular IBL)
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // BRDF lookup texture
    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Render the cubemap
    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // convert HDR equirectangular environment map to cubemap equivalent
    equirectangularShader.use();
    equirectangularShader.setInt("equirectangularMap", 0);
    equirectangularShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube(); // renders a 1x1 cube
    }

    // Generate mipmaps to use for prefiltering fix
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Convolute the cubemap to use for diffuse irradiance
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradianceShader.use();
    irradianceShader.setInt("environmentMap", 0);
    irradianceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }

    // Prefilter the cubemap to use for specular IBL
    prefilterShader.use();
    prefilterShader.setInt("environmentMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth  = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }

    // Generate BRDF lookup texture
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Configure shaders
    // -----------------
    // PBRShader.use();
    // PBRShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
    // PBRShader.setFloat("ao", 1.0f);
    // PBRShader.setInt("irradianceMap", 0);
    // PBRShader.setInt("prefilterMap", 1);
    // PBRShader.setInt("brdfLUT", 2);
    // for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
    // {
    //     PBRShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
    //     PBRShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    // }

    gPassPBRShader.use();
    gPassPBRShader.setInt("albedoMap", 0);
    gPassPBRShader.setInt("normalMap", 1);
    gPassPBRShader.setInt("metallicMap", 2);
    gPassPBRShader.setInt("roughnessMap", 3);
    gPassPBRShader.setInt("aoMap", 4);

    lPassPBRShader.use();
    lPassPBRShader.setInt("PositionMetallicMap", 0);
    lPassPBRShader.setInt("normalRoughnessMap", 1);
    lPassPBRShader.setInt("AlbedoAoMap", 2);
    lPassPBRShader.setInt("irradianceMap", 3);
    lPassPBRShader.setInt("prefilterMap", 4);
    lPassPBRShader.setInt("brdfLUT", 5);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

    for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
    {
        lPassPBRShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
        lPassPBRShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    }

    skyboxShader.use();
    skyboxShader.setInt("environmentMap", 0);

    // Rendering loop
    // --------------
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        const float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        // -----
        processInput(window);

        // Rendering
        // ---------

        // Geometry Pass
        // -------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);

        const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        gPassPBRShader.use();
        gPassPBRShader.setMat4("projection", projection);
        gPassPBRShader.setMat4("view", view);

        // for (int i = 0; i < MATERIAL_COUNT; ++i)
        // {
        //     // render material spheres
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_2D, materials[i].albedoTexture);
        //     glActiveTexture(GL_TEXTURE1);
        //     glBindTexture(GL_TEXTURE_2D, materials[i].normalTexture);
        //     glActiveTexture(GL_TEXTURE2);
        //     glBindTexture(GL_TEXTURE_2D, materials[i].metallicTexture);
        //     glActiveTexture(GL_TEXTURE3);
        //     glBindTexture(GL_TEXTURE_2D, materials[i].roughnessTexture);
        //     glActiveTexture(GL_TEXTURE4);
        //     glBindTexture(GL_TEXTURE_2D, materials[i].aoTexture);

        //     model = glm::mat4(1.0f);
        //     model = glm::translate(model, glm::vec3(3.0f * (i - (MATERIAL_COUNT - 1) / 2.0f), 0.0f, 0.0f));

        //     gPassPBRShader.setMat4("model", model);
        //     gPassPBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        //     renderSphere();
        // }

        // render gun
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gunMaterial.albedoTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gunMaterial.normalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gunMaterial.metallicTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gunMaterial.roughnessTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gunMaterial.aoTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        gPassPBRShader.setMat4("model", model);
        gPassPBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        gun.Draw(gPassPBRShader);

        // Lighting Pass
        // -------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        lPassPBRShader.use();
        lPassPBRShader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionMetallic);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormalRougness);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoAo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

        renderQuad();

        // Scene w/out deffered shading
        // ----------------------------
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);

        // PBRShader.use();
        // PBRShader.setMat4("projection", projection);
        // PBRShader.setMat4("view", view);
        // PBRShader.setVec3("camPos", camera.Position);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // for (int row = 0; row < nrRows; ++row) 
        // {
        //     PBRShader.setFloat("metallic", (float)row / (float)nrRows);
        //     for (int col = 0; col < nrColumns; ++col) 
        //     {
        //         // we limit the roughness to a minimum value of 0.05
        //         // on direct lighting.
        //         PBRShader.setFloat("roughness", glm::max((float)col / (float)nrColumns, 0.05f));
                
        //         model = glm::mat4(1.0f);
        //         model = glm::translate(model, glm::vec3(
        //             (col - (nrColumns / 2)) * spacing, 
        //             (row - (nrRows / 2)) * spacing, 
        //             0.0f
        //         ));
        //         PBRShader.setMat4("model", model);
        //         PBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        //         renderSphere();
        //     }
        // }

        // // render light source (simply re-render sphere at light positions)
        // // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // // keeps the codeprint small.
        // for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        // {
        //     model = glm::mat4(1.0f);
        //     model = glm::translate(model, lightPositions[i]);
        //     model = glm::scale(model, glm::vec3(0.5f));

        //     PBRShader.setMat4("model", model);
        //     PBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        //     renderSphere();
        // }

        // Additional rendering
        // --------------------

        // Copy depth from gBuffer to default framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Skybox
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);

        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        renderCube();

        // Debugging showcase
        DisplayFramebufferTexture(gNormalRougness);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ukoncenie programu
    // ------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // close window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // process movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the cursor moves this callback function executes
// ---------------------------------------------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // offset will be 0 if its the first mouse movement
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Get the offsets
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates are reversed in the system
    lastX = xpos;
    lastY = ypos;

    // Calculate the direction
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    SCR_WIDTH = (float)width;
    SCR_HEIGHT = (float)height;
    glViewport(0, 0, width, height);
}
