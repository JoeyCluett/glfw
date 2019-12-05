#include <iostream>

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

using namespace std;

const int WIDTH         = 800;
const int HEIGHT        = 600;
const int GRIDHEIGHT    = 50;
const int GRIDWIDTH     = 50;
const int TEXTUREHEIGHT = 16;
const int TEXTUREWIDTH  = 16;

auto key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

/*
    if(key == GLFW_KEY_UP) {
        if(button_state._accel && action == GLFW_RELEASE)
            button_state._accel = false;
        else if(!button_state._accel && (action == GLFW_PRESS || action == GLFW_REPEAT))
            button_state._accel = true;
    }
    else if(key == GLFW_KEY_DOWN) {
        if(button_state._decel && action == GLFW_RELEASE)
            button_state._decel = false;
        else if(!button_state._decel && (action == GLFW_PRESS || action == GLFW_REPEAT))
            button_state._decel = true;
    }
    else if(key == GLFW_KEY_LEFT) {
        if(button_state._turn_left && action == GLFW_RELEASE)
            button_state._turn_left = false;
        else if(!button_state._turn_left && (action == GLFW_PRESS || action == GLFW_REPEAT))
            button_state._turn_left = true;
    }
    else if(key == GLFW_KEY_RIGHT) {
        if(button_state._turn_right && action == GLFW_RELEASE)
            button_state._turn_right = false;
        else if(!button_state._turn_right && (action == GLFW_PRESS || action == GLFW_REPEAT))
            button_state._turn_right = true;
    }
*/
}

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(800, 600, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    cout << "DONE\n" << flush;

    //button_state._up    = false;
    //button_state._down  = false;
    //button_state._left  = false;
    //button_state._right = false;

    glfwSetKeyCallback(
        window, key_callback);

    glClearColor(0.0f, 0.0f, 0.35f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // i do this the dumb, slow, easy way
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    Texture* ground_tex;
    Shader* ground_tex_shader;
    Shader* box_shader;
    ModelInfo box;
    ModelInfo wheel;
    ModelInfo x_wheel;
    ModelInfo z_wheel;
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
                GL_NEAREST);
                //GL_LINEAR);

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

        // load the shader
        Shader::setVertexShaderDirectory("../assets/shaders/vertex/");
        Shader::setFragmentShaderDirectory("../assets/shaders/fragment/");
        ground_tex_shader = new Shader( "landtexture" );
    }

    // load the box model into the environment
    {
        SimpleModelParser::setFileLocation("../assets/models/");
        SimpleModelParser::loadModelList({
            { "box.txt",   "box.box",        &box },
            { "wheel.txt", "toplevel.wheel", &wheel },
            { "wheel.txt", "toplevel.x_wheel", &x_wheel },
            { "wheel.txt", "toplevel.z_wheel", &z_wheel },
        });

        box_shader = new Shader( "instance" );
    }

    // a camera to look around the field
    FloatCam camera({ 25.0, 2.0, 2.0 }, 6.0, WIDTH, HEIGHT, 0.07, window);
    //camera.setBounds({ -1.0f, -1.0f, -1.0f }, { GRIDWIDTH, 20.0f, GRIDHEIGHT });

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View =
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

    // data to register in the shaders
    auto mvp = Projection * View * Model;
    glm::vec3 dummy_color;
    glm::mat4 instance_tf =
        glm::translate(glm::vec3{ 25.0f, 10.0f, 25.0f });

    auto mvp_tex = ground_tex_shader->registerUniform( "MVP", mvp );

    auto mvp_index      = box_shader->registerUniform( "MVP", mvp);
    auto instance_index = box_shader->registerUniform( "instance_tf", instance_tf);
    auto color_index    = box_shader->registerUniform( "input_color", dummy_color);

    // by this time, all of our assets are loaded and ready to go
    auto iter_time = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto current_time = glfwGetTime();
        auto delta_time = current_time - iter_time;
        camera.update(float(delta_time));
        auto new_mvp = Projection * camera.getTf() * Model;

        // render the ground texture
        {
            ground_tex_shader->updateUniformData(mvp_tex, reinterpret_cast<void*>(&new_mvp[0][0]));
            ground_tex_shader->use();

            ground_tex->use(*ground_tex_shader, "texture_sampler");

            // prepare vertex position data
            glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
            glBindBuffer(GL_ARRAY_BUFFER, ground.buffer_id);
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
            glBindBuffer(GL_ARRAY_BUFFER, ground_uv.buffer_id);
            glVertexAttribPointer(
                1,        // attribute 1
                2,        // size (2 floats / uv coordinate pair)
                GL_FLOAT, // type
                GL_FALSE, // normalized
                0,        // stride
                (void*)0  // array buffer offset
            );

            glDrawArrays(GL_TRIANGLES, 0, ground.vertices); // 6 vertices, 2 triangle
        }

        iter_time = current_time;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
