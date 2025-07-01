#version 450
#extension GL_ARB_separate_shader_objects : enable

// ─────────────────────────────────────────────
// Sampler for the bound 2D texture
// ─────────────────────────────────────────────
layout(set = 1, binding = 1) uniform sampler2D u_TexSampler;

// ─────────────────────────────────────────────
// Inputs from the vertex shader
// ─────────────────────────────────────────────
layout(location = 0) in vec3 v_Color;       // Unused in current shader
layout(location = 1) in vec2 v_TexCoord;    // Texture UV coordinates

// ─────────────────────────────────────────────
// Output: Final fragment color
// ─────────────────────────────────────────────
layout(location = 0) out vec4 out_Color;

void main() {
    // Sample texture color using UV
    out_Color = texture(u_TexSampler, v_TexCoord);
}
