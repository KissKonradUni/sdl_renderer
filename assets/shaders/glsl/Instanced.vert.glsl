#version 460 core

// --- Constants for Offsets ---
const uint POSITION_OFFSET = 0;
const uint NORMAL_OFFSET   = 3;
const uint TANGENT_OFFSET  = 6;
const uint UV_OFFSET       = 9;

// --- SSBO Bindings ---
layout(std430, binding = 0) buffer VertexBuffer {
    float vertexData[];  // Stores positions, normals, tangents, UVs in sequence
};

layout(std430, binding = 1) buffer IndexBuffer {
    uint indices[];
};

struct MeshRecord {
    uint vertexOffset;
    uint indexOffset;
    uint vertexCount;
    uint indexCount;
};
layout(std430, binding = 2) buffer RecordBuffer {
    MeshRecord records[];
};

struct Instance {
    mat4 modelMatrix;
    uint recordIndex;
};
layout(std430, binding = 3) buffer InstanceBuffer {
    Instance instances[];
};

// --- Per-Vertex Outputs ---
out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec3 fragmentTangent;
out vec2 fragmentUV;
out mat3 tangentSpaceMatrix;

out vec3 cameraPosition;
out vec3 cameraDirection;

// --- Camera UBO ---
layout(std140, binding = 4) uniform Camera {
    mat4 camView;
    mat4 camProjection;
    vec3 camPosition;
    vec3 camDirection;
};

// --- Helper Functions ---
vec3 fetchVec3(uint baseIndex, uint offset) {
    uint index = baseIndex + offset;
    return vec3(vertexData[index], vertexData[index + 1], vertexData[index + 2]);
}

vec2 fetchVec2(uint baseIndex, uint offset) {
    uint index = baseIndex + offset;
    return vec2(vertexData[index], vertexData[index + 1]);
}

// --- Main ---
void main() {
    uint instanceID = gl_InstanceID;
    uint vertexID = gl_VertexID;

    // Get instance & mesh record
    Instance instance = instances[instanceID];
    MeshRecord record = records[instance.recordIndex];

    // Get correct vertex index
    uint index = indices[record.indexOffset + (vertexID % record.indexCount)];
    uint vertexIndex = record.vertexOffset + index;

    // Fetch vertex attributes
    vec3 vertexPosition = fetchVec3(vertexIndex, POSITION_OFFSET);
    vec3 vertexNormal   = fetchVec3(vertexIndex, NORMAL_OFFSET);
    vec3 vertexTangent  = fetchVec3(vertexIndex, TANGENT_OFFSET);
    vec2 vertexUV       = fetchVec2(vertexIndex, UV_OFFSET);

    // --- Correct Row-Major Multiplication Order ---
    gl_Position = vec4(vertexPosition, 1.0) * instance.modelMatrix * camView * camProjection;

    // Transform positions & normals
    fragmentPosition = vec3(vec4(vertexPosition, 1.0) * instance.modelMatrix);
    fragmentNormal = normalize(vec4(vertexNormal, 0.0) * transpose(inverse(instance.modelMatrix))).xyz;
    fragmentTangent = normalize(vec4(vertexTangent, 0.0) * transpose(inverse(instance.modelMatrix))).xyz;
    fragmentUV = vertexUV;

    // Compute bitangent and tangent space matrix
    vec3 bitangent = normalize(cross(fragmentNormal, fragmentTangent));
    tangentSpaceMatrix = mat3(fragmentTangent, bitangent, fragmentNormal);

    // Camera Data
    cameraPosition = camPosition;
    cameraDirection = camDirection;
}