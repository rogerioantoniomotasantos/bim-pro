/**
 * Custom WebGL Shaders for BIM Viewer
 * Optimized for large-scale geometry rendering
 */

// ============================================
// BASIC VERTEX SHADER
// ============================================
const BASIC_VERTEX_SHADER = `#version 300 es
precision highp float;

// Attributes
in vec3 a_position;
in vec3 a_normal;
in vec4 a_color;

// Uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat3 u_normalMatrix;

// Outputs
out vec3 v_normal;
out vec4 v_color;
out vec3 v_position;

void main() {
    vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
    v_position = worldPosition.xyz;
    
    v_normal = normalize(u_normalMatrix * a_normal);
    v_color = a_color;
    
    gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;
}
`;

// ============================================
// BASIC FRAGMENT SHADER
// ============================================
const BASIC_FRAGMENT_SHADER = `#version 300 es
precision highp float;

// Inputs
in vec3 v_normal;
in vec4 v_color;
in vec3 v_position;

// Uniforms
uniform vec3 u_cameraPosition;
uniform vec3 u_lightDirection;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

// Output
out vec4 fragColor;

void main() {
    // Normalize normal
    vec3 normal = normalize(v_normal);
    
    // Lighting direction (fixed directional light)
    vec3 lightDir = normalize(u_lightDirection);
    
    // Ambient
    float ambient = u_ambient;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    float diffuse = diff * u_diffuse;
    
    // Specular
    vec3 viewDir = normalize(u_cameraPosition - v_position);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), u_shininess);
    float specular = spec * u_specular;
    
    // Combine
    float lighting = ambient + diffuse + specular;
    
    // Apply color
    vec3 color = v_color.rgb * lighting;
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    fragColor = vec4(color, v_color.a);
}
`;

// ============================================
// INSTANCED VERTEX SHADER
// For rendering repeated elements (doors, windows, etc.)
// ============================================
const INSTANCED_VERTEX_SHADER = `#version 300 es
precision highp float;

// Attributes
in vec3 a_position;
in vec3 a_normal;

// Instance attributes
in mat4 a_instanceMatrix;
in vec4 a_instanceColor;

// Uniforms
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

// Outputs
out vec3 v_normal;
out vec4 v_color;
out vec3 v_position;

void main() {
    vec4 worldPosition = a_instanceMatrix * vec4(a_position, 1.0);
    v_position = worldPosition.xyz;
    
    // Transform normal
    mat3 normalMatrix = mat3(a_instanceMatrix);
    v_normal = normalize(normalMatrix * a_normal);
    
    v_color = a_instanceColor;
    
    gl_Position = u_projectionMatrix * u_viewMatrix * worldPosition;
}
`;

// ============================================
// INSTANCED FRAGMENT SHADER
// ============================================
const INSTANCED_FRAGMENT_SHADER = BASIC_FRAGMENT_SHADER;

// ============================================
// GPU FRUSTUM CULLING SHADER
// ============================================
const FRUSTUM_CULLING_COMPUTE = `#version 300 es
precision highp float;

// Frustum planes (6 planes)
uniform vec4 u_frustumPlanes[6];

// Bounding box
in vec3 a_bboxMin;
in vec3 a_bboxMax;

// Output visibility
out float v_visible;

bool testAABBPlane(vec3 bmin, vec3 bmax, vec4 plane) {
    vec3 pVertex;
    
    // P-vertex test
    pVertex.x = (plane.x > 0.0) ? bmax.x : bmin.x;
    pVertex.y = (plane.y > 0.0) ? bmax.y : bmin.y;
    pVertex.z = (plane.z > 0.0) ? bmax.z : bmin.z;
    
    return (dot(vec4(pVertex, 1.0), plane) >= 0.0);
}

void main() {
    // Test against all 6 frustum planes
    bool visible = true;
    
    for (int i = 0; i < 6; i++) {
        if (!testAABBPlane(a_bboxMin, a_bboxMax, u_frustumPlanes[i])) {
            visible = false;
            break;
        }
    }
    
    v_visible = visible ? 1.0 : 0.0;
}
`;

// ============================================
// EDGE DETECTION SHADER (For wireframe mode)
// ============================================
const EDGE_VERTEX_SHADER = `#version 300 es
precision highp float;

in vec3 a_position;
in vec3 a_barycentric;

uniform mat4 u_mvpMatrix;

out vec3 v_barycentric;

void main() {
    v_barycentric = a_barycentric;
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
}
`;

const EDGE_FRAGMENT_SHADER = `#version 300 es
precision highp float;

in vec3 v_barycentric;

uniform vec4 u_lineColor;
uniform float u_lineWidth;

out vec4 fragColor;

float edgeFactor() {
    vec3 d = fwidth(v_barycentric);
    vec3 a3 = smoothstep(vec3(0.0), d * u_lineWidth, v_barycentric);
    return min(min(a3.x, a3.y), a3.z);
}

void main() {
    float edge = 1.0 - edgeFactor();
    
    if (edge < 0.1) discard;
    
    fragColor = vec4(u_lineColor.rgb, edge);
}
`;

// Export shaders
window.BASIC_VERTEX_SHADER = BASIC_VERTEX_SHADER;
window.BASIC_FRAGMENT_SHADER = BASIC_FRAGMENT_SHADER;
window.INSTANCED_VERTEX_SHADER = INSTANCED_VERTEX_SHADER;
window.INSTANCED_FRAGMENT_SHADER = INSTANCED_FRAGMENT_SHADER;
window.FRUSTUM_CULLING_COMPUTE = FRUSTUM_CULLING_COMPUTE;
window.EDGE_VERTEX_SHADER = EDGE_VERTEX_SHADER;
window.EDGE_FRAGMENT_SHADER = EDGE_FRAGMENT_SHADER;
