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
#include "tangent_space_compute.h"


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
    Shader objectShader("shaders/vertex/lighting/3d_lphong.glsl", "shaders/fragment/lighting/adv-pointl_multi.glsl");
    Shader hdrShader("shaders/vertex/2d_tex.glsl", "shaders/fragment/lighting/hdr.glsl");

    // Set up uniforms and instancing buffer data
    const glm::vec3 lightColors[4] = {
        glm::vec3(200.0f, 200.0f, 200.0f),
        glm::vec3(0.1f,   0.0f,   0.0f),
        glm::vec3(0.0f,   0.0f,   0.2f),
        glm::vec3(0.0f,   0.1f,   0.0f),
    };

    const glm::vec3 lightPositions[4] = {
        glm::vec3(0.0f, 0.0f, -4.9f),
        glm::vec3(0.1f, -0.1f, 3.25f),
        glm::vec3(0.0f, -0.2f,  3.75f),
        glm::vec3(-0.1f, -0.1f, 3.5f),
    };

    // Set up framebuffers
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // generate texture to render to
    unsigned int colorBuffer;
    glGenTextures(1, &colorBuffer);

    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate renderbuffer for depth
    unsigned int hdrRBO;
    glGenRenderbuffers(1, &hdrRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBO);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
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

    // Get tangent space vectors
    const auto [tangent1, bitangent1, tangent2, bitangent2] = computeWallTangentSpace();

    const float wallVertices[] = {
        // positions         // normals         // tex coords // tangents                        // bitangents
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,   tangent1.x, tangent1.y, tangent1.z,  bitangent1.x, bitangent1.y, bitangent1.z,
        0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f,   tangent1.x, tangent1.y, tangent1.z,  bitangent1.x, bitangent1.y, bitangent1.z,
        0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f, 0.0f,   tangent1.x, tangent1.y, tangent1.z,  bitangent1.x, bitangent1.y, bitangent1.z,
        0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f,   tangent2.x, tangent2.y, tangent2.z,  bitangent2.x, bitangent2.y, bitangent2.z,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,   tangent2.x, tangent2.y, tangent2.z,  bitangent2.x, bitangent2.y, bitangent2.z,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,   tangent2.x, tangent2.y, tangent2.z,  bitangent2.x, bitangent2.y, bitangent2.z,
    };

    const float floorVertices[] = {
        -20.0f,  0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, -3.0f,
        20.0f,  0.0f,  20.0f,   0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
        20.0f,  0.0f, -20.0f,   0.0f, 1.0f, 0.0f,  5.0f, -3.0f,
        20.0f,  0.0f,  20.0f,   0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
        -20.0f,  0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, -3.0f,
        -20.0f,  0.0f,  20.0f,  0.0f, 1.0f, 0.0f,  -3.0f, 5.0f,
    };

    const float quadVertices[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,  // Top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // Bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // Bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // Top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // Bottom-right
        1.0f,  1.0f,   1.0f, 1.0f,   // Top-right
    };


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
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

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
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // wall VAO
    unsigned int wallVAO, wallVBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);

    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

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
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Configure texturing and load textures
    // ----------------------------------------------------------------------------
    const unsigned int roomTexture = TextureFromFile("resources/textures/wood.png", GAMMA_CORRECTED);

    // Configure shaders
    glActiveTexture(GL_TEXTURE0);

    objectShader.use();
    objectShader.setInt("material.diffuse", 0);
    objectShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
    objectShader.setFloat("material.shininess", 64.0f);

    objectShader.setVec3("lights[0].position", lightPositions[0]);
    objectShader.setVec3("lights[0].diffuse", lightColors[0]);
    objectShader.setVec3("lights[0].specular", 0.0f, 0.0f, 0.0f);

    objectShader.setVec3("lights[1].position", lightPositions[1]);
    objectShader.setVec3("lights[1].diffuse", lightColors[1]);
    objectShader.setVec3("lights[1].specular", 0.0f, 0.0f, 0.0f);

    objectShader.setVec3("lights[2].position", lightPositions[2]);
    objectShader.setVec3("lights[2].diffuse", lightColors[2]);
    objectShader.setVec3("lights[2].specular", 0.0f, 0.0f, 0.0f);

    objectShader.setVec3("lights[3].position", lightPositions[3]);
    objectShader.setVec3("lights[3].diffuse", lightColors[3]);
    objectShader.setVec3("lights[3].specular", 0.0f, 0.0f, 0.0f);

    objectShader.setVec3("ambient", 0.1f, 0.1f, 0.1f);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);
    hdrShader.setFloat("exposure", 0.15f);

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

        // Scene
        // -----
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat3 normalModel = glm::mat3(1.0f);

        // Tunnel
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 10.0f));
        normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

        objectShader.use();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        objectShader.setMat4("model", model);
        objectShader.setMat3("normalModel", normalModel);

        objectShader.setVec3("viewPos", camera.Position);

        glBindTexture(GL_TEXTURE_2D, roomTexture);
        glBindVertexArray(outCubeVAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render to the main framebuffer
        // ------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        // Apply tone mapping
        hdrShader.use();

        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glBindVertexArray(quadVAO);

        glDrawArrays(GL_TRIANGLES, 0, 6);

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
