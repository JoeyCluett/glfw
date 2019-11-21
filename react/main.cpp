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

    glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);


    Texture* ground_tex;
    ModelInfo box;
    ModelInfo ground;
    ModelInfo ground_uv;

    // load the texture data
    {
        vector<uint8_t> texture_data;
        srand(time(NULL));

        for(int i = 0; i < TEXTUREHEIGHT*TEXTUREWIDTH; i++) {
            int c = rand() % 256;
            texture_data.insert(texture_data.end(), { uint8_t(c), uint8_t(c), uint8_t(c) });
        }

        ground_tex =
            new Texture(
                texture_data,
                TEXTUREHEIGHT,
                TEXTUREWIDTH,
                GL_TEXTURE0,
                GL_REPEAT,
                GL_LINEAR);

        vector<GLfloat> texture_triangles = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, GRIDHEIGHT,
            GRIDWIDTH, 0.0f, 0.0f,

            GRIDWIDTH, 0.0f, 0.0f,
            GRIDWIDTH, 0.0f, GRIDHEIGHT,
            0.0f, 0.0f, GRIDHEIGHT
        };

        for(int i = 0; i < (int)texture_triangles.size(); i += 3) {
            texture_triangles[i]   -= 0.5f;
            texture_triangles[i+2] -= 0.5f;
        }

        ground = SimpleModelParser::loadForeignModelIntoRuntime(texture_triangles);

        vector<GLfloat> texture_coords = {
            0.0f,      0.0f,
            0.0f,      GRIDHEIGHT,
            GRIDWIDTH, 0.0f, //1.0f, 0.0f,

            GRIDWIDTH, 0.0f,
            GRIDWIDTH, GRIDHEIGHT,
            0.0f,      GRIDHEIGHT
        };

        ground_uv = SimpleModelParser::loadForeignModelIntoRuntime(texture_coords);

    }

    // load the model into the environment
    {
        SimpleModelParser::setFileLocation("../assets/models/");
        SimpleModelParser::loadModelList({
            { "box.txt", "box.box", &box }
        });
    }

    while(!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
