#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <functional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../lib/Initialization.h"
#include "../lib/Shader.h"
#include "../lib/SimpleModel.h"

// define less-than operator for glm::vec3
/*bool operator<(const glm::vec3& lhs, const glm::vec3& rhs) {
    return
        (lhs.x <  rhs.x) ||
        (lhs.x == rhs.x && lhs.y <  rhs.y) ||
        (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
}*/

bool less_than_vec3(const glm::vec3& lhs, const glm::vec3& rhs) {
    return
        (lhs.x <  rhs.x) ||
        (lhs.x == rhs.x && lhs.y <  rhs.y) ||
        (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
}

/*
    this class creates indexed VBO models 
    from the model data taken from SimpleModelParser
*/

class IndexedModel {
private:

    std::vector<GLfloat>      vertexdatabuffer;
    std::vector<unsigned int> vertexdataoffsetbuffer;

public:

    IndexedModel(SimpleModelParser& smp, const std::string& modelnane);

    std::vector<GLfloat>& vertexData(void) {
        return this->vertexdatabuffer;
    }

    std::vector<unsigned int>& vertexOffsets(void) {
        return this->vertexdataoffsetbuffer;
    }

};

IndexedModel::IndexedModel(SimpleModelParser& smp, const std::string& modelname) {

    // retrieve correct model data
    auto md = smp.getExportedModelData(modelname);
    
    std::map<
            glm::vec3, 
            unsigned int, 
            bool(*)(const glm::vec3&, const glm::vec3&)> 
        point_map(less_than_vec3);
    
    unsigned int current_index = 0;

    // iterate through vertex data and create proper 
    // indices for unique points, also create data in 
    /*int sz = md.first.size();
    for(int i = 0; i < sz; i += 3) {
        // just add everything to the buffers
        this->vertexdataoffsetbuffer.push_back(current_index);

        this->vertexdatabuffer.push_back(md.first[i+0]);
        this->vertexdatabuffer.push_back(md.first[i+1]);
        this->vertexdatabuffer.push_back(md.first[i+2]);

        current_index++;
    }*/

    int sz = md.first.size();
    for(int i = 0; i < sz; i += 3) {
        glm::vec3 v(
            md.first[i+0], 
            md.first[i+1], 
            md.first[i+2]
        );

        auto iter = point_map.find(v);
        if(iter == point_map.end()) {
            // point does not exist in map yet. insert it and apply new index
            
            point_map.insert({ v, current_index });
            this->vertexdataoffsetbuffer.push_back(current_index);

            this->vertexdatabuffer.push_back(v.x);
            this->vertexdatabuffer.push_back(v.y);
            this->vertexdatabuffer.push_back(v.z);

            current_index++;
        
        }
        else {
            // point does exist, just grab the data from the 
            // iterator. do not modify current_index
            //std::cout << "Reusing vertex data...\n";
            this->vertexdataoffsetbuffer.push_back(iter->second);

            //this->vertexdatabuffer.push_back(iter->first.x);
            //this->vertexdatabuffer.push_back(iter->first.y);
            //this->vertexdatabuffer.push_back(iter->first.z);

        }
    }

}
