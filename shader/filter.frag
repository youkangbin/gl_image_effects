#version 440
#ifdef GL_ES
precision highp float;
#endif

layout(binding = 0) uniform sampler2D u_input;
layout(location = 0) in vec2 v_texCoord;
layout(location = 0) out vec4 fragColor;

uniform float u_strength;  // 滤镜强度 0.0-1.0

void main() {
    vec4 color = texture(u_input, v_texCoord);
    
    // 灰度转换
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec3 grayscale = vec3(gray);
    
    // 混合原图和灰度（根据强度）
    vec3 result = mix(color.rgb, grayscale, u_strength);
    fragColor = vec4(result, color.a);
}
