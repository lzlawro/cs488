#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in vec3 position;
in vec3 normal;

out VsOutFsIn {
    vec3 position;
} vs_out;

uniform mat4 ModelView;
uniform mat4 Perspective;

void main() {
    vs_out.position = position;
    // vs_out.position_ES = position;
    gl_Position = Perspective * ModelView * vec4(position, 1.0);
}