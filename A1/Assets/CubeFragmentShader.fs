#version 330

// uniform vec3 colour;

// out vec4 fragColor;

// void main() {
// 	fragColor = vec4( colour, 1 );
// }

in vec3 v_vertexColors;

out vec4 color;

void main()
{
    color = vec4(v_vertexColors.r,
                v_vertexColors.g, 
                v_vertexColors.b,
                1.0f);
}
