#version 330

in vec3 position;

in vec3 ins_color;     // Instance color
in mat4 ins_transform; // instance transform

out vec3 fs_color;     // Output to fragment shader

void main() {
	fs_color = ins_color;

	gl_Position = ins_transform * vec4(position, 1.0);
}
