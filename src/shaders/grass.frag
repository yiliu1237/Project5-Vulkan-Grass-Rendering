#version 450
#extension GL_ARB_separate_shader_objects : enable

// ─────────────────────────────────────────────
// Uniforms
// ─────────────────────────────────────────────
layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 u_ViewMatrix;
    mat4 u_ProjMatrix;
};

// ─────────────────────────────────────────────
// Inputs from Tessellation Evaluation Shader
// ─────────────────────────────────────────────
layout(location = 0) in float fs_HeightParam;  // Height along the blade [0 at root → 1 at tip]
layout(location = 1) in vec3 fs_Normal;        // World-space surface normal
layout(location = 2) flat in int fs_BladeType;

// ─────────────────────────────────────────────
// Output: Final fragment color
// ─────────────────────────────────────────────
layout(location = 0) out vec4 out_Color;

void main() {
    // ─────────────────────────────────────────
    // Color interpolation from base to tip
    // ─────────────────────────────────────────
    vec3 baseColor = vec3(110.0, 180.0, 110.0) / 255.0; // soft green
    vec3 tipColor  = vec3(95.0, 160.0, 95.0) / 255.0;   // slightly darker tip


     // Choose colors based on blade type
    if (fs_BladeType == 0) {
        baseColor = vec3(110.0, 180.0, 110.0) / 255.0; // soft green
        tipColor  = vec3(95.0, 160.0, 95.0) / 255.0;
    }
    else if (fs_BladeType == 1) {
        baseColor = vec3(66.0, 104.0, 40.0) / 255.0; // reddish grass
        tipColor  = vec3(142.0, 69.0, 187.0) / 255.0;
    }
    else if (fs_BladeType == 2) {
        baseColor  = vec3(180.0, 180.0, 110.0) / 255.0; // dried yellowish grass
        tipColor = vec3(243.0, 200.0, 130.0) / 255.0;
    }


    vec3 albedo = mix(baseColor, tipColor, fs_HeightParam);

    // ─────────────────────────────────────────
    // Lighting: Simple ambient + directional
    // ─────────────────────────────────────────
    vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.0)); // Top-down light
    float ambientTerm = 1.0;

    float diffuseTerm = max(dot(normalize(fs_Normal), lightDirection), 0.0);

    vec3 finalColor = albedo * (ambientTerm + diffuseTerm);

    out_Color = vec4(finalColor, 1.0);
}
