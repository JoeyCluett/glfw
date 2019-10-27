#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "main.h"
#include "../lib/Initialization.h"
#include "../lib/Shader.h"

using namespace std;

int main(int argc, char* argv[]) {
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

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    static const GLfloat vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };

    static const GLfloat vertex_color_data[] = {
        1.0f, 0.0f, 0.0f, // red
        0.0f, 1.0f, 0.0f, // green
        0.0f, 0.0f, 1.0f, // blue
    };

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    GLuint color_buffer;
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color_data), vertex_color_data, GL_STATIC_DRAW);

    Shader::setVertexShaderDirectory("../shaders/");
    Shader::setFragmentShaderDirectory("../shaders/");

    // create the shader programs used here
    Shader helloworldshader("helloworldvertex", "helloworldfragment");
    Shader mvpshader("mvpvertex", "mvpfragment");

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.0, -1.0, -3.0 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(45.0f), // FOV 45.0 degrees
            float(800.0/600.0),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto mvp = Projection * View * Model;
    auto mvp_index = mvpshader.registerUniform("MVP", mvp);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 newModel = 
            glm::rotate(
                glm::mat4(1.0f), 
                (float)glfwGetTime(), 
                glm::vec3(0.0f, 1.0f, 0.0f));

        auto new_mvp = Projection * View * newModel;
        mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));

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

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

