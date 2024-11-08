#pragma once
#include <GLFW/glfw3.h>

// シェーダーのソースコード (頂点シェーダー)
const GLchar* _wtVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// シェーダーのソースコード (フラグメントシェーダー)
const GLchar* _wtFragmentShaderSource = R"(
#version 330 core
uniform vec4 userSetColor;
out vec4 FragColor;

void main() {
    FragColor = userSetColor;
}
)";
