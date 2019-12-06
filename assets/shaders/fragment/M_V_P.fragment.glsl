#version 330 core

/*
    matching fragment shader for M_V_P.vertex
*/

out vec3 color;
in vec3 fragmentColor;

void main() {

    color = fragmentColor;

}
