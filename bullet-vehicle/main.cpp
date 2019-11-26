#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bullet/btBulletDynamicsCommon.h>

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
const int TEXTUREHEIGHT = 8;
const int TEXTUREWIDTH  = 8;

// place to store collision entity information
btAlignedObjectArray<btCollisionShape*> collision_shapes;
vector<btRigidBody*> rigid_body_list; // this holds info we want every frame
vector<glm::mat4> transform_list;

void evaluateTransforms(void) {
    transform_list.clear();
    transform_list.resize(rigid_body_list.size());

    size_t sz = rigid_body_list.size();
    for(unsigned int i = 0; i < sz; i++) {
        btTransform tf;
        rigid_body_list[i]->getMotionState()->getWorldTransform(tf);
        tf.getOpenGLMatrix(glm::value_ptr(transform_list[i]));
    }

    //transform_list.push_back(glm::translate(glm::vec3{ 10.0f, -4.0f, 10.0f }));

}

void addCube(btDiscreteDynamicsWorld* dynamics_world, glm::vec3 pos, float _mass = 4.0) {
    btCollisionShape* box_shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
    collision_shapes.push_back(box_shape);

    btTransform box_tf;
    box_tf.setIdentity();
    box_tf.setOrigin(btVector3(pos.x, pos.y, pos.z));

    btScalar m(_mass);
    btVector3 local_inertia(0.0, 0.0, 0.0);
    box_shape->calculateLocalInertia(m, local_inertia);
    btDefaultMotionState* motion_state = new btDefaultMotionState(box_tf);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, box_shape, local_inertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setFriction(0.9);
    dynamics_world->addRigidBody(body);
    rigid_body_list.push_back(body);
}

struct {
    bool _up;
    bool _down;
    bool _left;
    bool _right;
} button_state;

auto key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    //else if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
    //    button_state._up = (action == GLFW_PRESS) ? true : (action == GLFW_RELEASE);

    //else if((key == GLFW_KEY_UP || key == GLFW_KEY_W) && action == GLFW_PRESS)
    //    button_state._up = true;
    //else if((key == GLFW_KEY_UP || key == GLFW_KEY_W) && action == GLFW_RELEASE)
    //        button_state._up = false;

}

void evaluate_keyboard_input(GLFWwindow* win) {

    if(glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    button_state._up =
            glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS;

    button_state._down =
            glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS;

    button_state._left =
            glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS;

    button_state._right =
            glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS;

}

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(800, 600, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
    cout << "DONE\n" << flush;
    cout << "Hello ReactPhysics3D\n";

	btDefaultCollisionConfiguration* collisionConfiguration =
        new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher =
        new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache =
        new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver =
        new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld =
        new btDiscreteDynamicsWorld(
            dispatcher,
            overlappingPairCache,
            solver,
            collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -10, 0));

    // add the ground as a static collision entity
    {
        // provide half-extents
        btCollisionShape* ground_shape = new btBoxShape(btVector3(25.0, 25.0, 25.0));
        collision_shapes.push_back(ground_shape);

        // specify where the ground is in world coordinates
        btTransform ground_tf;
        ground_tf.setIdentity();
        ground_tf.setOrigin(btVector3( 25.0, -25.0, 25.0 ));

        btScalar  m(0.0);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, ground_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(1.0);

        dynamicsWorld->addRigidBody(body);
    }

    for(int i = 0; i < 40; i++) {

        float y = 1.1*i + 0.5;

        //addCube(dynamicsWorld, {25.0, 0.5, 25.0}, 80.0);

        addCube(dynamicsWorld, {25.0, y, 10.0 + (0.1*i)}, 0.1);
        //addCube(dynamicsWorld, {23.0, y, 10.0 + (0.1*i)}, 80.0);
        //addCube(dynamicsWorld, {21.0, y, 10.0 + (0.1*i)}, 80.0);
        addCube(dynamicsWorld, {19.0, y, 10.0 + (0.1*i)}, 0.1);

    }

    button_state._up    = false;
    button_state._down  = false;
    button_state._left  = false;
    button_state._right = false;

    //glfwSetKeyCallback(
    //    window, key_callback);

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
            { "box.txt", "box.box", &box }
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
    double accumulated_time = 0.0;
    while(!glfwWindowShouldClose(window) /*&& glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS*/) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto current_time = glfwGetTime();
        auto delta_time = current_time - iter_time;
        camera.update(
            float(delta_time),
            button_state._up, button_state._down,
            button_state._left, button_state._right);
        //camera.update(float(delta_time));
        auto new_mvp = Projection * camera.getTf() * Model;
        accumulated_time += delta_time;

        // perform the physics simulation
        {
            const double timestep = 1.0/120.0;
            while(accumulated_time >= timestep) {
                dynamicsWorld->stepSimulation(timestep);
                accumulated_time -= timestep;
            }
        }

        // render the blocks in the scene
        {
            glm::vec3 color = { 1.0f, 0.0f, 0.0f };
            evaluateTransforms();

            box_shader->use();
            box_shader->updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));

            glBindBuffer(GL_ARRAY_BUFFER, box.buffer_id);
            box_shader->updateUniformData(color_index, (void*)&color);

            for(auto& m : transform_list) {
                box_shader->use();
                box_shader->updateUniformData(instance_index, (void*)glm::value_ptr(m));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, box.vertices);
            }
        }

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
        evaluate_keyboard_input(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
