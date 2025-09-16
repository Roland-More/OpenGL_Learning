#ifndef DEBUGGING_H
#define DEBUGGING_H

#include <iostream>
#include <string>

#include <glad/glad.h>

#include "shader.h"

// Simple light error checker
GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// EXTENSION OR OPENGL 4.3+ REQUIRED FOR THE FOLLOWING TO WORK:

// Heavier debug output
// Adding breakpoints to the specific error types or just the top of the function
// should help with backtracking the exact error location

// When using debug output add this before creating window:

    // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

// Then add this after creating window:

    // if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    // {
    //     glEnable(GL_DEBUG_OUTPUT);
    //     glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    //     glDebugMessageCallback(glDebugOutput, nullptr);
    //     glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    // }

// And uncomment this:
// void APIENTRY glDebugOutput(GLenum source, 
//                             GLenum type, 
//                             unsigned int id, 
//                             GLenum severity, 
//                             GLsizei length, 
//                             const char *message, 
//                             const void *userParam)
// {
//     // ignore non-significant error/warning codes
//     if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

//     std::cout << "---------------" << std::endl;
//     std::cout << "Debug message (" << id << "): " <<  message << std::endl;

//     switch (source)
//     {
//         case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
//         case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
//         case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
//         case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
//         case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
//         case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
//     } std::cout << std::endl;

//     switch (type)
//     {
//         case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
//         case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
//         case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
//         case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
//         case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
//         case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
//         case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
//         case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
//         case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
//     } std::cout << std::endl;
    
//     switch (severity)
//     {
//         case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
//         case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
//         case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
//         case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
//     } std::cout << std::endl;
//     std::cout << std::endl;
// }

// Custom debug output example:

    // glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0,                       
    //                      GL_DEBUG_SEVERITY_MEDIUM, -1, "error message here");

// Framebuffer texture attachment debugging:
void DisplayFramebufferTexture(unsigned int textureID)
{
    static bool initialized = false;
    static unsigned int shaderDisplayFBOOutput = 0;
    static unsigned int vaoDebugTexturedRect = 0;

    if (!initialized)
    {
        // Shader creation and configuration
        Shader shaderCreation("shaders/debug/vertex/framebuffer.glsl", "shaders/debug/fragment/framebuffer.glsl");
        shaderCreation.use();
        shaderCreation.setInt("fboAttachment", 0);

        shaderDisplayFBOOutput = shaderCreation.ID;

        // VAO creation and configuration
        const float vertices[] = {
            0.25f, 0.25f,       0.0f, 0.0f,  // Bottom-left
            0.9375f, 0.25f,     1.0f, 0.0f,  // Bottom-right
            0.25f,  0.9375f,    0.0f, 1.0f,  // Top-left

            0.25f,  0.9375f,    0.0f, 1.0f,  // Top-left
            0.9375f, 0.25f,     1.0f, 0.0f,  // Bottom-right
            0.9375f,  0.9375f,  1.0f, 1.0f,   // Top-right
        };

        unsigned int vbo;
        glGenVertexArrays(1, &vaoDebugTexturedRect);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vaoDebugTexturedRect);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        initialized = true;
    }
  
    glActiveTexture(GL_TEXTURE0);  	
    glUseProgram(shaderDisplayFBOOutput);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(vaoDebugTexturedRect);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    glUseProgram(0);
}

#endif
