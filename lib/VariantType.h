#pragma once

#include <iostream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Variant_t {
private:

    int flag;

    union {
        unsigned int _u;
        int          _i;
        glm::vec3    _vf3;
        glm::mat4    _mf4;
    };

    std::string s;

    std::string get_flag_string(void);

public:

    static const int FLAG_uint = 0;
    static const int FLAG_int  = 1;
    static const int FLAG_str  = 2;
    static const int FLAG_vf3  = 3;
    static const int FLAG_mf4  = 4;

    Variant_t(unsigned int d);
    Variant_t(int          d);
    Variant_t(std::string  s);
    Variant_t(glm::vec3    v3);
    Variant_t(glm::mat4    m4);

    unsigned int& getUint(void);
    int& getInt(void);
    glm::vec3& getVec3(void);
    glm::mat4& getMat4(void);
    std::string& getString(void);

};

std::string Variant_t::get_flag_string(void) {
    switch(this->flag) {
        case FLAG_uint: return "uint"; break;
        case FLAG_int:  return "int";  break;
        case FLAG_str:  return "str";  break;
        case FLAG_vf3:  return "vf3";  break;
        case FLAG_mf4:  return "mf4";  break;
        default:
            throw std::runtime_error("Variant_t : unknown inner type");        
    }
}

Variant_t::Variant_t(unsigned int d) {
    this->flag = FLAG_uint;
    this->_u = d;
}

Variant_t::Variant_t(int d) {
    this->flag = FLAG_int;
    this->_i = d;
}

Variant_t::Variant_t(std::string s) {
    this->flag = FLAG_str;
    this->s = s;
}

Variant_t::Variant_t(glm::vec3 v3) {
    this->flag = FLAG_vf3;
    this->_vf3 = v3;
}

Variant_t::Variant_t(glm::mat4 m4) {
    this->flag = FLAG_mf4;
    this->_mf4 = m4;
}

unsigned int& Variant_t::getUint(void) {
    if(this->flag != FLAG_uint)
        throw std::runtime_error(
            "Variant_t : requested type 'uint' does "
            "not match actual type '" + this->get_flag_string() + "'");
}

int& Variant_t::getInt(void) {
    if(this->flag != FLAG_uint)
        throw std::runtime_error(
            "Variant_t : requested type 'int' does "
            "not match actual type '" + this->get_flag_string() + "'");
}

glm::vec3& Variant_t::getVec3(void) {
    if(this->flag != FLAG_uint)
        throw std::runtime_error(
            "Variant_t : requested type 'vf3' does "
            "not match actual type '" + this->get_flag_string() + "'");
}

glm::mat4& Variant_t::getMat4(void) {
    if(this->flag != FLAG_uint)
        throw std::runtime_error(
            "Variant_t : requested type 'mf4' does "
            "not match actual type '" + this->get_flag_string() + "'");
}

std::string& Variant_t::getString(void) {
    if(this->flag != FLAG_uint)
        throw std::runtime_error(
            "Variant_t : requested type 'str' does "
            "not match actual type '" + this->get_flag_string() + "'");
}
