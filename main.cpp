#include <iostream>
#include <string>
#include <cmath>

// Glad sa importuje pred glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "texture_loader.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Update viewportu
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // cursor movement tracking
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // scrolling tracking
void processInput(GLFWwindow* window);


// Settings
float SCR_WIDTH = 800.0f;
float SCR_HEIGHT = 600.0f;

// For sphere rendering
const unsigned int SEGMENTS = 20;
const float RADIUS = 0.5f;

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

    // Load shader porgrams
    // --------------------
    Shader modelShader("shaders/vertex/3d_model.glsl","shaders/fragment/model.glsl");

    Model backpack((char*)"resources/objects/Backpack/backpack.obj", true);

    // Declare uniforms
    // glm::vec3 lightColor(0.89f, 0.376f, 0.008f);

    // Set uniforms
    // objectShader.use();
    // objectShader.setInt("material.diffuse", 0);
    // objectShader.setInt("material.specular", 1);
    // objectShader.setFloat("material.shininess", 32.0f);
    // objectShader.setVec3("ambient",  0.1f, 0.1f, 0.1f);

    // objectShader.setVec3("dirLight.direction", 0.2f,-1.0f, 0.3f);
    // objectShader.setVec3("dirLight.diffuse",  0.0f, 0.0f, 0.0f);
    // objectShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

    // for (int i = 0; i < 4; i++)
    // {
    //     std::string index = std::to_string(i);
        
    //     objectShader.setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
    //     objectShader.setFloat("pointLights[" + index + "].constant", 1.0f);
    //     objectShader.setFloat("pointLights[" + index + "].linear", 0.35f);
    //     objectShader.setFloat("pointLights[" + index + "].quadratic", 0.44f);
    //     objectShader.setVec3("pointLights[" + index + "].diffuse",  0.445f, 0.188f, 0.004f);
    //     objectShader.setVec3("pointLights[" + index + "].specular", 0.89f, 0.376f, 0.008f);
    // }

    // objectShader.setVec3("spotLight.position", camera.Position);
    // objectShader.setVec3("spotLight.direction", camera.Front);
    // objectShader.setFloat("spotLight.constant", 1.0f);
    // objectShader.setFloat("spotLight.linear", 0.22f);
    // objectShader.setFloat("spotLight.quadratic", 0.20f);
    // objectShader.setFloat("spotLight.cutOff", std::cos(glm::radians(12.5f)));
    // objectShader.setFloat("spotLight.outerCutOff", std::cos(glm::radians(17.5f)));
    // objectShader.setVec3("spotLight.diffuse",  0.445f, 0.188f, 0.004f);
    // objectShader.setVec3("spotLight.specular", 0.89f, 0.376f, 0.008f);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // Vertices with normal vectors
    // float cubeVertices[] = {
    //     // positions          // normals           // texture coords
    //     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    //     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    //     -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    //     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    //     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    //     -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    //     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    //     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    //     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    //     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    //     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    //     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    //     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    //     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    //     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    //     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    //     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    //     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    //     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    //     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    //     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    // };

    // glm::vec3 cubePositions[] = {
    //     glm::vec3( 0.0f,  0.0f,  0.0f),
    //     glm::vec3( 2.0f,  5.0f, -15.0f),
    //     glm::vec3(-1.5f, -2.2f, -2.5f),
    //     glm::vec3(-3.8f, -2.0f, -12.3f),
    //     glm::vec3( 2.4f, -0.4f, -3.5f),
    //     glm::vec3(-1.7f,  3.0f, -7.5f),
    //     glm::vec3( 1.3f, -2.0f, -2.5f),
    //     glm::vec3( 1.5f,  2.0f, -2.5f),
    //     glm::vec3( 1.5f,  0.2f, -1.5f),
    //     glm::vec3(-1.3f,  1.0f, -1.5f)
    // };

    // float sphereVertices[3 * (SEGMENTS + 1) * (SEGMENTS + 1)];
    // int index = 0;
    // for (int i = 0; i <= SEGMENTS; ++i) {
    //     float phi = glm::pi<float>() * i / SEGMENTS;
    //     for (int j = 0; j <= SEGMENTS; ++j) {
    //         float theta = glm::two_pi<float>() * j / SEGMENTS;

    //         float x = RADIUS * std::sin(phi) * std::cos(theta);
    //         float y = RADIUS * std::cos(phi);
    //         float z = RADIUS * std::sin(phi) * std::sin(theta);

    //         sphereVertices[index++] = x;
    //         sphereVertices[index++] = y;
    //         sphereVertices[index++] = z;
    //     }
    // }

    // unsigned int sphereIndices[6 * SEGMENTS * SEGMENTS];
    // index = 0;
    // for (int i = 0; i < SEGMENTS; ++i) {
    //     for (int j = 0; j < SEGMENTS; ++j) {
    //         int first = i * (SEGMENTS + 1) + j;
    //         int second = first + SEGMENTS + 1;

    //         sphereIndices[index++] = first;
    //         sphereIndices[index++] = second;
    //         sphereIndices[index++] = first + 1;

    //         sphereIndices[index++] = second;
    //         sphereIndices[index++] = second + 1;
    //         sphereIndices[index++] = first + 1;
    //     }
    // }

    // Generovanie vertex buffer objektu, element buffer objektu a vertex array objektu,
    // ktory uchovava konfiguraciu: EBO, VBO, vertex attribute pointer (ako ma spracovat vertexy)
    // unsigned int objectVAO;
    // unsigned int objectVBO;
    // glGenVertexArrays(1, &objectVAO);
    // glGenBuffers(1, &objectVBO);

    // Inicializacny kod
    
    // bind the Vertex Array Object first, then bind and set vertex and element buffer(s), and then configure vertex attributes(s).
    // glBindVertexArray(objectVAO);

    // glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Nastavime ako ma OpenGL spracovat nase vertex atributy na locations

    // 1st parameter: To which location of the shader program will the attribute be sent
    // 2nd parameter: The size of the attribute
    // 3rd parameter: Datatype of the attribute items
    // 4th parameter: Wheter we want to normalize the datatype
    // 5th parameter: The space from the start of the first attribute to the next one
    // 6th parameter: The offset from the start of the buffer to the start of the first attribute

    // Position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);

    // unsigned int lightVAO;
    // unsigned int lightVBO;
    // unsigned int lightEBO;
    // glGenVertexArrays(1, &lightVAO);
    // glGenBuffers(1, &lightVBO);
    // glGenBuffers(1, &lightEBO);
    
    // glBindVertexArray(lightVAO);

    // glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);
    // set the vertex attribute 
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // Configure texturing and load a texture
    // ----------------------------------------------------------------------------

    // Generate textures and its object and bind it
    // unsigned int objectDiffuseMap = TextureFromFile("images/container.png");
    // unsigned int objectSpecularMap = TextureFromFile("images/container_spec.png");

    // Set the texture number (pointer)
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, objectDiffuseMap);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, objectSpecularMap);

    // Coordinate systems
    // ------------------

    // Matrices that dont change are defined and sent to the shader here

    // projection matrix

    // view matrix

    // model matrix
    
    // Rendering loop
    // --------------
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        // -----
        processInput(window);

        // Rendering
        // ---------

        // Nastavenie pozadia
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // View matrix = camera
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 model = glm::mat4(1.0f);

        modelShader.use();
        modelShader.setMatrix4f("model", glm::value_ptr(model));
        modelShader.setMatrix4f("projection", glm::value_ptr(projection));
        modelShader.setMatrix4f("view", glm::value_ptr(view));

        backpack.Draw(modelShader);

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
