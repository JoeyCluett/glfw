#pragma once

/*

    a Mesh object encapsulates a Vertex Array Object responsible for

*/

#include <iostream>
#include <string>
#include <map>

#include "./Shader.h"
#include "./SimpleModel.h"
#include "./Texture.h"
#include "./IndexedModel.h"

typedef int MeshType_t;
const MeshType_t MESHTYPE_STATIC_SIMPLE           = 0;
const MeshType_t MESHTYPE_INSTANCED_STATIC_SIMPLE = 1;

class Mesh {
private:

    MeshType_t meshtype;
    GLuint vao_id;

    union {
        struct {
                GLuint program_id;
                GLuint 
        } static_simple;
    };

    // simplest possible type of mesh, it is defined in place
    void init_static_simple(std::map<std::string, std::string>& m);
    void init_instanced_static_simple(std::map<std::string, std::string>& m);

public:

    // constructor to read in all relevant data in one call. i use a map of 
    // strings because constructing this object shouldnt be part of the main game loop
    Mesh(const MeshType_t meshtype, std::map<std::string, std::string> m);

    void setUniform_Mat4f(const char* name, glm::mat4* val);
    void setUniform_Vec3f(const char* name, glm::vec3* val);

};

Mesh::Mesh(const MeshType_t meshtype, std::map<std::string, std::string> m) {
    switch(meshtype) {
        case MESHTYPE_STATIC_SIMPLE:
            this->init_static_simple(m);
            break;
        case MESHTYPE_INSTANCED_STATIC_SIMPLE:
            this->init_instanced_static_simple(m);
            break;
        default:
            throw std::runtime_error("Mesh : unknown MeshType_t");
    }

    this->meshtype = meshtype;
}

void Mesh::init_static_simple(std::map<std::string, std::string>& m) {

}

void Mesh::init_instanced_static_simple(std::map<std::string, std::string>& m) {

}

void Mesh::setUniform_Mat4f(const char* name, glm::mat4* val) {

}

void Mesh::setUniform_Vec3f(const char* name, glm::vec3* val) {

}
