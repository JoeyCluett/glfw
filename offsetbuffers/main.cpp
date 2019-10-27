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

void MessageCallback( 
        GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
    fprintf(
        stderr, 
        "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
        type, 
        severity, 
        message
    );
}

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(1366, 768, "Hello World", {4, 0});
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cout << "DONE\n" << flush;


    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);
        }
    );

    glfwPollEvents();

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    SimpleModelParser::setFileLocation("../assets/models/");
    
    GLuint VAO_ID;
    GLuint VERTEX_DATA_ID;
    GLuint VERTEX_OFFSET_ID;

    glGenVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    // sample data to load
    GLfloat verts[]        = { -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    unsigned int indices[] = { 0, 1, 2 };
    int num_indices = -1;

    try {

        SimpleModelParser smp("tank.txt");
        IndexedModel im(smp, "toplevel.group");
        auto md = smp.getExportedModelData("toplevel.group");

	    glGenBuffers(1, &VERTEX_DATA_ID);
	    glBindBuffer(GL_ARRAY_BUFFER, VERTEX_DATA_ID);
	    
        //glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &verts[0], GL_STATIC_DRAW);
        
        glBufferData(GL_ARRAY_BUFFER, im.vertexData().size() * 4, im.vertexData().data(), GL_STATIC_DRAW);
        num_indices = im.vertexData().size();

        
        glGenBuffers(1, &VERTEX_OFFSET_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VERTEX_OFFSET_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        cout << up.what() << endl << flush;
        exit(1);
    }

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");
    Shader mvpshader("indexed-vertex", "indexed-fragment");
    //mvpshader.use();

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View = 
        glm::lookAt(
            glm::vec3( 0.0, 2.0, 2.0 ),  // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection = 
        glm::perspective(
            glm::radians(66.0f),  // FOV 45.0 degrees
            float(1366.0/768.0),  // aspect ratio
            0.001f,               // near clipping plane
            100.0f);              // far clipping plane

    auto mvp = Projection * View * Model;
    auto mvp_index = mvpshader.registerUniform("MVP", mvp);

    auto start_time = glfwGetTime();

    while(
            !glfwWindowShouldClose(window) && 
            glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 newModel = 
            glm::rotate(
                glm::mat4(1.0f), 
                (float)(glfwGetTime() - start_time), 
                glm::vec3(0.0f, 1.0f, 0.0f));

        glBindVertexArray(VAO_ID);

        //auto new_mvp = Projection * View * newModel;
        mvp = Projection * View * newModel;
        //mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));
        mvpshader.use();

        glBindBuffer(GL_ARRAY_BUFFER, VERTEX_DATA_ID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VERTEX_OFFSET_ID);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

