#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP;         // model - view - projection matrix
uniform mat4 instance_tf; // tf for this specific instance
uniform vec3 mesh_color;  // supply a solid color for this entire mesh

out vec3 fragment_color;

void main() {

    gl_Position = MVP * instance_tf * vec4(vertexPosition, 1.0);    
    
    // color is supplied by user
    fragment_color = mesh_color;

}

