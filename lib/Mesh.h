#pragma once

/*

    Mesh class encapsulates model data and shaders
    various flags allow the user to specify where 
    the files for various pieces of information can be found

    shaders are expected to follow a particular naming convention
    - vertex shaders are to be named as <name>.vertex.glsl
    - fragment shaders are to be named as <name>.fragment.glsl
    failure to follow these conventions will result in thrown exceptions

*/

#include <iostream>
#include <string>

constexpr struct {
    
    const int STATIC = 0;

} MESHTYPE;

constexpr struct {

    const int TXT        = 0;
    const int STL_BINARY = 1;
    const int STL_ASCII  = 2;

} MESHMODELTYPE;

class Mesh {
private:

public:

    Mesh(
        const int mesh_type,
        const std::string& modelfile, 
        const int model_type, 
        const std::string& shadername);

};

Mesh::Mesh(const std::string& modelfile, const int model_type, const std::string& shadername) {

}



