#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 aNormal;
out vec3 fragPos;
void main()
{
    gl_Position = projection * view * model *  vec4(aPos, 1.0f);
    fragPos = vec3(model * vec4(aPos, 1.0f));
    aNormal = normal;
}