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
vector<glm::mat4> cube_transform_list;
vector<glm::mat4> wheel_transform_list;
vector<glm::mat4> x_wheel_transform_list;
vector<glm::mat4> z_wheel_transform_list;

void evaluateTransforms(void) {
    cube_transform_list.clear();
    wheel_transform_list.clear();
    x_wheel_transform_list.clear();
    z_wheel_transform_list.clear();

    size_t sz = rigid_body_list.size();
    for(unsigned int i = 0; i < sz; i++) {

        btRigidBody* rb = rigid_body_list[i];
        vector<glm::mat4>* ptr = (vector<glm::mat4>*)rb->getUserPointer();
        ptr->push_back(glm::mat4());

        btTransform tf;
        rb->getMotionState()->getWorldTransform(tf);
        tf.getOpenGLMatrix(glm::value_ptr( ptr->back() ));

    }

    cube_transform_list.push_back(glm::translate(glm::vec3{ 10.0f, -4.0f, 10.0f }));
    wheel_transform_list.push_back(glm::translate(glm::vec3{ 10.0f, -4.0f, 10.0f }));
    x_wheel_transform_list.push_back(glm::translate(glm::vec3{ 10.0f, -4.0f, 10.0f }));
    z_wheel_transform_list.push_back(glm::translate(glm::vec3{ 10.0f, -4.0f, 10.0f }));
}

btRigidBody* addCube(btDiscreteDynamicsWorld* dynamics_world, glm::vec3 pos, float _mass = 4.0) {
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
    body->setUserPointer((void*)&cube_transform_list);
    body->setFriction(0.9);
    dynamics_world->addRigidBody(body);
    rigid_body_list.push_back(body);
    return body;
}


btRigidBody* addCylinder(btDiscreteDynamicsWorld* dynamics_world, glm::vec3 pos, char axis = 'y', float _mass = 4.0) {
    btCollisionShape* cyl_shape = NULL;


    if(axis == 'z') {
        // rotate about x axis
        cyl_shape = new btCylinderShapeZ(btVector3( 0.5, 0.5, 0.1 ));
    }
    else if(axis == 'x') {
        // rotate about z axis
        cyl_shape = new btCylinderShapeX(btVector3( 0.1, 0.5, 0.5 ));
    }
    else if(axis == 'y') {

        cyl_shape = new btCylinderShape(btVector3( 0.5, 0.1, 0.5 ));
    }

    collision_shapes.push_back(cyl_shape);

    btTransform wheel_tf;
    wheel_tf.setIdentity();
    wheel_tf.setOrigin(btVector3( 0.0, 0.0, 0.0 ));
    wheel_tf.setOrigin(btVector3( pos.x, pos.y, pos.z ));

    btScalar m(_mass);
    btVector3 local_inertia(0.0, 0.0, 0.0);
    cyl_shape->calculateLocalInertia(m, local_inertia);
    btDefaultMotionState* motion_state = new btDefaultMotionState(wheel_tf);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, cyl_shape, local_inertia);

    btRigidBody* body = new btRigidBody(rbInfo);
    if(axis == 'x')
        body->setUserPointer((void*)&x_wheel_transform_list);
    else if(axis == 'y')
        body->setUserPointer((void*)&wheel_transform_list);
    else if(axis == 'z')
        body->setUserPointer((void*)&z_wheel_transform_list);

    body->setFriction(1.0);
    body->setRollingFriction(0.1);
    dynamics_world->addRigidBody(body);
    rigid_body_list.push_back(body);
    return body;
}


struct {
    bool _up;
    bool _down;
    bool _left;
    bool _right;

    bool _accel;
    bool _decel;
    bool _turn_left;
    bool _turn_right;

} button_state;

auto key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    button_state._up    = key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT);
    button_state._down  = key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT);
    button_state._left  = key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT);
    button_state._right = key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT);

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

}

void evaluate_keyboard_input(GLFWwindow* win) {

    if(glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    auto eval_bool_key = [&win](bool& b, int key) -> void {
        int st = glfwGetKey(win, key);

        if(!b && st == GLFW_PRESS) {
            b = true;
        }
        else if(b && st == GLFW_RELEASE) {
            b = false;
        }
    };

    eval_bool_key(button_state._accel, GLFW_KEY_UP);
    eval_bool_key(button_state._decel, GLFW_KEY_DOWN);
    eval_bool_key(button_state._turn_left, GLFW_KEY_LEFT);
    eval_bool_key(button_state._turn_right, GLFW_KEY_RIGHT);

    button_state._up    = glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS;
    button_state._down  = glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS;
    button_state._left  = glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS;
    button_state._right = glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS;

    //button_state._accel = glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS;
    //button_state._decel = glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS;
    //button_state._turn_left = glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS;
    //button_state._turn_right = glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS;
}

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(800, 600, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
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
        ground_tf.setOrigin(btVector3( 25.0, -25.0, 24.5 ));

        btScalar  m(0.0);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, ground_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(1.0);
        body->setRollingFriction(0.9);

        dynamicsWorld->addRigidBody(body);
    }

    btRigidBody* rbA = addCube(dynamicsWorld, { 25.0, 10.0, 25.0 });
    btRigidBody* rbB = addCube(dynamicsWorld, { 25.0, 12.0, 25.0 });
    btRigidBody* rbC = addCube(dynamicsWorld, { 25.0, 14.0, 25.0 });
    btRigidBody* rbD = addCube(dynamicsWorld, { 25.0, 16.0, 25.0 });
    auto* p2p_0 = new btPoint2PointConstraint(*rbA, *rbB, btVector3(0.0, 1.0, 0.0), btVector3(0.0, -1.0, 0.0));
    auto* p2p_1 = new btPoint2PointConstraint(*rbB, *rbC, btVector3(0.0, 1.0, 0.0), btVector3(0.0, -1.0, 0.0));
    auto* p2p_2 = new btPoint2PointConstraint(*rbC, *rbD, btVector3(0.0, 1.0, 0.0), btVector3(0.0, -1.0, 0.0));
    dynamicsWorld->addConstraint(p2p_0);
    dynamicsWorld->addConstraint(p2p_1);
    dynamicsWorld->addConstraint(p2p_2);

    addCylinder(dynamicsWorld, {25.0, 25.0, 25.0}, 'x');
    addCylinder(dynamicsWorld, {25.0, 27.0, 25.0}, 'z');
    addCylinder(dynamicsWorld, {25.0, 29.0, 25.0});


    // add all the entities needed for a small vehicle
    btRigidBody* rbChassis        = addCube(dynamicsWorld,     { 10.0, 23.0, 10.0 }, 2.0);
    btRigidBody* rbLeftWheel      = addCylinder(dynamicsWorld, { 11.0, 22.0, 12.0 }, 'x', 1.0);
    btRigidBody* rbRightWheel     = addCylinder(dynamicsWorld, { 9.0,  22.0, 12.0 }, 'x', 1.0);
    btRigidBody* rbRearLeftWheel  = addCylinder(dynamicsWorld, { 11.0, 22.0, 8.0 },  'x', 1.0);
    btRigidBody* rbRearRightWheel = addCylinder(dynamicsWorld, { 9.0,  22.0, 8.0 },  'x', 1.0);
    rbLeftWheel->setActivationState(DISABLE_DEACTIVATION);
    auto* hinge_0 = new btHingeConstraint(
            *rbChassis, *rbLeftWheel,
            btVector3(0.0, -1.0, 2.0), btVector3(-1.0, 0.0, 0.0),
            btVector3(1.0, 0.0, 0.0),  btVector3(1.0, 0.0, 0.0));
    auto* hinge_1 = new btHingeConstraint(
            *rbChassis, *rbRightWheel,
            btVector3(0.0, -1.0, 2.0), btVector3(1.0, 0.0, 0.0),
            btVector3(1.0, 0.0, 0.0),  btVector3(1.0, 0.0, 0.0));
    auto* hinge_2 = new btHingeConstraint(
            *rbChassis, *rbRearLeftWheel,
            btVector3(0.0, -1.0, -2.0), btVector3(-1.0, 0.0, 0.0),
            btVector3(1.0, 0.0, 0.0),   btVector3(1.0, 0.0, 0.0));
    auto* hinge_3 = new btHingeConstraint(
            *rbChassis, *rbRearRightWheel,
            btVector3(0.0, -1.0, -2.0), btVector3(1.0, 0.0, 0.0),
            btVector3(1.0, 0.0, 0.0),   btVector3(1.0, 0.0, 0.0));

    hinge_0->enableMotor(true);
    hinge_1->enableMotor(true);
    hinge_2->enableMotor(true);
    hinge_3->enableMotor(true);

    dynamicsWorld->addConstraint(hinge_0);
    dynamicsWorld->addConstraint(hinge_1);
    dynamicsWorld->addConstraint(hinge_2);
    dynamicsWorld->addConstraint(hinge_3);

    //addCube(dynamicsWorld, { 11.0, 0.6, 12.6 });

    button_state._up    = false;
    button_state._down  = false;
    button_state._left  = false;
    button_state._right = false;

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

        float FORWARD = -12.0;

        hinge_0->enableMotor(true);
        hinge_1->enableMotor(true);
        hinge_2->enableMotor(true);
        hinge_3->enableMotor(true);

        if(button_state._accel) {
            hinge_0->enableAngularMotor(true, FORWARD, 6.0); // front left
            hinge_1->enableAngularMotor(true, FORWARD, 6.0); // front right
            hinge_2->enableAngularMotor(true, FORWARD, 6.0); // rear left
            hinge_3->enableAngularMotor(true, FORWARD, 6.0); // rear right

            if(button_state._turn_left) {
                hinge_0->enableAngularMotor(true, 0, 6.0);
                hinge_2->enableAngularMotor(true, 0, 6.0);
            }
            if(button_state._turn_right) {
                hinge_1->enableAngularMotor(true, 0, 6.0);
                hinge_3->enableAngularMotor(true, 0, 6.0);
            }

        }
        else if(button_state._decel) {
            hinge_0->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_1->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_2->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_3->enableAngularMotor(true, -FORWARD, 6.0);
        }
        else if(button_state._turn_left) {
            hinge_0->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_1->enableAngularMotor(true, FORWARD, 6.0);
            hinge_2->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_3->enableAngularMotor(true, FORWARD, 6.0);
        }
        else if(button_state._turn_right) {
            hinge_0->enableAngularMotor(true, FORWARD, 6.0);
            hinge_1->enableAngularMotor(true, -FORWARD, 6.0);
            hinge_2->enableAngularMotor(true, FORWARD, 6.0);
            hinge_3->enableAngularMotor(true, -FORWARD, 6.0);
        }
        else {
            hinge_0->enableAngularMotor(true, 0.0, 6.0);
            hinge_1->enableAngularMotor(true, 0.0, 6.0);
            hinge_2->enableAngularMotor(true, 0.0, 6.0);
            hinge_3->enableAngularMotor(true, 0.0, 6.0);
        }

        // perform the physics simulation
        {

            const double timestep = 1.0/120.0;
            while(accumulated_time >= timestep) {
                dynamicsWorld->stepSimulation(timestep/1.0);
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
            for(auto& m : cube_transform_list) {
                box_shader->use();
                box_shader->updateUniformData(instance_index, (void*)glm::value_ptr(m));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, box.vertices);
            }

            glBindBuffer(GL_ARRAY_BUFFER, wheel.buffer_id);
            box_shader->updateUniformData(color_index, (void*)&color);
            for(auto& tf : wheel_transform_list) {
                box_shader->use();
                box_shader->updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, wheel.vertices);
            }

            glBindBuffer(GL_ARRAY_BUFFER, x_wheel.buffer_id);
            box_shader->updateUniformData(color_index, (void*)&color);
            for(auto& tf : x_wheel_transform_list) {
                box_shader->use();
                box_shader->updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, x_wheel.vertices);
            }

            glBindBuffer(GL_ARRAY_BUFFER, z_wheel.buffer_id);
            box_shader->updateUniformData(color_index, (void*)&color);
            for(auto& tf : z_wheel_transform_list) {
                box_shader->use();
                box_shader->updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, z_wheel.vertices);
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
        //evaluate_keyboard_input(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
