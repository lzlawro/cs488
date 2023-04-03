#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in VsOutFsIn {
    vec2 texCoord;
} fs_in;

out vec4 fragColour;

uniform sampler2D tiles;

void main() {

    fragColour = texture2D(tiles, fs_in.texCoord);

}