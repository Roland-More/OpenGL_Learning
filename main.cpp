#include <iostream>

#include <glad/glad.h> // Glad sa importuje pred glfw
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
    Shader normalShader("shaders/vertex/normal_visual.glsl", "shaders/fragment/simple_colors/yellow.glsl", "shaders/geometry/normal_visual.glsl");

    // Load models
    Model backpack((char*)"resources/models/backpack/backpack.obj", true);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // Configure texturing and load a texture
    // ----------------------------------------------------------------------------

    // Configure shaders
    modelShader.use();
    
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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(view * model)));
    
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setMat4("model", model);

        backpack.Draw(modelShader);

        normalShader.use();
        normalShader.setMat4("projection", projection);
        normalShader.setMat4("view", view);
        normalShader.setMat4("model", model);
        normalShader.setMat3("normalMat", normalMat);

        backpack.Draw(normalShader);

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
