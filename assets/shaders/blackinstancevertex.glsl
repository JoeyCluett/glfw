#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP; // model - view - projection matrix
uniform mat4 instance_tf; // tf for this specific instance

void main() {

    gl_Position = MVP * instance_tf * vec4(vertexPosition, 1.0);    

}