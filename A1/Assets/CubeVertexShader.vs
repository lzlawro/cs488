#version 330

// uniform mat4 P;
// uniform mat4 V;
// uniform mat4 M;
// in vec3 position;

// void main() {
// 	gl_Position = P * V * M * vec4(position, 1.0);
// }

layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexColors;

out vec3 v_vertexColors;

void main()
{
    v_vertexColors = vertexColors;

    gl_Position = vec4(position.x, position.y, position.z, 1.0f);
}
