
const int FILTER_GRAYSCALE = 1;    // 1 << 0
const int FILTER_INVERT    = 2;    // 1 << 1
const int FILTER_BLUR      = 4;    // 1 << 2
const int FILTER_SHARPEN   = 8;    // 1 << 3
const int FILTER_EDGE      = 16;   // 1 << 4
const int FILTER_WARM      = 32;   // 1 << 5
const int FILTER_COOL      = 64;   // 1 << 6
const int FILTER_SEPIA     = 128;  // 1 << 7
const int FILTER_LUT       = 256;  // 1 << 8
const int FILTER_MASK      = 512;  // 1 << 9

uniform sampler2D u_texture;
uniform int       u_filterMode;

in  vec2 v_texCoord;
out vec4 fragColor;

bool hasFilter(int flag) {
    return (u_filterMode & flag) != 0;
}

vec4 sampleOffset(vec2 uv, vec2 offset) {
    return texture(u_texture, uv + offset);
}

void main()
{
    vec2 uv     = v_texCoord;
    vec2 texel  = vec2(1.0) / vec2(textureSize(u_texture, 0));  // 单个像素大小

    // ① 先取原始颜色
    vec4 color = texture(u_texture, uv);

    // ── 模糊（3×3 均值，优先于锐化/边缘，作用于原图）─────────────
    if (hasFilter(FILTER_BLUR)) {
        vec4 blurred = vec4(0.0);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                blurred += sampleOffset(uv, vec2(float(x), float(y)) * texel);
            }
        }
        color = blurred / 9.0;
    }

    // ── 锐化（Unsharp Mask）────────────────────────────────────────
    if (hasFilter(FILTER_SHARPEN)) {
        vec4 center = texture(u_texture, uv);
        vec4 blur4  = (
            sampleOffset(uv, vec2(-1.0,  0.0) * texel) +
            sampleOffset(uv, vec2( 1.0,  0.0) * texel) +
            sampleOffset(uv, vec2( 0.0, -1.0) * texel) +
            sampleOffset(uv, vec2( 0.0,  1.0) * texel)
        ) / 4.0;
        color = clamp(center * 2.0 - blur4, 0.0, 1.0);
    }

    // ── 边缘检测（Sobel）──────────────────────────────────────────
    if (hasFilter(FILTER_EDGE)) {
        float gx =
            -1.0 * sampleOffset(uv, vec2(-1.0,  1.0) * texel).r +
             1.0 * sampleOffset(uv, vec2( 1.0,  1.0) * texel).r +
            -2.0 * sampleOffset(uv, vec2(-1.0,  0.0) * texel).r +
             2.0 * sampleOffset(uv, vec2( 1.0,  0.0) * texel).r +
            -1.0 * sampleOffset(uv, vec2(-1.0, -1.0) * texel).r +
             1.0 * sampleOffset(uv, vec2( 1.0, -1.0) * texel).r;

        float gy =
             1.0 * sampleOffset(uv, vec2(-1.0,  1.0) * texel).r +
             2.0 * sampleOffset(uv, vec2( 0.0,  1.0) * texel).r +
             1.0 * sampleOffset(uv, vec2( 1.0,  1.0) * texel).r +
            -1.0 * sampleOffset(uv, vec2(-1.0, -1.0) * texel).r +
            -2.0 * sampleOffset(uv, vec2( 0.0, -1.0) * texel).r +
            -1.0 * sampleOffset(uv, vec2( 1.0, -1.0) * texel).r;

        float edge = clamp(sqrt(gx * gx + gy * gy), 0.0, 1.0);
        color = vec4(vec3(edge), 1.0);
    }

    // ── 灰度 ──────────────────────────────────────────────────────
    if (hasFilter(FILTER_GRAYSCALE)) {
        // 人眼亮度加权
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color = vec4(vec3(gray), color.a);
    }

    // ── 反色 ──────────────────────────────────────────────────────
    if (hasFilter(FILTER_INVERT)) {
        color = vec4(1.0 - color.rgb, color.a);
    }

    // ── 暖色调（增强 R，削弱 B）────────────────────────────────────
    if (hasFilter(FILTER_WARM)) {
        color.r = clamp(color.r * 1.2,  0.0, 1.0);
        color.g = clamp(color.g * 1.05, 0.0, 1.0);
        color.b = clamp(color.b * 0.8,  0.0, 1.0);
    }

    // ── 冷色调（增强 B，削弱 R）────────────────────────────────────
    if (hasFilter(FILTER_COOL)) {
        color.r = clamp(color.r * 0.8,  0.0, 1.0);
        color.g = clamp(color.g * 1.05, 0.0, 1.0);
        color.b = clamp(color.b * 1.2,  0.0, 1.0);
    }

    // ── 复古（Sepia）──────────────────────────────────────────────
    if (hasFilter(FILTER_SEPIA)) {
        float r = dot(color.rgb, vec3(0.393, 0.769, 0.189));
        float g = dot(color.rgb, vec3(0.349, 0.686, 0.168));
        float b = dot(color.rgb, vec3(0.272, 0.534, 0.131));
        color = vec4(clamp(vec3(r, g, b), 0.0, 1.0), color.a);
    }

    fragColor = color;
}
