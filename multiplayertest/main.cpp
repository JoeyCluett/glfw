#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../lib/Initialization.h"
#include "../lib/Shader.h"
#include "../lib/SimpleModel.h"
#include "../lib/FloatCam.h"

#include "main.h"

using namespace std;

bool wireframe = false;

// screen
const int WIDTH = 800;
const int HEIGHT = 600;

// slection grid
const int GRIDHEIGHT = 20;
const int GRIDWIDTH  = 20;

// texture
const int TEXTUREHEIGHT = 400;
const int TEXTUREWIDTH  = 400;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(WIDTH, HEIGHT, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    cout << "DONE\n" << flush;
    
    FloatCam camera({ 2.0, 2.0, 2.0 }, 6.0, WIDTH, HEIGHT, 0.07, window);
    camera.setBounds({ -1.0f, 1.0f, -1.0f }, { 20.0f, 20.0f, 20.0f });

    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
        }
    );

    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // some libs need to know where to find stuff
    SimpleModelParser::setFileLocation("../assets/models/");
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");

    // palce to store the model info we'll actually need later
    struct {

        ModelInfo truck;
        ModelInfo mine;
        ModelInfo tower;
        ModelInfo gridlines;
        ModelInfo groundtexture;

        vector<glm::vec3> truck_tfs;
        vector<glm::vec3> mine_tfs;
        vector<glm::vec3> tower_tfs;

    } modelinfo;

    // load all of the models we will be using
    try {

        {
            SimpleModelParser smp("tank.txt");
            modelinfo.truck = smp.loadExportedModelIntoRuntime("toplevel.tank");
        }

        {
            SimpleModelParser smp("mine.txt");
            modelinfo.truck = smp.loadExportedModelIntoRuntime("toplevel.mine");
        }

        {
            SimpleModelParser smp("tower.txt");
            modelinfo.truck = smp.loadExportedModelIntoRuntime("toplevel.tower");
        }

        {
            // 20x20 grid, each cell is 1 unit x 1 unit
            auto v = generate_grid(20, 20, 1.0f, 1.0f);
            modelinfo.gridlines = SimpleModelParser::loadForeignModel(v);
        }

        {
            // load the triangles that the texture will sit on

        }
    }
    catch(runtime_error& up) {
        cout << up.what() << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(1);
    }

    // place to store shader data
    struct {

        Shader* grid; // will load ../assets/shaders/grid.vertex.glsl and ../assets/shaders/grid.fragment.glsl
        Shader* landtex;

    } shaders;
    shaders.grid    = new Shader("grid");
    shaders.landtex = new Shader("landtexture");

    // place to store texture data
    struct {

        Texture* ground;

    } textures;

    // generate the texture data used for the land portion of the ground
    try {

        vector<uint8_t> texture_data;
        vector<uint8_t> texture_data;
        srand(time(NULL));
        uint8_t browns[] = {
            161, 64, 5,  // 0
            145, 41, 3,  // 1
            117, 26, 3,  // 2
            97,  26, 9,  // 3
            102, 27, 4,  // 4
            51,  16, 8,  // 5
            77,  31, 10, // 6
            37,  14, 3,  // 7
            46,  16, 3,  // 8
            135, 64, 8,  // 9
            133, 60, 8,  // 10
            122, 49, 7,  // 11
            87,  45, 9,  // 12 last brown
            120, 102, 47, // 13
            125, 102, 24, // 14
            94,  89,  25, // 15
            64,  69,  25, // 16
            89,  80,  19, // 17
            110, 81,  22, // 18
            74,  63,  21, // 19
            78,  62,  21, // 20
            158, 149, 56, // 21
            176, 189, 72, // 22
            166, 200, 64, // 23
            179, 213, 34, // 24
        };



    }
    catch(runtime_error& up) {
        cout << up.what() << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(1);
    }


    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = // this changes every frame but we have to initialize the Shader with something
        glm::lookAt(
            glm::vec3( 0.5, 1.0, 2.5 ),    // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(66.0f), // FOV 66.0 degrees
            float(WIDTH)/float(HEIGHT),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane
    
    auto iter_time = glfwGetTime();
    while(
            !glfwWindowShouldClose(window) && 
            glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto current_time = glfwGetTime();
        auto deltaT = current_time - iter_time;

        // new View tf matrix comes from the camera
        camera.update((float)deltaT);
        auto new_mvp = Projection * camera.getTf() * Model;
        
        // all of the rendering logic happens here



        glfwSwapBuffers(window);
        glfwPollEvents();
        iter_time = current_time;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

