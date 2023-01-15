// Vertex Shader
#version 330

in vec3 position;
in vec3 color;

out vec3 vsColor;

void main() {
    gl_Position = vec4(position, 1.0);
    
    vsColor = color;
}
