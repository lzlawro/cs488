#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in vec3 position;
// in vec3 normal;

out VsOutFsIn {
    vec2 texCoord;
} vs_out;

uniform mat4 ModelView;
uniform mat4 Perspective;

void main() {
    // vs_out.position_ES = position;

    vec2 t00 = vec2(0.0, 0.0); // bottom left
    vec2 t01 = vec2(0.0, 1.0); // top left
    vec2 t10 = vec2(1.0, 0.0); // bottom right
    vec2 t11 = vec2(1.0, 1.0); // top right

    if (position.x > 0.0 && position.z > 0.0) {
        vs_out.texCoord = t00;
    } else if (position.x > 0.0 && position.z < 0.0) {
        vs_out.texCoord = t01;
    } else if (position.x < 0.0 && position.z > 0.0) {
        vs_out.texCoord = t10;
    } else {
        vs_out.texCoord = t11;
    }

    gl_Position = Perspective * ModelView * vec4(position, 1.0);
}