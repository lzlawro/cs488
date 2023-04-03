#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in vec3 position;

out VsOutFsIn {
    vec3 position_ES;
} vs_out;

uniform mat4 ModelView;
uniform mat4 Perspective;


void main() {
    // position = gl_Vertex.xyz;
    // position.y = ((1.0 - position.y) * (7.0 / 12.0) - 1.0) * poolHeight;
    // gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);

    vs_out.position = position;
    // vs_out.position.y = ((1.0 - position.y) * (7.0 / 12.0) - 1.0) * poolHeight;

    gl_Position = Perspective * ModelView * vec4(position, 1.0);
}