#pragma once
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "Texture.h"

#include <vector>

static std::string read_shader_code(std::string& filename);
static GLuint create_vertex_shader(std::string& filename);
static GLuint create_fragment_shader(std::string& filename);
static GLuint CREATE_SHADER(GLuint shader_type, std::string filename);
static GLuint LINK_SHADERS(GLuint vertex_shader, GLuint fragment_shader, GLint delete_shaders);

static const int UNIFORM_MAT4FV = 0;
static const int UNIFORM_VEC3F  = 1;

class Shader {
private:
    GLuint programid;

    struct uniform {
        GLint id;
        int    type;
        void*  data;
    };

    std::vector<uniform> uniform_list;

    static std::string vertex_shader_dir;
    static std::string fragment_shader_dir;

public:

    Shader(std::string vertexshader, std::string fragmentshader) {
        this->programid =
            LINK_SHADERS(
                CREATE_SHADER(
                    GL_VERTEX_SHADER,
                    Shader::vertex_shader_dir + vertexshader + ".glsl"),

                CREATE_SHADER(
                    GL_FRAGMENT_SHADER,
                    Shader::fragment_shader_dir + fragmentshader + ".glsl"),

                GL_TRUE
            );
    }

    Shader(std::string shadername) {
        this->programid =
            LINK_SHADERS(
                CREATE_SHADER(
                    GL_VERTEX_SHADER,
                    Shader::vertex_shader_dir + shadername + ".vertex.glsl"),

                CREATE_SHADER(
                    GL_FRAGMENT_SHADER,
                    Shader::fragment_shader_dir + shadername + ".fragment.glsl"),

                GL_TRUE
            );
    }

    static void setVertexShaderDirectory(std::string shader_loc) {
        Shader::vertex_shader_dir = shader_loc;
    }

    static void setFragmentShaderDirectory(std::string shader_loc) {
        Shader::fragment_shader_dir = shader_loc;
    }

    // overloaded method will allow sending multiple different types of uniform
    int registerUniform(const char* uniformname, glm::mat4& m4) {

        glUseProgram(this->programid);
        auto matrixid = glGetUniformLocation(this->programid, uniformname);

        this->uniform_list.push_back({
            matrixid,
            UNIFORM_MAT4FV,
            reinterpret_cast<void*>(&m4[0][0])
        });

        return this->uniform_list.size() - 1;
    }

    int registerUniform(const char* uniformname, glm::vec3& v3) {

        glUseProgram(this->programid);
        auto vecid = glGetUniformLocation(this->programid, uniformname);

        this->uniform_list.push_back({
            vecid,
            UNIFORM_VEC3F,
            reinterpret_cast<void*>(&v3)
        });

        return this->uniform_list.size() - 1;
    }

    GLuint getShaderId(void) {
        return this->programid;
    }

    void updateUniformData(int index, void* data) {
        this->uniform_list[index].data = data;
    }

    void use(void) {
        glUseProgram(this->programid);

        for(auto& u : this->uniform_list) {
            switch(u.type) {
                case UNIFORM_MAT4FV:
                    glUniformMatrix4fv(
                            u.id,
                            1,
                            GL_FALSE,
                            reinterpret_cast<float*>(u.data));
                    break;
                case UNIFORM_VEC3F:
                    glUniform3fv(
                            u.id,
                            1,
                            reinterpret_cast<float*>(u.data));
                    break;
                default:
                    throw std::runtime_error("Unknown uniform type encountered in Shader.use()");
            }
        }

    }

};

std::string Shader::vertex_shader_dir   = "./";
std::string Shader::fragment_shader_dir = "./";

static std::string read_shader_code(std::string& filename) {
    std::string shader_code;

    std::ifstream is(filename);
    std::vector<char> buf((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    buf.push_back('\0');

    shader_code = buf.data();
    return shader_code;

}

static GLuint create_vertex_shader(std::string& filename) {
    auto shaderid   = glCreateShader(GL_VERTEX_SHADER);
    auto shadercode = read_shader_code(filename);
    auto cptr = shadercode.c_str();

    //std::cout << "vertex shader:\n" << shadercode << std::endl << std::flush;

    // compile shader source
    glShaderSource(shaderid, 1, &cptr, NULL);
    glCompileShader(shaderid);

    // check compiled shader
    GLint result = GL_FALSE;
    int log_length;
    glGetShaderiv(shaderid, GL_COMPILE_STATUS,  &result);
    glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &log_length);

    if(log_length > 0) {
        std::vector<char> v(log_length + 1);
        glGetShaderInfoLog(shaderid, log_length, NULL, v.data());
        std::cout << v.data() << std::endl;

        throw std::runtime_error(
            "\nCREATE_VERTEX_SHADER (" + filename + "): " + std::string(v.data())
        );
    }

    return shaderid;

}

static GLuint create_fragment_shader(std::string& filename) {
    auto shaderid   = glCreateShader(GL_FRAGMENT_SHADER);
    auto shadercode = read_shader_code(filename);
    auto cptr = shadercode.c_str();

    //std::cout << "fragment shader:\n" << shadercode << std::endl << std::flush;

    // compile shader source
    glShaderSource(shaderid, 1, &cptr, NULL);
    glCompileShader(shaderid);

    // check compiled shader
    GLint result = GL_FALSE;
    int log_length;
    glGetShaderiv(shaderid, GL_COMPILE_STATUS,  &result);
    glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &log_length);

    if(log_length > 0) {
        std::vector<char> v(log_length + 1);
        glGetShaderInfoLog(shaderid, log_length, NULL, v.data());
        std::cout << v.data() << std::endl;

        throw std::runtime_error(
            "\nCREATE_FRAGMENT_SHADER (" + filename + "): " + std::string(v.data())
        );
    }

    return shaderid;
}

static GLuint CREATE_SHADER(GLuint shader_type, std::string filename) {
    if(shader_type == GL_VERTEX_SHADER)
        return create_vertex_shader(filename);

    else if(shader_type == GL_FRAGMENT_SHADER)
        return create_fragment_shader(filename);

    else
        throw std::runtime_error("CREATESHADER : unknown shader type");
}

static GLuint LINK_SHADERS(GLuint vertex_shader, GLuint fragment_shader, GLint delete_shaders) {

    // link final program
    auto programid = glCreateProgram();
    glAttachShader(programid, vertex_shader);
    glAttachShader(programid, fragment_shader);
    glLinkProgram(programid);

    // check final program
    GLint result = GL_FALSE;
    int log_length;
    glGetProgramiv(programid, GL_LINK_STATUS,     &result);
    glGetProgramiv(programid, GL_INFO_LOG_LENGTH, &log_length);

    if(log_length > 0) {
        std::vector<char> v(log_length + 1);
        glGetProgramInfoLog(programid, log_length, NULL, v.data());
        std::cout << v.data() << std::endl;

        throw std::runtime_error("\nLINK_SHADERS : " + std::string(v.data()));
    }

    glDetachShader(programid, vertex_shader);
    glDetachShader(programid, fragment_shader);

    if(delete_shaders == GL_TRUE) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    return programid;
}
