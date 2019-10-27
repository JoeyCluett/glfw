#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "main.h"
#include "../lib/Initialization.h"
#include "../lib/Shader.h"
#include "../lib/SimpleModel.h"
#include "../lib/FloatCam.h"

using namespace std;

bool wireframe = false;

int main(int argc, char* argv[]) {

    if(argc != 3) {
        cout << "specify a file and a top-level entity\n";
        return 1;
    }

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(1366, 768, "Hello World", {4, 0});
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    cout << "DONE\n" << flush;
    
    FloatCam camera({ 0.0, 0.0, 2.0 }, 1.0, 1366, 768, 0.07, window);
    //camera.setOrientation(M_PI, 0.0f);
    camera.setLookAt({ 0.0f, 0.0f, 0.0f });

    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);

            else if(key == GLFW_KEY_Q && action == GLFW_PRESS)
                wireframe = (wireframe ? false : true);

        }
    );


    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    pair<vector<float>, int> mv; // model data

    try {
        SimpleModelParser::setFileLocation("../assets/models/");
        SimpleModelParser smp(argv[1]);
        mv = smp.getExportedModelData(argv[2]);
    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        cout << up.what() << endl << flush;

        throw up; // lol
        exit(1);
    }

    GLfloat*        vertex_buffer_data = mv.first.data();
    vector<GLfloat> vertex_color_data;

    for(int i = 0; i < mv.second/3; i++) {
        auto& v = vertex_color_data;

        for(auto f : { 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f })
            v.push_back(f);
    }

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, mv.second*12, vertex_buffer_data, GL_STATIC_DRAW);

    GLuint color_buffer;
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, mv.second*12, vertex_color_data.data(), GL_STATIC_DRAW);

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");
    Shader mvpshader("mvpvertex", "mvpfragment");

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.5, 1.0, 2.5 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(66.0f), // FOV 66.0 degrees
            float(1366.0/768.0),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto mvp = Projection * View * Model;
    auto mvp_index = mvpshader.registerUniform("MVP", mvp);

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
        mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));

        mvpshader.use();

        // prepare vertex position data
        glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(
            0,          // attribute 0 (must match layout in shader)
            3,          // size
            GL_FLOAT,   // type
            GL_FALSE,   // normalized
            0,          // stride
            (void*)0    // array buffer offset
        );

        // prepare vertex color data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
        glVertexAttribPointer(
            1,        // attribute 1
            3,        // size 
            GL_FLOAT, // type
            GL_FALSE, // normalized
            0,        // stride
            (void*)0  // array buffer offset
        );

        if(!wireframe)
            glDrawArrays(GL_TRIANGLES, 0, mv.second);
        else
            glDrawArrays(GL_LINES, 0, mv.second);
    
        //glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        iter_time = current_time;

    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

