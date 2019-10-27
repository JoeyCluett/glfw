#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

flat out vec3 fragmentColor;
out vec3 view_position;

uniform mat4 VP; // view - projection matrix
uniform mat4 M;  // model

void main() {
    vec3 light_position;
    light_position.xyz = vec3( 0.0, 0.0, 100000.0 );

    vec3 light_color;
    light_color.xyz = vec3(1.0, 1.0, 1.0);

    view_position.xyz = vec3( 0.0, 2.5, 4.0 );

    vec3  ambient_light_color;
    ambient_light_color.xyz = vec3( 1.0, 1.0, 1.0 );
    vec3 ambient = 0.6 * ambient_light_color;

    vec4 norm_tf = M * vec4(vertexNormal, 0.0);
    vec3 norm = normalize(norm_tf.xyz); // normals need to be transformed with the model
    vec3 light_dir = normalize(light_position - vertexPosition);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = 0.4 * diff * light_color;



    vec3 result = (ambient + diffuse) * vertexColor;

    gl_Position = VP * M * vec4(vertexPosition, 1.0);    
    fragmentColor = result;
}
