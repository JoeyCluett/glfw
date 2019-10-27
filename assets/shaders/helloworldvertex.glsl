#version 330 core

// tells shader where to look for data
layout(location = 0) in vec3 vertexPosition_modelspace;

void main() {

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;

}
