#pragma once

#include <memory>
#include <vector>
#include "ifc_processor.h"

namespace bim {
namespace geometry {

class LODGenerator {
public:
    enum LODLevel {
        LOD_0 = 0,  // Bounding box only
        LOD_1 = 1,  // Simplified mesh (~1000 triangles)
        LOD_2 = 2   // Full detail
    };
    
    LODGenerator();
    ~LODGenerator();
    
    // Generate all LOD levels for a mesh
    std::vector<std::shared_ptr<Mesh>> generateLODs(
        const std::shared_ptr<Mesh>& highDetailMesh,
        const std::vector<int>& levels = {0, 1, 2}
    );
    
    // Generate specific LOD level
    std::shared_ptr<Mesh> generateLOD(
        const std::shared_ptr<Mesh>& highDetailMesh,
        LODLevel level
    );
    
    // LOD 0: Generate bounding box mesh
    std::shared_ptr<Mesh> generateBoundingBox(const BoundingBox& bbox);
    
    // LOD 1: Mesh decimation with target triangle count
    std::shared_ptr<Mesh> decimateMesh(
        const std::shared_ptr<Mesh>& mesh,
        size_t targetTriangles
    );
    
    // Set decimation parameters
    void setPreserveTopology(bool preserve) { preserveTopology_ = preserve; }
    void setPreserveBoundaries(bool preserve) { preserveBoundaries_ = preserve; }
    
private:
    bool preserveTopology_;
    bool preserveBoundaries_;
    
    // Mesh decimation using edge collapse
    std::shared_ptr<Mesh> edgeCollapseDecimation(
        const std::shared_ptr<Mesh>& mesh,
        size_t targetTriangles
    );
    
    // Calculate quadric error metric for edge
    float calculateQuadricError(
        const std::vector<float>& vertices,
        const std::vector<uint32_t>& indices,
        uint32_t v1, uint32_t v2
    );
    
    // Recalculate normals after decimation
    void recalculateNormals(std::shared_ptr<Mesh>& mesh);
};

} // namespace geometry
} // namespace bim
