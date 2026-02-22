#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ifcparse/IfcFile.h>
#include <ifcgeom/IfcGeomIterator.h>
#include "geometry.pb.h"

namespace bim {
namespace geometry {

struct Mesh {
    std::vector<float> vertices;      // [x,y,z, x,y,z, ...]
    std::vector<float> normals;       // [nx,ny,nz, ...]
    std::vector<uint32_t> indices;    // Triangle indices
    std::vector<float> colors;        // [r,g,b,a, ...]
    
    BoundingBox bbox;
    MaterialInfo material;
    ObjectMetadata metadata;
    
    size_t vertexCount() const { return vertices.size() / 3; }
    size_t triangleCount() const { return indices.size() / 3; }
};

class IFCProcessor {
public:
    IFCProcessor();
    ~IFCProcessor();
    
    // Load IFC file
    bool loadFile(const std::string& filepath);
    
    // Extract all geometry
    std::vector<std::shared_ptr<Mesh>> extractGeometry();
    
    // Extract specific object
    std::shared_ptr<Mesh> extractObject(const std::string& guid);
    
    // Get total object count
    size_t getObjectCount() const;
    
    // Progress callback
    using ProgressCallback = std::function<void(float progress, const std::string& message)>;
    void setProgressCallback(ProgressCallback callback);
    
private:
    std::unique_ptr<IfcParse::IfcFile> ifcFile_;
    std::unique_ptr<IfcGeom::Iterator> iterator_;
    ProgressCallback progressCallback_;
    
    // Convert IfcOpenShell geometry to our Mesh format
    std::shared_ptr<Mesh> convertToMesh(const IfcGeom::TriangulationElement* element);
    
    // Extract metadata from IFC entity
    ObjectMetadata extractMetadata(const IfcUtil::IfcBaseClass* entity);
};

} // namespace geometry
} // namespace bim
