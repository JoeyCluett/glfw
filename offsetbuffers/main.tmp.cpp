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
#include "../lib/IndexedModel.h"
#include "../lib/FloatCam.h"

using namespace std;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(1366, 768, "Hello World", {4, 0});
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cout << "DONE\n" << flush;
    
    //FloatCam camera({ 0.0, 0.0, 2.0 }, 1.0, 1366, 768, 0.1, window);
    //camera.setOrientation(M_PI, 0.0f);

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

    SimpleModelParser::setFileLocation("../assets/models/");

    GLuint VERTEX_ARRAY_ID;
    GLuint VERTEX_DATA_ID;
    GLuint VERTEX_OFFSET_ID;
    int num_indices = -1;

    glGenVertexArrays(1, &VERTEX_ARRAY_ID);
    glBindVertexArray(VERTEX_ARRAY_ID);

    try {

        SimpleModelParser smp("tank.txt");
        IndexedModel im(smp, "toplevel.group");
        auto md = smp.getExportedModelData("toplevel.group");

        cout << "Num raw tris:    " << md.second << endl;
        cout << "Num unique tris: " << im.vertexData().size()/3 << endl;
        
        // load vertex data
        glGenBuffers(1, &VERTEX_DATA_ID);                
        glBindBuffer(GL_ARRAY_BUFFER, VERTEX_DATA_ID);
        glBufferData(
                GL_ARRAY_BUFFER, 
                im.vertexData().size() * sizeof(GLfloat), 
                im.vertexData().data(), 
                GL_STATIC_DRAW);

        // load vertex offset data
        glGenBuffers(1, &VERTEX_OFFSET_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VERTEX_OFFSET_ID);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            im.vertexOffsets().size() * sizeof(unsigned int),
            im.vertexOffsets().data(),
            GL_STATIC_DRAW);

        num_indices = im.vertexOffsets().size();
    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        cout << up.what() << endl;

        throw up; // lol
        exit(1);
    }

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");
    Shader mvpshader("indexed-vertex", "indexed-fragment");

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.0, 1.0, 2.5 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(45.0f), // FOV 45.0 degrees
            float(1366.0/768.0),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto mvp = Projection * View * Model;
    auto mvp_index = mvpshader.registerUniform("MVP", mvp);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 newModel = 
            glm::rotate(
                glm::mat4(1.0f), 
                (float)(glfwGetTime() / 2.0), 
                glm::vec3(0.0f, 1.0f, 0.0f));

        auto new_mvp = Projection * View * newModel;
        mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));
        mvpshader.use();

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VERTEX_DATA_ID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VERTEX_OFFSET_ID);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, (void*)0);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

