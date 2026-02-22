#include "lod_generator.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

namespace bim {
namespace geometry {

LODGenerator::LODGenerator()
    : preserveTopology_(true)
    , preserveBoundaries_(true) {
}

LODGenerator::~LODGenerator() = default;

std::vector<std::shared_ptr<Mesh>> LODGenerator::generateLODs(
    const std::shared_ptr<Mesh>& highDetailMesh,
    const std::vector<int>& levels) {
    
    std::vector<std::shared_ptr<Mesh>> lods;
    lods.reserve(levels.size());
    
    for (int level : levels) {
        auto lodMesh = generateLOD(highDetailMesh, static_cast<LODLevel>(level));
        if (lodMesh) {
            lods.push_back(lodMesh);
        }
    }
    
    return lods;
}

std::shared_ptr<Mesh> LODGenerator::generateLOD(
    const std::shared_ptr<Mesh>& highDetailMesh,
    LODLevel level) {
    
    switch (level) {
        case LOD_0:
            return generateBoundingBox(highDetailMesh->bbox);
            
        case LOD_1: {
            // Target: ~1000 triangles
            size_t targetTriangles = std::min(
                static_cast<size_t>(1000),
                highDetailMesh->triangleCount()
            );
            return decimateMesh(highDetailMesh, targetTriangles);
        }
            
        case LOD_2:
            // Full detail - just copy
            return std::make_shared<Mesh>(*highDetailMesh);
            
        default:
            return nullptr;
    }
}

std::shared_ptr<Mesh> LODGenerator::generateBoundingBox(const BoundingBox& bbox) {
    auto mesh = std::make_shared<Mesh>();
    
    float minX = bbox.min().x();
    float minY = bbox.min().y();
    float minZ = bbox.min().z();
    float maxX = bbox.max().x();
    float maxY = bbox.max().y();
    float maxZ = bbox.max().z();
    
    // 8 vertices of box
    mesh->vertices = {
        minX, minY, minZ,  // 0
        maxX, minY, minZ,  // 1
        maxX, maxY, minZ,  // 2
        minX, maxY, minZ,  // 3
        minX, minY, maxZ,  // 4
        maxX, minY, maxZ,  // 5
        maxX, maxY, maxZ,  // 6
        minX, maxY, maxZ   // 7
    };
    
    // 12 triangles (2 per face)
    mesh->indices = {
        // Bottom
        0, 1, 2,  0, 2, 3,
        // Top
        4, 6, 5,  4, 7, 6,
        // Front
        0, 5, 1,  0, 4, 5,
        // Back
        2, 7, 3,  2, 6, 7,
        // Left
        0, 3, 7,  0, 7, 4,
        // Right
        1, 6, 2,  1, 5, 6
    };
    
    // Simple normals (face normals)
    mesh->normals.resize(24, 0.0f);  // 8 vertices * 3 components
    
    // Calculate normals
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        uint32_t i0 = mesh->indices[i] * 3;
        uint32_t i1 = mesh->indices[i + 1] * 3;
        uint32_t i2 = mesh->indices[i + 2] * 3;
        
        float v0x = mesh->vertices[i1] - mesh->vertices[i0];
        float v0y = mesh->vertices[i1 + 1] - mesh->vertices[i0 + 1];
        float v0z = mesh->vertices[i1 + 2] - mesh->vertices[i0 + 2];
        
        float v1x = mesh->vertices[i2] - mesh->vertices[i0];
        float v1y = mesh->vertices[i2 + 1] - mesh->vertices[i0 + 1];
        float v1z = mesh->vertices[i2 + 2] - mesh->vertices[i0 + 2];
        
        // Cross product
        float nx = v0y * v1z - v0z * v1y;
        float ny = v0z * v1x - v0x * v1z;
        float nz = v0x * v1y - v0y * v1x;
        
        // Normalize
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 0.0001f) {
            nx /= len; ny /= len; nz /= len;
        }
        
        // Add to all 3 vertices
        for (int j = 0; j < 3; j++) {
            uint32_t idx = mesh->indices[i + j] * 3;
            mesh->normals[idx] += nx;
            mesh->normals[idx + 1] += ny;
            mesh->normals[idx + 2] += nz;
        }
    }
    
    // Normalize all normals
    for (size_t i = 0; i < mesh->normals.size(); i += 3) {
        float len = std::sqrt(
            mesh->normals[i] * mesh->normals[i] +
            mesh->normals[i + 1] * mesh->normals[i + 1] +
            mesh->normals[i + 2] * mesh->normals[i + 2]
        );
        if (len > 0.0001f) {
            mesh->normals[i] /= len;
            mesh->normals[i + 1] /= len;
            mesh->normals[i + 2] /= len;
        }
    }
    
    mesh->bbox = bbox;
    
    return mesh;
}

std::shared_ptr<Mesh> LODGenerator::decimateMesh(
    const std::shared_ptr<Mesh>& mesh,
    size_t targetTriangles) {
    
    if (mesh->triangleCount() <= targetTriangles) {
        return std::make_shared<Mesh>(*mesh);
    }
    
    return edgeCollapseDecimation(mesh, targetTriangles);
}

std::shared_ptr<Mesh> LODGenerator::edgeCollapseDecimation(
    const std::shared_ptr<Mesh>& mesh,
    size_t targetTriangles) {
    
    // Simplified edge collapse algorithm
    // Full implementation would use quadric error metrics
    // This is a basic version for demonstration
    
    auto decimated = std::make_shared<Mesh>(*mesh);
    
    // Build edge map
    struct Edge {
        uint32_t v0, v1;
        float error;
        
        bool operator<(const Edge& other) const {
            return error > other.error;  // Min heap
        }
    };
    
    std::priority_queue<Edge> edgeQueue;
    std::unordered_map<uint64_t, bool> collapsedEdges;
    
    auto makeEdgeKey = [](uint32_t v0, uint32_t v1) -> uint64_t {
        if (v0 > v1) std::swap(v0, v1);
        return (static_cast<uint64_t>(v0) << 32) | v1;
    };
    
    // Build initial edge queue
    for (size_t i = 0; i < decimated->indices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            uint32_t v0 = decimated->indices[i + j];
            uint32_t v1 = decimated->indices[i + ((j + 1) % 3)];
            
            float error = calculateQuadricError(
                decimated->vertices, 
                decimated->indices, 
                v0, v1
            );
            
            edgeQueue.push({v0, v1, error});
        }
    }
    
    // Collapse edges until target reached
    size_t currentTriangles = decimated->triangleCount();
    
    while (currentTriangles > targetTriangles && !edgeQueue.empty()) {
        Edge edge = edgeQueue.top();
        edgeQueue.pop();
        
        uint64_t edgeKey = makeEdgeKey(edge.v0, edge.v1);
        if (collapsedEdges[edgeKey]) continue;
        
        // Collapse edge v0 -> v1
        // Replace all occurrences of v1 with v0
        for (size_t i = 0; i < decimated->indices.size(); i++) {
            if (decimated->indices[i] == edge.v1) {
                decimated->indices[i] = edge.v0;
            }
        }
        
        collapsedEdges[edgeKey] = true;
        currentTriangles--;
    }
    
    // Rebuild mesh without degenerate triangles
    std::vector<uint32_t> newIndices;
    for (size_t i = 0; i < decimated->indices.size(); i += 3) {
        uint32_t i0 = decimated->indices[i];
        uint32_t i1 = decimated->indices[i + 1];
        uint32_t i2 = decimated->indices[i + 2];
        
        // Skip degenerate triangles
        if (i0 != i1 && i1 != i2 && i0 != i2) {
            newIndices.push_back(i0);
            newIndices.push_back(i1);
            newIndices.push_back(i2);
        }
    }
    
    decimated->indices = std::move(newIndices);
    
    // Recalculate normals
    recalculateNormals(decimated);
    
    return decimated;
}

float LODGenerator::calculateQuadricError(
    const std::vector<float>& vertices,
    const std::vector<uint32_t>& indices,
    uint32_t v1, uint32_t v2) {
    
    // Simplified error metric: edge length
    float x1 = vertices[v1 * 3];
    float y1 = vertices[v1 * 3 + 1];
    float z1 = vertices[v1 * 3 + 2];
    
    float x2 = vertices[v2 * 3];
    float y2 = vertices[v2 * 3 + 1];
    float z2 = vertices[v2 * 3 + 2];
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

void LODGenerator::recalculateNormals(std::shared_ptr<Mesh>& mesh) {
    // Zero out normals
    mesh->normals.assign(mesh->vertices.size(), 0.0f);
    
    // Calculate face normals and accumulate
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        uint32_t i0 = mesh->indices[i] * 3;
        uint32_t i1 = mesh->indices[i + 1] * 3;
        uint32_t i2 = mesh->indices[i + 2] * 3;
        
        float v0x = mesh->vertices[i1] - mesh->vertices[i0];
        float v0y = mesh->vertices[i1 + 1] - mesh->vertices[i0 + 1];
        float v0z = mesh->vertices[i1 + 2] - mesh->vertices[i0 + 2];
        
        float v1x = mesh->vertices[i2] - mesh->vertices[i0];
        float v1y = mesh->vertices[i2 + 1] - mesh->vertices[i0 + 1];
        float v1z = mesh->vertices[i2 + 2] - mesh->vertices[i0 + 2];
        
        // Cross product
        float nx = v0y * v1z - v0z * v1y;
        float ny = v0z * v1x - v0x * v1z;
        float nz = v0x * v1y - v0y * v1x;
        
        // Add to all vertices of triangle
        for (int j = 0; j < 3; j++) {
            uint32_t idx = mesh->indices[i + j] * 3;
            mesh->normals[idx] += nx;
            mesh->normals[idx + 1] += ny;
            mesh->normals[idx + 2] += nz;
        }
    }
    
    // Normalize
    for (size_t i = 0; i < mesh->normals.size(); i += 3) {
        float len = std::sqrt(
            mesh->normals[i] * mesh->normals[i] +
            mesh->normals[i + 1] * mesh->normals[i + 1] +
            mesh->normals[i + 2] * mesh->normals[i + 2]
        );
        if (len > 0.0001f) {
            mesh->normals[i] /= len;
            mesh->normals[i + 1] /= len;
            mesh->normals[i + 2] /= len;
        }
    }
}

} // namespace geometry
} // namespace bim
