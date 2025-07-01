#version 450
#extension GL_ARB_separate_shader_objects : enable

// ─────────────────────────────────────────────
// Tessellation Evaluation Shader
// - Receives 1 quad patch per grass blade
// - Computes curved blade geometry via Bezier interpolation
// ─────────────────────────────────────────────

layout(quads, equal_spacing, ccw) in;

// ──────────────
// Uniforms
// ──────────────
layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 u_ViewMatrix;
    mat4 u_ProjMatrix;
};

// ──────────────
// Inputs from TCS
// ──────────────
layout(location = 0) in vec4 tcs_WorldPos0[];  // v0: root (with .w = orientation)
layout(location = 1) in vec4 tcs_WorldPos1[];  // v1: mid
layout(location = 2) in vec4 tcs_WorldPos2[];  // v2: tip (with .w = width)
layout(location = 3) in int tes_BladeType[];   // Blade type

// ──────────────
// Outputs to FS
// ──────────────
layout(location = 0) out float fs_HeightParam;
layout(location = 1) out vec3 fs_Normal;
layout(location = 2) flat out int fs_BladeType;  // going to FS

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Extract blade data
    vec3 root = tcs_WorldPos0[0].xyz;
    vec3 mid  = tcs_WorldPos1[0].xyz;
    vec3 tip  = tcs_WorldPos2[0].xyz;

    float orientation = tcs_WorldPos0[0].w;
    float width       = tcs_WorldPos2[0].w;

    int bladeType = tes_BladeType[0];


    // ─────────────────────────────────────
    // Step 1: Modify the mid point based on bladeType
    // ─────────────────────────────────────
    vec3 up = vec3(0.0, 1.0, 0.0); // World up
    vec3 bendAxis = normalize(cross(up, tip - root));
    if (bladeType == 1) {
        mid += 0.1 * sin(v * 3.1415 * 2.0) * bendAxis; // Wavy
    } else if (bladeType == 2) {
        mid += 0.05 * up; // Straighter and taller
    } 

    // bladeType == 0 is default shape (no change)

    // ─────────────────────────────────────
    // Bezier curve point along the center
    // ─────────────────────────────────────
    vec3 lerpA = mix(root, mid, v);
    vec3 lerpB = mix(mid, tip, v);
    vec3 curvePoint = mix(lerpA, lerpB, v);

    // ─────────────────────────────────────
    // Build local frame (tangent + bitangent)
    // ─────────────────────────────────────
    vec3 tangent   = normalize(lerpB - lerpA);
    vec3 bitangent = normalize(vec3(-cos(orientation), 0.0, sin(orientation)));

    // ─────────────────────────────────────
    // Build blade edges and interpolate across width
    // ─────────────────────────────────────
    vec3 leftEdge  = curvePoint - width * bitangent;
    vec3 rightEdge = curvePoint + width * bitangent;

    float edgeInterp = u + 0.5 * v - u * v; // Biased interpolation
    vec3 worldPos = mix(leftEdge, rightEdge, edgeInterp);

    // ─────────────────────────────────────
    // Output
    // ─────────────────────────────────────
    fs_HeightParam = v;
    fs_Normal = normalize(cross(tangent, bitangent));
    gl_Position = u_ProjMatrix * u_ViewMatrix * vec4(worldPos, 1.0);
    fs_BladeType = bladeType;

}
