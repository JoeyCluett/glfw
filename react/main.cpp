#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <reactphysics3d/reactphysics3d.h>

#include "../lib/Initialization.h"
#include "../lib/Texture.h"
#include "../lib/Shader.h"
#include "../lib/SimpleModel.h"
#include "../lib/FloatCam.h"

using namespace std;
using namespace reactphysics3d;

const int GRIDHEIGHT    = 50;
const int GRIDWIDTH     = 50;
const int TEXTUREHEIGHT = 32;
const int TEXTUREWIDTH  = 32;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(800, 600, "Hello World", {3, 3});
    cout << "DONE\n" << flush;
    cout << "Hello ReactPhysics3D\n";

    glfwSetKeyCallback(
        window,
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
        }
    );

    Texture* ground_tex;

    // load the texture data
    {
        vector<uint8_t> texture_data;
        srand(time(NULL));

        for(int i = 0; i < TEXTUREHEIGHT*TEXTUREWIDTH; i++) {
            int c = rand() % 256;
            texture_data.insert(texture_data.end(), { uint8_t(c), uint8_t(c), uint8_t(c) });
        }

        ground_tex = new Texture(texture_data, TEXTUREHEIGHT, TEXTUREWIDTH, GL_TEXTURE0, GL_REPEAT, GL_LINEAR);
    }

    while(!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
