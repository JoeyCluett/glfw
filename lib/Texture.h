#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int TEXTURE_CUSTOM_TXT = 0;

class Texture {
private:
    GLuint texture_unit; // always GL_TEXTURE0 + some constant
    GLuint texture_id;

public:

    int height;
    int width;

private:

    // use custom txt format for describing image data
    void loadCustomTextureType(
            const std::string& filename, 
            GLuint texture_unit, 
            GLuint texture_wrap, 
            GLuint texture_minmag) {
        
        std::ifstream is(filename);
        
        // remove all of the comments from the input file
        {
            std::ofstream os("/tmp/texture-file.txt");
            std::string str;
            bool comment = false;
            while(is >> str) {

                if(comment) {
                    //std::cout << "// " << str << std::endl << std::flush;
                    if(str == "**>")
                        comment = false;
                }
                else {
                    //std::cout << str << std::endl << std::flush;
                    if(str == "<**") {
                        comment = true;
                    }
                    else {
                        os << str << ' ';
                    }
                }
            }
        }

        is.close();
        is.open("/tmp/texture-file.txt");
        std::vector<uint8_t> image_data;
        
        {
            int rows, columns;

            std::string str = "";

            is >> str;
            if(str != "ROWS")
                throw std::runtime_error("Malformed texture file: " + filename + ", missing ROWS attribute");
            is >> rows;
            is >> str;
            if(str != "COLUMNS")
                throw std::runtime_error("Malformed texture file: " + filename + ", missing COLUMNS attribute");
            is >> columns;

            this->width  = columns;
            this->height = rows;

            int elements = rows * columns;

            for(int i = 0; i < elements; i++) {
                int r, g, b;
                is >> r >> g >> b;
                this->image_data.push_back(r & 0xFF);
                this->image_data.push_back(g & 0xFF);
                this->image_data.push_back(b & 0xFF);
            }
        }
    
        // generate all of the OpenGL stuff we need for this texture

        glGenTextures(1, &this->texture_id);
        glBindTexture(GL_TEXTURE_2D, this->texture_id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_minmag);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_minmag);
        
        // give this data to OpenGL
        glTexImage2D(
            GL_TEXTURE_2D, // texture type
            0,             // base image level - no mipmapping
            GL_RGB,        // internal format of the texture
            this->width,   // width of the image
            this->height,  // height of the image
            0,             // border width, MUST BE ZERO
            GL_RGB,        // format, MUST MATCH INTERNAL FORMAT
            GL_UNSIGNED_BYTE,       // data type
            this->image_data.data() // pointer to image data
        );

        glGenerateMipmap(GL_TEXTURE_2D);

        // now that opengl owns the data, we can remove it from our local storage
        this->image_data.clear();
        this->image_data.shrink_to_fit();
        this->texture_unit_id = texture_unit_id;
    }

public:

    Texture(
            const std::string& filename, 
            const int filetype, 
            GLuint texture_unit_id, 
            GLuint texture_wrap, 
            GLuint texture_minmag) {

        if(filetype == TEXTURE_CUSTOM_TXT) {
            this->loadCustomTextureType(filename, texture_unit_id, texture_wrap, texture_minmag);
        }
        else {
            throw std::runtime_error("Texture : Unsupported filetype");
        }
    }

    Texture(
            std::vector<uint8_t>& texture_data, 
            int height,
            int width,
            GLuint texture_unit, 
            GLuint texture_wrap, 
            GLuint texture_minmag) {

        this->height = height;
        this->width  = width;



    }

    GLuint getTextureUnit(void) {
        return this->texture_unit_id;
    }

    GLuint getTextureId(void) {
        return this->texture_id;
    }

    void use(void) {
        glActiveTexture(this->texture_unit_id);
        glBindTexture(GL_TEXTURE_2D, this->texture_id);
    }

};

