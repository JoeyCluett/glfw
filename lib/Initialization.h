#pragma once

#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>
#include <utility>
#include <vector>

GLFWwindow* GLFWINITWINDOW(
        int width, int height, 
        const char* title, 
        std::pair<int, int> ver, int samples = 4, bool fullscreen = false) {

    std::cout << "    Setting error callback..." << std::flush;
    glfwSetErrorCallback(
        [](int error, const char* desc) -> void {
            std::cout << "error [" << error << "] : " << desc << std::endl;
        }
    );
    std::cout << "  DONE\n" << std::flush;

    std::cout << "    Initializing GLFW..." << std::flush;
    if(!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        exit(1);
    }
    std::cout << "       DONE\n" << std::flush;

    std::cout << "    Setting GLFW window hints...\n";
    glfwWindowHint(GLFW_SAMPLES,               samples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver.first);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ver.second);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);

    std::cout << "        GLFW Context Version: " << ver.first << "." << ver.second << "\n";
    std::cout << "        GLFW Samples:         " << samples << "\n";

    std::cout << "    DONE\n";


    // create the monitor according to input args
    std::cout << "    Creating GLFW window..." << std::flush;
    GLFWwindow* window = NULL;

    if(fullscreen)
        window = glfwCreateWindow(width, height, title, glfwGetPrimaryMonitor(), NULL);
    else
        window = glfwCreateWindow(width, height, title, NULL, NULL);
    
    if(!window) {
        std::cout << "Failed to create OpenGL context\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(1);
    }
    std::cout << "    DONE\n" << std::flush;

    glfwMakeContextCurrent(window);

    std::cout << "    Initializing GLEW..." << std::flush;
    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(1);
    }
    std::cout << "       DONE\n" << std::flush;

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSwapInterval(1);

    return window;
}

