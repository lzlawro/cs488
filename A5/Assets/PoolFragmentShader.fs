#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in VsOutFsIn {
    vec2 texCoord;
} fs_in;

out vec4 fragColour;

uniform sampler2D tiles;

void main() {

    // fragColour = vec4(getSphereColor(fs_in.position), 1.0);
    // fragColour = vec4(0.0, 0.0, 0.3, 1.0);
    fragColour = texture2D(tiles, fs_in.texCoord);
    // vec4 info = texture2D(water, position.xz * 0.5 + 0.5);
    // if (position.y < info.r) {
    //     gl_FragColor.rgb *= underwaterColor * 1.2;
    // }
}