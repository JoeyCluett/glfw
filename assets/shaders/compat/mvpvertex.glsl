#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;
uniform mat4 MVP; // model - view - projection matrix

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1.0);    

    // calculate the color of this vertex
    float mod_result = mod( gl_VertexID, 3 );
    if(mod_result < 0.1) {                          fragmentColor = vec3( 0.0, 0.0, 0.0 ); }
    else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = vec3( 0.5, 0.0, 0.0 ); }
    else {                                          fragmentColor = vec3( 1.0, 0.0, 0.0 ); }

    //fragmentColor = vertexColor;
}
