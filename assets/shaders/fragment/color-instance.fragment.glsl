#version 330 core

out vec3 color;
in vec3 fragment_color;

void main() {
    // rgb triplet
    color = fragment_color;
}