#pragma once

#include <vector>
#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::vector<GLfloat> generate_grid(int width, int height, GLfloat cell_width = 1.0f, GLfloat cell_height = 1.0f) {

    std::vector<GLfloat> vertices;

    for(int i = 0; i <= width; i++) {
        vertices.insert(
            vertices.end(),
            { 
                cell_width*i, 0.05f, 0.0f,    
                cell_width*i, 0.05f, cell_height*height 
            }
        );
    }

    for(int j = 0; j <= height; j++) {
        vertices.insert(
            vertices.end(),
            {  
                0.0f,             0.05f, cell_height*j,
                cell_width*width, 0.05f, cell_height*j
            }
        );
    }


/*
    for(int i = 0; i < 40; i++) {

        float cx = float(i) / 4;
        vertices.insert(
            vertices.end(),
            { cx, 0.0f, 0.0f, cx, 0.0f, 10.0f });

    }
*/
    return vertices;
}

/*
    Entity map stores x/y transform information with model vertex IDs
*/

class EntityMap {
private:

    std::map<GLuint, glm::vec3> m;

public:

    EntityMap(std::vector<GLuint> v) {
        for(auto i : v) {
            this->m[i] = {0.0f, 0.0f, 0.0f};
        }
    }



};

