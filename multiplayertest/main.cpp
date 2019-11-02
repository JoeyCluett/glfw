#include <iostream>
#include <map>
#include <functional>

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

#include "main.h"

using namespace std;

bool wireframe = false;

const int WIDTH         = 800;
const int HEIGHT        = 600;
const int GRIDHEIGHT    = 50;
const int GRIDWIDTH     = 50;
const int TEXTUREHEIGHT = 32;
const int TEXTUREWIDTH  = 32;

struct {

    bool space_pressed = false;

} keyboard_events;

int main(int argc, char* argv[]) {

    cout << "Creating window...\n" << flush;
    auto window = GLFWINITWINDOW(WIDTH, HEIGHT, "Hello World", {3, 3}, 4, true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    cout << "DONE\n" << flush;
    
    FloatCam camera({ 2.0, 2.0, 2.0 }, 6.0, WIDTH, HEIGHT, 0.07, window);
    camera.setBounds({ -1.0f, -1.0f, -1.0f }, { GRIDWIDTH, 20.0f, GRIDHEIGHT });

    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(win, true);

            else if(key == GLFW_KEY_Q && action == GLFW_PRESS)
                wireframe = (wireframe ? false : true);

            else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
                keyboard_events.space_pressed = true;

        }
    );

    glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    struct {

        // models with static data
        ModelInfo mine;
        ModelInfo tank;
        ModelInfo tower;
        ModelInfo selector;

        // models with data that is generated at run time
        ModelInfo grid;
        ModelInfo reticle;
        ModelInfo ground_pos;
        ModelInfo ground_uv;
        ModelInfo borderwall;

    } models;

    SimpleModelParser::setFileLocation("../assets/models/");

    try {
    
        SimpleModelParser::loadModelList({
            { "mine.txt",        "toplevel.mine",     &models.mine  },
            { "tank.txt",        "toplevel.tank",     &models.tank  },
            { "tower.txt",       "toplevel.tower",    &models.tower },
            { "selectorbox.txt", "toplevel.selector", &models.selector }
        });

        // create and load the grid array data
        {

            auto grid_vertices = ::generate_grid(GRIDWIDTH, GRIDHEIGHT);

            for(int i = 0; i < (int)grid_vertices.size(); i += 3) {
                grid_vertices[i+0] -= 0.5f;
                grid_vertices[i+2] -= 0.5f;
            }

            models.grid = SimpleModelParser::loadForeignModelIntoRuntime(grid_vertices);
        }

    }
    catch(runtime_error& up) {
        glfwDestroyWindow(window);
        glfwTerminate();        
        cout << up.what() << endl << flush;
        exit(1);
    }

    struct {

        Texture* regular_ground;

    } textures;

    // generate and load proper texture data into OpenGL
    {
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
        };

        for(int i = 0; i < TEXTUREHEIGHT*TEXTUREWIDTH; i++) {
            int ind = (rand() % 13) * 3;
            texture_data.insert(texture_data.end(), { browns[ind+0], browns[ind+1], browns[ind+2] });
        }

        //Texture textureobj(texture_data, TEXTUREHEIGHT, TEXTUREWIDTH, GL_TEXTURE0+0, GL_REPEAT, GL_LINEAR);
        textures.regular_ground = 
            new Texture(
                texture_data, 
                TEXTUREHEIGHT, 
                TEXTUREWIDTH, 
                GL_TEXTURE+0, 
                GL_REPEAT, 
                GL_LINEAR);
    }

    // triangles that texture will be on top of
    {
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

        models.ground_pos = SimpleModelParser::loadForeignModelIntoRuntime(texture_triangles);
    }

    // uv coordinates that describe where the texture will be applied to the triangles
    {
        vector<GLfloat> texture_coords = {
            0.0f,      0.0f,
            0.0f,      GRIDHEIGHT,
            GRIDWIDTH, 0.0f, //1.0f, 0.0f,

            GRIDWIDTH, 0.0f,
            GRIDWIDTH, GRIDHEIGHT,
            0.0f,      GRIDHEIGHT
        };

        models.ground_uv = SimpleModelParser::loadForeignModelIntoRuntime(texture_coords);
    }

    // reticle that is displayed onscreen
    {
        vector<GLfloat> reticle_data = {
            -1.0, 0.0f, -1.0f,
            1.0, 0.0f, -1.0f,
            0.0f, 1.0f, -1.0f,
            0.0f, -1.0f, -1.0f
        };

        for(auto& f : reticle_data)
            f /= 15.0f;

        models.reticle = SimpleModelParser::loadForeignModelIntoRuntime(reticle_data);
    }

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
    Shader::setVertexShaderDirectory("../assets/shaders/vertex/");
    Shader::setFragmentShaderDirectory("../assets/shaders/fragment/");

    Shader mvpshader( "instance" );
    Shader texshader( "landtexture" );
    Shader gridshader( "grid" );

    // create the shader programs used here
    Shader::setVertexShaderDirectory("../assets/shaders/compat/");
    Shader::setFragmentShaderDirectory("../assets/shaders/compat/");

    Shader blackshader( "blackinstancevertex", "blackinstancefragment");
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
    glm::vec3 dummy_color;

    auto mvp_index      = mvpshader.registerUniform(   "MVP",   mvp);
    auto instance_index = mvpshader.registerUniform(   "instance_tf", instance_tf);
    auto color_index    = mvpshader.registerUniform(   "input_color", dummy_color);

    auto grid_mvp_index = gridshader.registerUniform(  "MVP",  mvp);
    
    auto mvp_tex        = texshader.registerUniform(   "MVP",   mvp);
    
    auto blackshadermvp = blackshader.registerUniform( "MVP", mvp);
    auto blackshaderins = blackshader.registerUniform( "instance_tf", instance_tf);

    // entity type IDs
    const int MINE_FRIENDLY   = 0;
    const int TRUCK_FRIENDLY  = 1;
    const int CANNON_FRIENDLY = 2;
    const int MINE_ENEMY      = 3;
    const int TRUCK_ENEMY     = 4;
    const int CANNON_ENEMY    = 5;

    struct {
    private:
        // track the location of every entity on the map
        map<pair<int, int>, const int> board_map;

        bool placeUnit(int x, int y, const int entity_id) {
            auto iter = this->board_map.find({ x, y });
            if(iter == this->board_map.end()) {
                this->board_map.insert({{x, y}, entity_id});
                return true;
            }
            else {
                return false;
            }
        }

    public:

        // every model needs a transform and a color
        struct ModelDescription {
            glm::mat4 tf;
            glm::vec3 c;
        };

        vector<ModelDescription> instance_info_mine;
        vector<ModelDescription> instance_info_truck;
        vector<ModelDescription> instance_info_cannon;

        GLuint instance_size_id_mine;
        GLuint instance_size_id_truck;
        GLuint instance_size_id_cannon;

        // place pieces on the board
        void init(void) {

            int k = 0;

            for(const int i : { 
                    MINE_FRIENDLY,   MINE_ENEMY, 
                    TRUCK_FRIENDLY,  TRUCK_ENEMY, 
                    CANNON_FRIENDLY, CANNON_ENEMY }) {

                for(int j = 0; j < 10;) {
                    //int x = rand() % GRIDWIDTH;
                    //int y = rand() % GRIDHEIGHT;

                    //if(this->placeUnit(x, y, i))
                    //    j++;

                    this->placeUnit(j, k, i);
                    j++;
                }
                k++;

            }
        }
        
        // friendly unit tfs
        struct {
            vector<glm::mat4> mine;
            vector<glm::mat4> truck;
            vector<glm::mat4> cannon;
        } friendly;

        // enemy unit tfs
        struct {
            vector<glm::mat4> mine;
            vector<glm::mat4> truck;
            vector<glm::mat4> cannon;
        } enemy;

        // clear out all of the previous transforms
        void reset(void) {

            //this->instance_info_truck.clear();

            // shortcut way of 'clearing' the vectors holding the calculated transforms
            // avoids calling any destructors
            for(auto* vptr : { 
                    &friendly.mine, &friendly.truck, &friendly.cannon, 
                    &enemy.mine,    &enemy.truck,    &enemy.cannon }) {

                auto optr = reinterpret_cast<glm::mat4**>(vptr);
                optr[1] = optr[0];
            }

        }

        void recalculate_model_transforms(void) {
            this->reset();

            /*
            for(auto p : this->board_map) {
                switch(p.second) {
                    case TRUCK_FRIENDLY:

                        break;
                    case TRUCK_ENEMY:
                        break;
                    default:
                        break;
                }
            }
            */

            for(auto p : this->board_map) {
                switch(p.second) {
                    case MINE_FRIENDLY:
                        this->friendly.mine.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second }));
                        break;
                    case TRUCK_FRIENDLY:
                        this->friendly.truck.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second }));
                        break;
                    case CANNON_FRIENDLY:
                        this->friendly.cannon.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second }));
                        break;
                    case MINE_ENEMY:
                        this->enemy.mine.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second })
                                * glm::rotate((float)M_PI, glm::vec3{0.0f, 1.0f, 0.0f}));
                        break;
                    case TRUCK_ENEMY:
                        this->enemy.truck.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second })
                                * glm::rotate((float)M_PI, glm::vec3{0.0f, 1.0f, 0.0f}));
                        break;
                    case CANNON_ENEMY:
                        this->enemy.cannon.push_back(
                            glm::translate(
                                glm::vec3{ (float)p.first.first, 0.0f, (float)p.first.second })
                                * glm::rotate((float)M_PI, glm::vec3{0.0f, 1.0f, 0.0f}));
                        break;
                    default:
                        throw runtime_error("model_locs.recalculate_model_transforms");
                }
            }
        }

    } model_locs;

    model_locs.init();

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

        if(keyboard_events.space_pressed) {
            camera.setPosition({ pu.x, camera.getPosition().y, pu.z });
            keyboard_events.space_pressed = false;
        }

        if(pu.x < 0 || pu.x >= GRIDWIDTH || pu.z < 0 || pu.z >= GRIDHEIGHT) {
            pu = { 10, -3, 10 };
        }

        model_locs.recalculate_model_transforms();

        glm::vec3 friendly_color( 0.0f, 0.0f, 1.0f);
        glm::vec3 enemy_color(    0.0f, 1.0f, 0.0f);

        // display all of the models on the field
        {
            
            mvpshader.use();
            mvpshader.updateUniformData(mvp_index, reinterpret_cast<void*>(&new_mvp[0][0]));

            glBindBuffer(GL_ARRAY_BUFFER, models.mine.buffer_id);
            mvpshader.updateUniformData(color_index, (void*)&friendly_color);
            for(auto& tf : model_locs.friendly.mine) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.mine.vertices);
            }
            mvpshader.updateUniformData(color_index, (void*)&enemy_color);
            for(auto& tf : model_locs.enemy.mine) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.mine.vertices);
            }

            glBindBuffer(GL_ARRAY_BUFFER, models.tank.buffer_id);
            mvpshader.updateUniformData(color_index, (void*)&friendly_color);
            for(auto& tf : model_locs.friendly.truck) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.tank.vertices);
            }
            mvpshader.updateUniformData(color_index, (void*)&enemy_color);
            for(auto& tf : model_locs.enemy.truck) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.tank.vertices);
            }

            glBindBuffer(GL_ARRAY_BUFFER, models.tower.buffer_id);
            mvpshader.updateUniformData(color_index, (void*)&friendly_color);
            for(auto& tf : model_locs.friendly.cannon) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.tower.vertices);
            }
            mvpshader.updateUniformData(color_index, (void*)&enemy_color);
            for(auto& tf : model_locs.enemy.cannon) {
                mvpshader.use();
                mvpshader.updateUniformData(instance_index, (void*)glm::value_ptr(tf));
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, models.tower.vertices);
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

            glBindBuffer(GL_ARRAY_BUFFER, models.grid.buffer_id);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*)0
            );

            glDrawArrays(GL_LINES, 0, models.grid.vertices);

        }

        // display the texture below the grid (texture.glsl)
        {
            texshader.updateUniformData(mvp_tex, reinterpret_cast<void*>(&new_mvp[0][0]));
            texshader.use();

            textures.regular_ground->use(texshader, "texture_sampler");

            // prepare vertex position data
            glEnableVertexAttribArray(0); // 1st attribute buffer : vertices
            glBindBuffer(GL_ARRAY_BUFFER, models.ground_pos.buffer_id);
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
            glBindBuffer(GL_ARRAY_BUFFER, models.ground_uv.buffer_id);
            glVertexAttribPointer(
                1,        // attribute 1
                2,        // size (2 floats / uv coordinate pair) 
                GL_FLOAT, // type
                GL_FALSE, // normalized
                0,        // stride
                (void*)0  // array buffer offset
            );

            glDrawArrays(GL_TRIANGLES, 0, models.ground_pos.vertices); // 6 vertices, 2 triangle
        }

        // display the aiming reticle in the middle of the screen (notf.glsl)
        {

            notfshader.use();
            glBindBuffer(GL_ARRAY_BUFFER, models.reticle.buffer_id);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*)0
            );

            glDrawArrays(GL_LINES, 0, models.reticle.vertices);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        iter_time = current_time;

    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

