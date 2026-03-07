// filter.vert
// 注意：不要写 #version，由 C++ 在运行时动态注入
// C++ 注入后效果示例：
//   ES:      #version 320 es\nprecision highp float;\n...
//   Desktop: #version 330 core\n...

in vec2 a_position;
in vec2 a_texCoord;
out vec2 v_texCoord;

void main()
{
    v_texCoord  = a_texCoord;
    gl_Position = vec4(a_position, 0.0, 1.0);
}
