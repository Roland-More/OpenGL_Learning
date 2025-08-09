#include <iostream>
#include <string>

#include <glad/glad.h> // Glad sa importuje pred glfw
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "texture_loader.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Update viewportu
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // cursor movement tracking
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // scrolling tracking
void processInput(GLFWwindow* window);


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

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Enable culling
    // glEnable(GL_CULL_FACE);

    // Enable MSAA (Multi Sample Anti-Aliasing)
    glEnable(GL_MULTISAMPLE);

    // Load shader porgrams
    // --------------------
    Shader depthCubemapShader("shaders/vertex/lighting/wrld_depth.glsl", "shaders/fragment/lighting/depth_cubemap.glsl",
                                "shaders/geometry/lighting/depth_cubemap.glsl");
    Shader sourceShader("shaders/vertex/3d.glsl", "shaders/fragment/simple_colors/white.glsl");
    Shader objectShader("shaders/vertex/lighting/3d_lphong_shadow.glsl", "shaders/fragment/lighting/adv-pointl_shadow.glsl");

    // Set up uniforms and instancing buffer data
    // const glm::vec3 lightDirection = glm::vec3(2.0f, -4.0f, 1.0f);
    const glm::vec3 lightPos = glm::vec3(1.5f, 1.0f, 1.5f);

    // Set up framebuffers

    // Depth buffer for the shadow map
    unsigned int depthCubemapFBO;
    glGenFramebuffers(1, &depthCubemapFBO);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    // the texture attachment (depth buffer -> map)
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach the texture to the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);

    // No color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load models

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    const float cubeVertices[] = {
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

        // Front face (z = +0.5)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

        // Left face (x = -0.5)
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

        // Right face (x = +0.5)
        0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

        // Bottom face (y = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

        // Top face (y = +0.5)
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
    };

    const float outCubeVertices[] = {
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 1.0f,

        // Front face (z = +0.5)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  -1.0f,   0.0f, 0.0f,

        // Left face (x = -0.5)
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

        // Right face (x = +0.5)
        0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

        // Bottom face (y = -0.5)
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,  0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 1.0f,  0.0f,   1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   0.0f, 1.0f,  0.0f,   1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f, 1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,  0.0f,   0.0f, 1.0f,

        // Top face (y = +0.5)
        -0.5f,  0.5f, -0.5f,   0.0f,  -1.0f,  0.0f,   0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  -1.0f,  0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  -1.0f,  0.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  -1.0f,  0.0f,   0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  -1.0f,  0.0f,   0.0f, 0.0f,
    };

    const float floorVertices[] = {
        -20.0f,  0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, -3.0f,
        20.0f,  0.0f,  20.0f,   0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
        20.0f,  0.0f, -20.0f,   0.0f, 1.0f, 0.0f,  5.0f, -3.0f,
        20.0f,  0.0f,  20.0f,   0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
        -20.0f,  0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, -3.0f,
        -20.0f,  0.0f,  20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, 5.0f,
    };

    // Configure texturing and load a texture
    // ----------------------------------------------------------------------------
    const unsigned int floorTexture = TextureFromFile("resources/textures/floor.jpg", GAMMA_CORRECTED);
    const unsigned int cubeTexture = TextureFromFile("resources/textures/container.jpg", GAMMA_CORRECTED);

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // out cube VAO
    unsigned int outCubeVAO, outCubeVBO;
    glGenVertexArrays(1, &outCubeVAO);
    glGenBuffers(1, &outCubeVBO);

    glBindVertexArray(outCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outCubeVertices), outCubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // floor VAO
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // Configure shaders
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    glActiveTexture(GL_TEXTURE0);

    depthCubemapShader.use();
    depthCubemapShader.setVec3("lightPos", lightPos);
    depthCubemapShader.setFloat("far_plane", 25.0f);

    objectShader.use();
    objectShader.setInt("material.diffuse", 0);
    objectShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
    objectShader.setFloat("material.shininess", 32.0f);

    objectShader.setVec3("light.position", lightPos);
    objectShader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
    objectShader.setVec3("light.diffuse", 0.75f, 0.75f, 0.75f);
    objectShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    objectShader.setFloat("light.constant", 1.0f);
    objectShader.setFloat("light.linear", 0.07f);
    objectShader.setFloat("light.quadratic", 0.017f);

    objectShader.setInt("shadowMap", 1);
    objectShader.setFloat("far_plane", 25.0f);
    
    // Rendering loop
    // --------------
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
        const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 25.0f);
        
        const glm::mat4 shadowTransforms[6] = {
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)),
        };

        glm::mat4 model = glm::mat4(1.0f);

        // depth map
        // ---------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(1.0f, 0.5f, 0.25f));

        depthCubemapShader.use();
        for (unsigned int i = 0; i < 6; i++)
            depthCubemapShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

        depthCubemapShader.setMat4("model", model);

        glBindVertexArray(cubeVAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.5f, 0.5f, -1.5f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        depthCubemapShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 0.375f, 2.0f));
        model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));

        depthCubemapShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Scene
        // -----
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();
        glm::mat3 normalModel = glm::mat3(1.0f);

        // light source
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        sourceShader.use();
        sourceShader.setMat4("projection", projection);
        sourceShader.setMat4("view", view);
        sourceShader.setMat4("model", model);

        glBindVertexArray(cubeVAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // room
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

        objectShader.use();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        objectShader.setMat4("model", model);
        objectShader.setMat3("normalModel", normalModel);

        objectShader.setVec3("viewPos", camera.Position);

        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glBindVertexArray(outCubeVAO);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(1.0f, 0.5f, 0.25f));
        normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

        objectShader.setMat4("model", model);
        objectShader.setMat3("normalModel", normalModel);

        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glBindVertexArray(cubeVAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.5f, 0.5f, -1.5f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

        objectShader.setMat4("model", model);
        objectShader.setMat3("normalModel", normalModel);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 0.375f, 2.0f));
        model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));
        normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

        objectShader.setMat4("model", model);
        objectShader.setMat3("normalModel", normalModel);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // End of rendering
        glBindVertexArray(0);
    
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
