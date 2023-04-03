#version 330

// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------

in VsOutFsIn {
    vec3 position;
} fs_in;

out vec4 fragColour;

void main() {

    // fragColour = vec4(getSphereColor(fs_in.position), 1.0);
    fragColour = vec4(0.0, 0.0, 1.0, 1.0);
    // vec4 info = texture2D(water, position.xz * 0.5 + 0.5);
    // if (position.y < info.r) {
    //     gl_FragColor.rgb *= underwaterColor * 1.2;
    // }

}