#include <fstream>

#include <iostream>
#include "include/glad/gl.h"
#include <GLFW/glfw3.h>

#include "vao.h"
#include "shader.h"
#include "texture3d.h"
#include "camera.h"
#include <random>

int SCREEN_WIDTH = 960, SCREEN_HEIGHT = 960;
int GRID_WIDTH = 512, GRID_HEIGHT = 512, GRID_DEPTH = 512;

// E 13-26, F 14-19, spawn 50%
uint32_t surviveMask13_26 = 0x07ffe000;
uint32_t birthMask14_19 = 0x000fc000;

// E 5-7, F 4-4, spawn
uint32_t surviveMask5_7 = 0x000000e0; // environment E
uint32_t birthMask6 = 0x00000040; // fertility F

// E 4-5, F 5-5
uint32_t surviveMask5 = 0x00000030; // environment E
uint32_t birthMask7 = 0x00000020; // fertility F

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void seedGrid3D(GLuint textureID, float p) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution seed(p);

    size_t totalCells = (size_t)GRID_WIDTH * GRID_HEIGHT * GRID_DEPTH;
    std::vector<uint8_t> initialData(totalCells, 0);

    for (size_t i = 0; i < totalCells; i++) {
        if (seed(gen)) {
            initialData[i] = 1;
        }
    }

    glBindTexture(GL_TEXTURE_3D, textureID);
    glTexSubImage3D(GL_TEXTURE_3D, 0,
        0, 0, 0,
        GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH,
        GL_RED_INTEGER, GL_UNSIGNED_BYTE, initialData.data());
    glBindTexture(GL_TEXTURE_3D, 0);
}

int main() {
    glfwInit(); // initialize GLFW

    glfwSetErrorCallback([](int error, const char* description) {
        fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
    });

    // specify which OpenGL version to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // version 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5); // version x.5
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // core profile

    GLFWwindow* window = glfwCreateWindow(
        SCREEN_WIDTH, // width
        SCREEN_HEIGHT, // height
        "Basic window", // window title
        nullptr, // monitor for fullscreen mode, or nullptr for windowed mode
        nullptr // window whose context to share resources with
    );
    std::cout << "Window Created" << std::endl;

    if (window == nullptr) {
        // window creation failed
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    std::cout << "Window is current context" << std::endl;
    glfwSwapInterval(0);

    std::cout << "Loading Glad..." << std::endl;
    // load glad and check for errors
    if (!gladLoadGL(glfwGetProcAddress)) {
        return -1;
    }
    std::cout << "Glad successfully loaded" << std::endl;

    glViewport(
        0, 0, // bottom left corner of the render window
        SCREEN_WIDTH, SCREEN_HEIGHT // width & height of the render window
    );

    glDisable(GL_DEPTH_TEST);

    // specify what to call on window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // load, compile, and activate shaders
    std::cout << "Loading shaders..." << std::endl;
    RenderShader renderShader("resources/shaders/fullscreen.vert", "resources/shaders/raymarch.frag");
    ComputeShader computeShader("resources/shaders/simulation_2.comp");
    std::cout << "Shaders successfully loaded." << std::endl;

    // create & bind VAO
    VAO emptyVAO;
    emptyVAO.bind();

    // textures
    std::cout << "Loading textures..." << std::endl;

    // create 2 3D textures, so one is the current grid state, and
    // we use it to calculate the next grid state on the other 3D texture,
    // then we swap them and repeat
    Texture3D textureA(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH);
    Texture3D textureB(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH);
    std::cout << "Textures successfully loaded." << std::endl;

    std::cout << "Seeding texture..." << std::endl;
    seedGrid3D(textureA.id_, 0.2f);
    std::cout << "Texture successfully seeded." << std::endl;

    int simFrame = 0;
    int renderFrames = 0;
    double lastSimTime = glfwGetTime();
    double lastFpsTime = glfwGetTime();
    double simUpdateRate = 1.0 / 15.0;

    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT, glm::vec3(1.0f * GRID_DEPTH, 1.0f * GRID_DEPTH, 1.0f * GRID_DEPTH));

    GLuint timeQueries[2];
    glGenQueries(2, timeQueries);
    int i = 0;

    while (/*!glfwWindowShouldClose(window)*/ i < 1000) {
        i++;
        // SIMULATION PASS
        camera.inputs(window);
        double currentTime = glfwGetTime();

        glBindTexture(GL_TEXTURE_3D, 0);

        if (currentTime - lastSimTime >= simUpdateRate) {
            computeShader.activateShader();

            glUniform1ui(glGetUniformLocation(computeShader.id_, "surviveRules"), surviveMask5_7);
            glUniform1ui(glGetUniformLocation(computeShader.id_, "birthRules"), birthMask6);

            // find which texture is the current and which the next state of the grid
            GLuint readTexture = (simFrame % 2 == 0) ? textureA.id_ : textureB.id_;
            GLuint writeTexture = (simFrame % 2 == 0) ? textureB.id_ : textureA.id_;

            // bind textures for reading or writing
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, readTexture);
            glBindImageTexture(1, writeTexture,0,GL_TRUE,0,GL_WRITE_ONLY, GL_R8UI);

            // dispatch compute groups
            uint32_t clearVal = 0;
            glClearTexImage(writeTexture, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &clearVal);

            glBeginQuery(GL_TIME_ELAPSED, timeQueries[0]);
            glDispatchCompute(GRID_WIDTH / 8, GRID_HEIGHT / 8, GRID_DEPTH / 8);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            glEndQuery(GL_TIME_ELAPSED);

            simFrame++;
            lastSimTime = currentTime;
        }

        // RENDERING PASS
        glClear(GL_COLOR_BUFFER_BIT);
        renderShader.activateShader();

        camera.matrix(45.0f, 0.1f, 1000.0f, renderShader);

        GLuint readTexture = (simFrame % 2 == 0) ? textureA.id_ : textureB.id_;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, readTexture);

        int currentWindowWidth, currentWindowHeight;
        glfwGetWindowSize(window, &currentWindowWidth, &currentWindowHeight);
        glUniform2f(glGetUniformLocation(renderShader.id_, "resolution"), (float)currentWindowWidth, (float)currentWindowHeight);
        glUniform1i(glGetUniformLocation(renderShader.id_, "gridVolume"), 0);

        glBeginQuery(GL_TIME_ELAPSED, timeQueries[1]);
        emptyVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glEndQuery(GL_TIME_ELAPSED);

        glfwSwapBuffers(window); // swap buffers (displays newly rendered frame)
        glfwPollEvents(); // process inputs

        renderFrames++;
        if (currentTime - lastFpsTime >= 1.0) {
            GLuint64 computeTimeNs = 0;
            GLuint64 renderTimeNs = 0;

            // Pull the nanosecond timings from the GPU
            glGetQueryObjectui64v(timeQueries[0], GL_QUERY_RESULT, &computeTimeNs);
            glGetQueryObjectui64v(timeQueries[1], GL_QUERY_RESULT, &renderTimeNs);

            // Convert nanoseconds to milliseconds
            double computeMs = computeTimeNs / 1000000.0;
            double renderMs  = renderTimeNs / 1000000.0;

            std::cout << "GPU Compute: " << computeMs << " ms | GPU Render: " << renderMs << " ms" << std::endl;

            glfwSetWindowTitle(window, ("FPS: " + std::to_string(renderFrames)).c_str());
            renderFrames = 0;
            lastFpsTime = currentTime;
        }
    }

    std::cout << "Window closed." << std::endl;

    // delete objects
    emptyVAO.deleteVAO();
    renderShader.deleteShader();
    computeShader.deleteShader();
    textureA.deleteTexture();
    textureB.deleteTexture();

    glfwDestroyWindow(window);
    glfwTerminate(); // destroys remaining windows, frees resources

    return 0;
}