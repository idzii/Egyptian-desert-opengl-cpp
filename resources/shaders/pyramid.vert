#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCords;
layout (location = 2) in vec3 normal;

out vec3 aNormal;
out vec3 fragPos;
out vec2 texCords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    aNormal = transpose(inverse(mat3(model))) * normal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    texCords = aTexCords;
    fragPos = vec3(model * vec4(aPos, 1.0));
}