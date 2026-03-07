#version 440

layout(location = 0) in vec2 vertex;    // 2D顶点坐标
layout(location = 1) in vec2 texCoord;  // 纹理坐标

layout(location = 0) out vec2 v_texCoord;

void main() {
    gl_Position = vec4(vertex, 0.0, 1.0);
    v_texCoord = texCoord;
}
