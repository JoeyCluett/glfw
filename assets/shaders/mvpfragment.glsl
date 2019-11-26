#version 330 core

out vec3 color;
in vec3 fragmentColor;

void main() {
    // rgb triplet
    //color = vec3(1.0, 0.0, 0.0);
    
    color = fragmentColor;

}
