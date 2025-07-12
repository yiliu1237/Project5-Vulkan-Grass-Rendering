#version 450
#extension GL_ARB_separate_shader_objects : enable

// ─────────────────────────────────────────────
// Uniform buffer for model transformation
// ─────────────────────────────────────────────
layout(set = 1, binding = 0) uniform ModelBuffer {
    mat4 u_ModelMatrix; // Transforms from object to world space
};

// ─────────────────────────────────────────────
// Vertex Attributes: positions of triangle vertices
// ─────────────────────────────────────────────
layout(location = 0) in vec4 a_Pos0;  // Vertex position 0 (includes metadata in .w)
layout(location = 1) in vec4 a_Pos1;  // Vertex position 1
layout(location = 2) in vec4 a_Pos2;  // Vertex position 2
layout(location = 3) in vec4 a_Up;        // up
layout(location = 4) in int a_BladeType; 

// ─────────────────────────────────────────────
// Vertex Outputs to the next stage
// ─────────────────────────────────────────────
layout(location = 0) out vec4 v_WorldPos0;
layout(location = 1) out vec4 v_WorldPos1;
layout(location = 2) out vec4 v_WorldPos2;
layout(location = 3) flat out int v_BladeType;


// Required built-in output
out gl_PerVertex {
    vec4 gl_Position;
};

// ─────────────────────────────────────────────
// Function to transform vertex position to world space
// while preserving metadata in the .w component
// ─────────────────────────────────────────────
vec4 TransformToWorldSpace(mat4 modelMatrix, vec4 localPosition) {
    vec4 worldPosition = modelMatrix * vec4(localPosition.xyz, 1.0);
    worldPosition.w = localPosition.w; 
    return worldPosition;
}

// ─────────────────────────────────────────────
// Main vertex shader logic
// ─────────────────────────────────────────────
void main() {
    // Convert local space triangle vertices to world space
    v_WorldPos0 = TransformToWorldSpace(u_ModelMatrix, a_Pos0);
    v_WorldPos1 = TransformToWorldSpace(u_ModelMatrix, a_Pos1);
    v_WorldPos2 = TransformToWorldSpace(u_ModelMatrix, a_Pos2);

    // Only v_WorldPos0 is used to write gl_Position.
    // This is just to satisfy Vulkan validation requirements—
    // the tessellation stage will use the world positions directly.
    gl_Position = v_WorldPos0;

    v_BladeType = a_BladeType;

}
