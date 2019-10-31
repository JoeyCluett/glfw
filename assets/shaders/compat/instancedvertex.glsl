#version 330 core

layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentColor;

uniform mat4 MVP; // model - view - projection matrix
uniform mat4 instance_tf; // tf for this specific instance

void main() {

    gl_Position = MVP * instance_tf * vec4(vertexPosition, 1.0);    

    // calculate the color of this vertex
    float mod_result = mod( gl_VertexID, 3 );
    if(mod_result < 0.1) {                          fragmentColor = vec3( 0.0, 0.0, 0.0 ); }
    else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = vec3( 0.0, 0.0, 0.5 ); }
    else {                                          fragmentColor = vec3( 0.0, 0.0, 1.0 ); }

}
