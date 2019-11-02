#include <iostream>
#include <map>
#include <functional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../lib/Initialization.h"
#include "../lib/Texture.h"
#include "../lib/Shader.h"
#include "../lib/SimpleModel.h"
#include "../lib/FloatCam.h"
#include "../lib/Mesh.h"

using namespace std;

const int WIDTH         = 800;
const int HEIGHT        = 600;
const int GRIDHEIGHT    = 50;
const int GRIDWIDTH     = 50;
const int TEXTUREHEIGHT = 32;
const int TEXTUREWIDTH  = 32;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(WIDTH, HEIGHT, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    cout << "DONE\n" << flush;
    
    FloatCam camera({ 2.0, 2.0, 2.0 }, 6.0, WIDTH, HEIGHT, 0.07, window);
    camera.setBounds({ -1.0f, -1.0f, -1.0f }, { GRIDWIDTH, 20.0f, GRIDHEIGHT });

    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
        }
    );

    glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    Mesh truck_mesh(MESHTYPE_STATIC_SIMPLE, {});

    auto iter_time = glfwGetTime();
    while(
            !glfwWindowShouldClose(window) && 
            glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        glfwSwapBuffers(window);
        glfwPollEvents();


    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

