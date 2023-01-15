#version 330

in vec3 fs_color;

out vec4 fragColor;

void main() {
	fragColor = vec4(fs_color, 1.0);
}
