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

using namespace std;

int main(int argc, char* argv[]) {

    if(argc != 3) {
        cout << "specify a file and a top-level entity\n";
        return 1;
    }

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(800, 600, "Hello World", {4, 0});
    cout << "DONE\n" << flush;
    
    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
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
    catch(runtime_error& err) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        throw err; // lol
        exit(1);
    }
    catch(exception& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        throw up; // lol
        exit(1);
    }

    GLfloat*        vertex_buffer_data = mv.first.data();
    vector<GLfloat> vertex_color_data;
    cout << "generating normals..." << flush;
    vector<GLfloat> vertex_normal_data = SimpleModelParser::calculateNormals(mv.first);
    cout << endl << flush;

    for(int i = 0; i < mv.second/3; i++) {
        auto& v = vertex_color_data;

        //for(auto f : { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f })
        for(auto f : { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f })
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

    GLuint normal_buffer;
    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, mv.second*12, vertex_normal_data.data(), GL_STATIC_DRAW);

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");
    Shader mvpshader(
        "directional-light-vertex", 
        "directional-light-fragment");

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.0, 2.5, 4.0 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(45.0f), // FOV 45.0 degrees
            float(800.0/600.0),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto mvp = Projection * View;
    auto vp_index = mvpshader.registerUniform("VP", mvp);
    auto m_index  = mvpshader.registerUniform("M",  Model); // doesnt matter what we pass as it is overwritten every frame

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 newModel = 
            glm::rotate(
                glm::mat4(1.0f), 
                (float)(glfwGetTime()), 
                //0.0f,
                //(float)(M_PI/3.0),
                glm::vec3(0.0f, 1.0f, 0.0f));

        mvpshader.updateUniformData(m_index, reinterpret_cast<void*>(&newModel[0][0]));

        //helloworldshader.use();
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

        // prepare vertex normal data
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(
            2,        // attribute 2
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized
            0,        // stride
            (void*)0  // array buffer offset
        );

        glDrawArrays(GL_TRIANGLES, 0, mv.second);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}