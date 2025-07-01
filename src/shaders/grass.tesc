#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(vertices = 1) out;

// ─────────────────────────────────────────────
// Uniforms
// ─────────────────────────────────────────────
layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 u_ViewMatrix;
    mat4 u_ProjMatrix;
};

// ─────────────────────────────────────────────
// Inputs from Vertex Shader
// ─────────────────────────────────────────────
layout(location = 0) in vec4 v_WorldPos0[];  // .w = orientation
layout(location = 1) in vec4 v_WorldPos1[];  // .w = height
layout(location = 2) in vec4 v_WorldPos2[];  // .w = width
layout(location = 3) flat in int v_BladeType[];

// ─────────────────────────────────────────────
// Outputs to Evaluation Shader
// ─────────────────────────────────────────────
layout(location = 0) out vec4 tcs_WorldPos0[];
layout(location = 1) out vec4 tcs_WorldPos1[];
layout(location = 2) out vec4 tcs_WorldPos2[];
layout(location = 3) flat out int tcs_BladeType[];

// ─────────────────────────────────────────────
// Tessellation Configuration
// ─────────────────────────────────────────────
#define BASE_TESS_LEVEL 8.0
#define DYNAMIC_TESS_LEVEL 1

float computeTessellationLevel(float distanceToCamera) {
    float level = BASE_TESS_LEVEL;
    if (distanceToCamera > 10.0) level *= 0.5;
    if (distanceToCamera > 20.0) level *= 0.5;
    return level;
}

void main() {
    // Pass vertex position to built-in output
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Forward blade data
    tcs_WorldPos0[gl_InvocationID] = v_WorldPos0[gl_InvocationID];
    tcs_WorldPos1[gl_InvocationID] = v_WorldPos1[gl_InvocationID];
    tcs_WorldPos2[gl_InvocationID] = v_WorldPos2[gl_InvocationID];
    //tcs_BladeType[gl_InvocationID] = v_BladeType[gl_InvocationID];
    tcs_BladeType[gl_InvocationID] = 1;


    // Compute distance-based LOD
    float tessLevel = BASE_TESS_LEVEL;

#if DYNAMIC_TESS_LEVEL
    vec3 camWorldPos = inverse(u_ViewMatrix)[3].xyz;
    vec3 bladeRoot = v_WorldPos0[gl_InvocationID].xyz;
    float dist = distance(bladeRoot, camWorldPos);
    tessLevel = computeTessellationLevel(dist);
#endif

    // Assign tessellation levels (quads)
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
}
