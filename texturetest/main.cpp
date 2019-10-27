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
#include "../lib/Texture.h"

using namespace std;

bool wireframe = false;
const int WIDTH = 800;
const int HEIGHT = 600;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(WIDTH, HEIGHT, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    cout << "DONE\n" << flush;
    
    FloatCam camera({ 4.0, 3.0, 3.0 }, 1.0, 1366, 768, 0.07, window);
    camera.setLookAt({ 0.0, 0.0, 0.0 });

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

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    vector<GLfloat> vertex_data = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    vector<GLfloat> uv_data = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
    };

    GLuint vertex_buffer_id;
    glGenBuffers(1, &vertex_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * 4, vertex_data.data(), GL_STATIC_DRAW);

    GLuint uv_buffer_id;
    glGenBuffers(1, &uv_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, uv_data.size() * 4, uv_data.data(), GL_STATIC_DRAW);

    // grab our shader stuff
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");

    //Texture texture_object("../assets/textures/exampletexture.txt", TEXTURE_CUSTOM_TXT, GL_TEXTURE0 + 0);
    Shader  texture_shader("texturevertex", "texturefragment");

    //GLfloat texture_data[] = {
    //    0.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f,
    //    1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 0.0f
    //};

    uint8_t texture_data[] = {
        255, 0, 0,          0, 255, 0,
        0, 0, 255,          0, 0, 0
    };

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load the texture data
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texture_data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.5, 1.0, 2.5 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(66.0f), // FOV 66.0 degrees
            //float(1366.0/768.0),  // aspect ratio
            float(WIDTH)/float(HEIGHT),
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto mvp = Projection * View * Model;
    auto mvp_index = texture_shader.registerUniform("MVP", mvp);

    //GLuint texture_id         = texture_object.getTextureId();
    GLuint texture_sampler_id = glGetUniformLocation(texture_shader.getShaderId(), "texture_sampler");

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
        texture_shader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));
        texture_shader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glUniform1i(texture_sampler_id, 0);

        // prepare vertex position data
        glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
        glVertexAttribPointer(
            0,          // attribute 0 (must match layout in shader)
            3,          // size
            GL_FLOAT,   // type
            GL_FALSE,   // normalized
            0,          // stride
            (void*)0    // array buffer offset
        );

        // prepare UV data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_id);
        glVertexAttribPointer(
            1,        // attribute 1
            2,        // size (2 floats / uv coordinate pair) 
            GL_FLOAT, // type
            GL_FALSE, // normalized
            0,        // stride
            (void*)0  // array buffer offset
        );

        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 vertices, 1 triangle

        glfwSwapBuffers(window);
        glfwPollEvents();

        iter_time = current_time;

    }

    glfwDestroyWindow(window);
    glfwTerminate();

}
