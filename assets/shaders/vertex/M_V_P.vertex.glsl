#version 330 core

/*
    this shader is used when the user needs to place a specific
    entity whose loca/global tf may change on every frame. in
    this case, its easier to just send that data to the shader
    seperately and let the shader make those calculations
*/

layout(location = 0) in vec3 vertexPosition;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 input_color; // color for this instance
out vec3 fragmentColor;

void main() {

    gl_Position = MVP * instance_tf * vec4(vertexPosition, 1.0);

    // calculate the color of this vertex
    //float mod_result = mod( gl_VertexID, 3 );
    //if(mod_result < 0.1) {                          fragmentColor = vec3( 0.0, 0.0, 0.0 ); }
    //else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = vec3( 0.5, 0.0, 0.0 ); }
    //else {                                          fragmentColor = vec3( 0.25, 0.0, 0.0 ); }

    // shades of input color
    float mod_result = mod( gl_VertexID, 3 );
    if(mod_result < 0.1) {                          fragmentColor = input_color * 0.0; }
    else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = input_color * 0.5; }
    else {                                          fragmentColor = input_color * 1.0; }

}
