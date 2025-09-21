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
#include "PBR_setup.h"
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
    loadFont("fonts/arial.ttf");

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Blending needed for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

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

    Shader skyboxShader("shaders/vertex/cubemap.glsl", "shaders/fragment/cubemap/skyboxhrd.glsl");

    Shader textShader("shaders/text/vertex/text.glsl", "shaders/text/fragment/text.glsl");

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

    // Load textures
    // -------------
    const unsigned int MATERIAL_COUNT = 3;
    const PBRMaterial materials[MATERIAL_COUNT] = {
        PBRMaterial("resources/textures/PBR_materials/carbon-fiber"),
        PBRMaterial("resources/textures/PBR_materials/gold-scuffed"),
        PBRMaterial("resources/textures/PBR_materials/rusted_iron"),
    };

    // Load models
    // -----------
    const PBRMaterial gunMaterial = PBRMaterial("resources/textures/PBR_materials/gun");
    Model gun("resources/models/Cerberus_gun/Cerberus_LP.FBX", ModelLoad_CustomTex);

    // PBR framebuffers and textures
    // -----------------------------
    const auto [gBuffer,
                gPositionMetallic,
                gNormalRoughness,
                gAlbedoAo,
                brdfLUTTexture]
    = PBR_deferredFramebuffersSetup3x4f(SCR_WIDTH, SCR_HEIGHT);

    const auto [irradianceMap,
                prefilterMap,
                envCubemap]
    = generateIBLCubemaps_env("resources/textures/equirectangular/ibl_hdr_radiance.png");

    // Configure shaders
    // -----------------
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

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        gPassPBRShader.use();
        gPassPBRShader.setMat4("projection", projection);
        gPassPBRShader.setMat4("view", view);

        for (int i = 0; i < MATERIAL_COUNT; ++i)
        {
            // render material spheres
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, materials[i].albedoTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, materials[i].normalTexture);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, materials[i].metallicTexture);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, materials[i].roughnessTexture);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, materials[i].aoTexture);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(3.0f * (i - (MATERIAL_COUNT - 1) / 2.0f), 0.0f, 0.0f));

            gPassPBRShader.setMat4("model", model);
            gPassPBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();
        }

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
        glBindTexture(GL_TEXTURE_2D, gNormalRoughness);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoAo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

        renderQuad();

        // Additional rendering
        // --------------------

        // Copy depth from gBuffer to default framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);

        // Skybox
        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        renderCube();

        // Render text
        projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);

        textShader.use();
        textShader.setMat4("projection", projection);
        textShader.setInt("text", 0);

        glEnable(GL_BLEND);
        RenderText(textShader, "SERUS", 320.0f, 280.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        glDisable(GL_BLEND);

        DisplayFramebufferTexture(gAlbedoAo);

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
