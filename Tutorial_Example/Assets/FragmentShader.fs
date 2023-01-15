// Fragment Shader
#version 330

in vec3 vsColor;

out vec4 fragColor;

void main() {
    fragColor = vec4(vsColor, 1.0);
}
