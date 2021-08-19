#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 aTexCords;

out vec3 aNormal;
out vec2 texCords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;

void main()
{
    aNormal = transpose(inverse(mat3(model))) * normals;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    texCords = aTexCords;
    fragPos = vec3(model * vec4(aPos, 1.0));
}