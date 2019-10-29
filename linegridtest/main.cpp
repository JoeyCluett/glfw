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

using namespace std;

bool wireframe = false;

const int WIDTH = 800;
const int HEIGHT = 600;

const int GRIDHEIGHT = 20;
const int GRIDWIDTH  = 20;

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

            else if(key == GLFW_KEY_Q && action == GLFW_PRESS)
                wireframe = (wireframe ? false : true);

        }
    );

    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    pair<vector<float>, int> mv; // model data

    try {
        SimpleModelParser::setFileLocation("../assets/models/");
        SimpleModelParser smp("mine.txt");
        mv = smp.getExportedModelData("toplevel.mine");
    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        
        cout << up.what() << endl << flush;

        throw up; // lol
        exit(1);
    }

    GLfloat*        vertex_buffer_data = mv.first.data();

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, mv.second*12, vertex_buffer_data, GL_STATIC_DRAW);

    ModelInfo 
        tank_info, 
        tower_info;

    // load the armored vehicle model
    try {
        
        {
            SimpleModelParser smp("tank.txt");
            tank_info = smp.loadExportedModelIntoRuntime("toplevel.tank");
        }

        {
            SimpleModelParser smp("tower.txt");
            tower_info = smp.loadExportedModelIntoRuntime("toplevel.tower");
        }

    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();
        cout << up.what() << endl;
        exit(1);
    }

    // create and load the grid array data
    vector<GLfloat> grid_vertices;
    for(int i = 0; i <= 20; i++) {
        
        grid_vertices.insert(
            grid_vertices.end(), 
            { -0.5f + 0.0f, 0.05f, -0.5f + (GLfloat)i,   -0.5f + 20.0f, 0.05f, -0.5f + (GLfloat)i  });
    
        grid_vertices.insert(
            grid_vertices.end(),
            { -0.5f + (GLfloat)i, 0.05f, -0.5f + 0.0f,   -0.5f + (GLfloat)i, 0.05f, -0.5f + 20.0f });

    }

    GLuint vertex_grid_buffer;
    glGenBuffers(1, &vertex_grid_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_grid_buffer);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 4, grid_vertices.data(), GL_STATIC_DRAW);

    // generate and load a 20x20 texture
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
    for(int i = 0; i < GRIDHEIGHT*GRIDWIDTH; i++) {
        //texture_data.push_back(rand() % 256); // r
        //texture_data.push_back(rand() % 256); // g
        //texture_data.push_back(rand() % 256); // b
    
        //texture_data.insert(texture_data.end(), { 0, (unsigned char)((rand() & 127)+128), 0 });

        int ind = (rand() % 13) * 3;
        texture_data.insert(texture_data.end(), { browns[ind+0], browns[ind+1], browns[ind+2] });

    }

    // triangles that texture will be on top of
    vector<GLfloat> texture_triangles = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 20.0f,
        20.0f, 0.0f, 0.0f,

        20.0f, 0.0f, 0.0f,
        20.0f, 0.0f, 20.0f,
        0.0f, 0.0f, 20.0f
    };

    for(int i = 0; i < (int)texture_triangles.size(); i += 3) {
        texture_triangles[i]   -= 0.5f;
        texture_triangles[i+2] -= 0.5f;
    }

    vector<GLfloat> texture_coords = {
        0.0f, 0.0f,
        0.0f, GRIDHEIGHT, //1.0f,
        GRIDWIDTH, 0.0f, //1.0f, 0.0f,

        GRIDWIDTH, 0.0f,
        GRIDWIDTH, GRIDHEIGHT,
        0.0f,      GRIDHEIGHT

        //1.0f, 0.0f,
        //1.0f, 1.0f,
        //0.0f, 1.0f     
    };

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GRIDWIDTH, GRIDHEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // load the texture vertex data
    GLuint texture_vertex_id;
    glGenBuffers(1, &texture_vertex_id);
    glBindBuffer(GL_ARRAY_BUFFER, texture_vertex_id);
    glBufferData(GL_ARRAY_BUFFER, texture_triangles.size() * 4, texture_triangles.data(), GL_STATIC_DRAW);

    GLuint uv_buffer_id;
    glGenBuffers(1, &uv_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * 4, texture_coords.data(), GL_STATIC_DRAW);

    vector<GLfloat> reticle_data = {
        -1.0, 0.0f, -1.0f,
        1.0, 0.0f, -1.0f,
        0.0f, 1.0f, -1.0f,
        0.0f, -1.0f, -1.0f
    };

    for(auto& f : reticle_data)
        f /= 15.0f;

    GLuint ret_vertex_id;
    glGenBuffers(1, &ret_vertex_id);
    glBindBuffer(GL_ARRAY_BUFFER, ret_vertex_id);
    glBufferData(GL_ARRAY_BUFFER, reticle_data.size() * 4, reticle_data.data(), GL_STATIC_DRAW);

    GLuint selector_vertex_id;
    glGenBuffers(1, &selector_vertex_id);
    glBindBuffer(GL_ARRAY_BUFFER, selector_vertex_id);
    int selector_vertex_count;

    // load the selector model
    try {

        SimpleModelParser smp("selectorbox.txt");
        auto mv = smp.getExportedModelData("toplevel.selector");
        selector_vertex_count = mv.first.size()/3;
        glBufferData(GL_ARRAY_BUFFER, mv.first.size() * 4, mv.first.data(), GL_STATIC_DRAW);

    } catch(runtime_error& up) {

        glfwDestroyWindow(window);
        glfwTerminate();

        throw up; // lol
        exit(1);
    }

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/");
    Shader::setFragmentShaderDirectory("../assets/shaders/");

    Shader mvpshader(   "instancedvertex",     "instancedfragment");
    Shader blackshader( "blackinstancevertex", "blackinstancefragment");
    Shader gridshader(  "linegridvertex",      "linegridfragment");
    Shader texshader(   "texturevertex",       "texturefragment");
    Shader notfshader(  "notfvertex",          "notffragment");

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

    glm::mat4 instance_tf = 
        glm::translate(glm::vec3{ 0.0f, 0.0f, 0.0f });
    
    auto mvp = Projection * View * Model;

    auto mvp_index      = mvpshader.registerUniform(  "MVP",   mvp);
    auto instance_index = mvpshader.registerUniform(  "instance_tf", instance_tf);
    auto grid_mvp_index = gridshader.registerUniform( "MVP",  mvp);
    auto mvp_tex        = texshader.registerUniform(  "MVP",   mvp);
    auto blackshadermvp = blackshader.registerUniform("MVP", mvp);
    auto blackshaderins = blackshader.registerUniform("instance_tf", instance_tf);

    vector<glm::vec3> tf_vector;
    for(int i = 0; i < 20; i++) {
        for(int j = 0; j < 20; j++) {

            if((j+i) % 2 && j > 4 && j < 7)
                tf_vector.push_back({ i, 0.0f, j });

        }
    }

    vector<glm::vec3> tf_vector_truck;
    for(int i = 0; i < 20; i++) {
        for(int j = 0; j < 20; j++) {

            if((j == 4 || j == 3) && !((i+j+1)%2))
                tf_vector_truck.push_back({ i, 0.0f, j });

        }
    }

    vector<glm::vec3> tf_vector_tower;
    for(int i = 0; i < 20; i++) {
        for(int j = 0; j < 20; j++) {

            if(j == 0 && i%2)
                tf_vector_tower.push_back({ i, 0.0f, j });

        }
    }

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

        // find where in the grid the camera is pointing
        glm::vec3 projected_unit;
        {
            auto pos = camera.getPosition();
            auto dir = camera.getDirection();


            GLfloat t = -pos.y / dir.y;

            projected_unit.x = pos.x + t*dir.x;
            projected_unit.y = 0;
            projected_unit.z = pos.z + t*dir.z;

        }
        auto& pu = projected_unit;
        projected_unit = { round(pu.x), pu.y, round(pu.z) };

        if(pu.x < 0 || pu.x > 19 || pu.z < 0 || pu.z > 19) {
            pu = { 10, -3, 10 };
        }

        // display all of the tank models, all shades of red (instancedvertex.glsl)
        {

            mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));
            
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            for(auto v : tf_vector) {

                instance_tf = glm::translate(v);

                mvpshader.updateUniformData(instance_index, reinterpret_cast<void*>(&instance_tf[0][0]));
                mvpshader.use();

                // prepare vertex position data
                glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
                glVertexAttribPointer(
                    0,          // attribute 0 (must match layout in shader)
                    3,          // size
                    GL_FLOAT,   // type
                    GL_FALSE,   // normalized
                    0,          // stride
                    (void*)0    // array buffer offset
                );

                glDrawArrays(GL_TRIANGLES, 0, mv.second);
            
            }

            glBindBuffer(GL_ARRAY_BUFFER, tank_info.buffer_id);
            for(auto v : tf_vector_truck) {

                instance_tf = glm::translate(v);

                mvpshader.updateUniformData(instance_index, reinterpret_cast<void*>(&instance_tf[0][0]));
                mvpshader.use();

                // prepare vertex position data
                glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
                glVertexAttribPointer(
                    0,          // attribute 0 (must match layout in shader)
                    3,          // size
                    GL_FLOAT,   // type
                    GL_FALSE,   // normalized
                    0,          // stride
                    (void*)0    // array buffer offset
                );

                glDrawArrays(GL_TRIANGLES, 0, tank_info.vertices);
            
            }

            glBindBuffer(GL_ARRAY_BUFFER, tower_info.buffer_id);
            for(auto v : tf_vector_tower) {

                instance_tf = glm::translate(v);

                mvpshader.updateUniformData(instance_index, reinterpret_cast<void*>(&instance_tf[0][0]));
                mvpshader.use();

                // prepare vertex position data
                glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
                glVertexAttribPointer(
                    0,          // attribute 0 (must match layout in shader)
                    3,          // size
                    GL_FLOAT,   // type
                    GL_FALSE,   // normalized
                    0,          // stride
                    (void*)0    // array buffer offset
                );

                glDrawArrays(GL_TRIANGLES, 0, tower_info.vertices);
            
            }


        }

        // display the selector box (blackinstance.glsl)
        {

            glBindBuffer(GL_ARRAY_BUFFER, selector_vertex_id);

            auto s = glm::translate(projected_unit);
            blackshader.updateUniformData(blackshadermvp, reinterpret_cast<void*>(&new_mvp[0][0]));
            blackshader.updateUniformData(blackshaderins, reinterpret_cast<void*>(&s[0][0]));
            blackshader.use();

            glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
            glVertexAttribPointer(
                0,          // attribute 0 (must match layout in shader)
                3,          // size
                GL_FLOAT,   // type
                GL_FALSE,   // normalized
                0,          // stride
                (void*)0    // array buffer offset
            );

            glDrawArrays(GL_TRIANGLES, 0, selector_vertex_count);

        }

        // display the grid lines all in black (linegrid.glsl)
        {

            gridshader.updateUniformData(grid_mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));
            gridshader.use();

            glBindBuffer(GL_ARRAY_BUFFER, vertex_grid_buffer);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*)0
            );

            glDrawArrays(GL_LINES, 0, grid_vertices.size() / 3);

        }

        // display the texture below the grid (texture.glsl)
        {
            texshader.updateUniformData(mvp_tex, reinterpret_cast<void*>(&new_mvp[0][0]));
            texshader.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glUniform1i(glGetUniformLocation(texshader.getShaderId(), "texture_sampler"), 0);

            // prepare vertex position data
            glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
            glBindBuffer(GL_ARRAY_BUFFER, texture_vertex_id);
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

            glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices, 2 triangle
        }

        // display the aiming reticle in the middle of the screen (notf.glsl)
        {

            notfshader.use();
            glBindBuffer(GL_ARRAY_BUFFER, ret_vertex_id);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*)0
            );

            glDrawArrays(GL_LINES, 0, 4);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        iter_time = current_time;

    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

