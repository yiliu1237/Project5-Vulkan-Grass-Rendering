#version 450
#extension GL_ARB_separate_shader_objects : enable

// ─────────────────────────────────────────────
// Camera Uniforms (View and Projection Matrices)
// ─────────────────────────────────────────────
layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 u_ViewMatrix;
    mat4 u_ProjMatrix;
};

// ─────────────────────────────────────────────
// Model Transform Uniforms
// - model: object-to-world matrix
// - objectTrans: xyz = offset, w = scale
// ─────────────────────────────────────────────
layout(set = 1, binding = 0) uniform ModelBuffer {
    mat4 u_ModelMatrix;
    vec4 u_ObjectTransform; // .xyz = offset, .w = scale
};

// ─────────────────────────────────────────────
// Vertex Attributes
// ─────────────────────────────────────────────
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_BladeType; 

// ─────────────────────────────────────────────
// Outputs to Fragment Shader / TCS
// ─────────────────────────────────────────────
layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 3) flat out int v_BladeType;


// Required built-in
out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec3 worldPosition = a_Position;

    // ─────────────────────────────────────────
    // Conditional transformation for special case
    // If UV == (0.7, 0.7), apply scale and translation
    // ─────────────────────────────────────────
    if (abs(a_TexCoord.x - 0.7) < 0.001 && abs(a_TexCoord.y - 0.7) < 0.001) {
        worldPosition = worldPosition * u_ObjectTransform.w + u_ObjectTransform.xyz;
    }

    // ─────────────────────────────────────────
    // Final MVP transformation to clip space
    // ─────────────────────────────────────────
    gl_Position = u_ProjMatrix * u_ViewMatrix * u_ModelMatrix * vec4(worldPosition, 1.0);

    // Pass through color, texcoord, and blade type
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_BladeType = 1;
}
