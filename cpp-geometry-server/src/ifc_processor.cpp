#include "ifc_processor.h"
#include <iostream>
#include <chrono>

namespace bim {
namespace geometry {

IFCProcessor::IFCProcessor() 
    : ifcFile_(nullptr)
    , iterator_(nullptr)
    , progressCallback_(nullptr) {
}

IFCProcessor::~IFCProcessor() = default;

bool IFCProcessor::loadFile(const std::string& filepath) {
    try {
        auto start = std::chrono::high_resolution_clock::now();
        
        if (progressCallback_) {
            progressCallback_(0.0f, "Loading IFC file...");
        }
        
        // Load IFC file
        ifcFile_ = std::make_unique<IfcParse::IfcFile>(filepath);
        
        if (!ifcFile_ || !ifcFile_->good()) {
            std::cerr << "Failed to load IFC file: " << filepath << std::endl;
            return false;
        }
        
        // Create geometry iterator
        IfcGeom::IteratorSettings settings;
        settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
        settings.set(IfcGeom::IteratorSettings::WELD_VERTICES, true);
        settings.set(IfcGeom::IteratorSettings::CONVERT_BACK_UNITS, true);
        settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
        
        iterator_ = std::make_unique<IfcGeom::Iterator>(settings, ifcFile_.get());
        
        if (!iterator_ || !iterator_->initialize()) {
            std::cerr << "Failed to initialize geometry iterator" << std::endl;
            return false;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (progressCallback_) {
            progressCallback_(1.0f, "IFC file loaded in " + 
                             std::to_string(duration.count()) + "ms");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception loading IFC: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::shared_ptr<Mesh>> IFCProcessor::extractGeometry() {
    std::vector<std::shared_ptr<Mesh>> meshes;
    
    if (!iterator_) {
        return meshes;
    }
    
    size_t totalObjects = getObjectCount();
    size_t processed = 0;
    
    // Iterate through all IFC products with geometry
    do {
        const IfcGeom::TriangulationElement* geom = 
            dynamic_cast<const IfcGeom::TriangulationElement*>(iterator_->get());
        
        if (geom) {
            auto mesh = convertToMesh(geom);
            if (mesh) {
                meshes.push_back(mesh);
            }
        }
        
        processed++;
        
        if (progressCallback_ && processed % 10 == 0) {
            float progress = static_cast<float>(processed) / totalObjects;
            progressCallback_(progress, 
                "Processing object " + std::to_string(processed) + 
                " / " + std::to_string(totalObjects));
        }
        
    } while (iterator_->next());
    
    return meshes;
}

std::shared_ptr<Mesh> IFCProcessor::convertToMesh(
    const IfcGeom::TriangulationElement* element) {
    
    auto mesh = std::make_shared<Mesh>();
    
    // Get triangulation
    const IfcGeom::Triangulation<double>& triangulation = element->geometry();
    
    // Vertices
    const std::vector<double>& verts = triangulation.verts();
    mesh->vertices.reserve(verts.size());
    for (double v : verts) {
        mesh->vertices.push_back(static_cast<float>(v));
    }
    
    // Normals
    const std::vector<double>& norms = triangulation.normals();
    mesh->normals.reserve(norms.size());
    for (double n : norms) {
        mesh->normals.push_back(static_cast<float>(n));
    }
    
    // Indices
    const std::vector<int>& faces = triangulation.faces();
    mesh->indices.reserve(faces.size());
    for (int idx : faces) {
        mesh->indices.push_back(static_cast<uint32_t>(idx));
    }
    
    // Material
    const IfcGeom::Material& mat = triangulation.material();
    mesh->material.set_name(mat.name());
    
    auto* diffuse = mesh->material.mutable_diffuse_color();
    diffuse->set_x(mat.diffuse()[0]);
    diffuse->set_y(mat.diffuse()[1]);
    diffuse->set_z(mat.diffuse()[2]);
    
    mesh->material.set_opacity(mat.transparency());
    
    // Calculate bounding box
    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
    
    for (size_t i = 0; i < mesh->vertices.size(); i += 3) {
        float x = mesh->vertices[i];
        float y = mesh->vertices[i + 1];
        float z = mesh->vertices[i + 2];
        
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);
    }
    
    auto* bboxMin = mesh->bbox.mutable_min();
    bboxMin->set_x(minX); bboxMin->set_y(minY); bboxMin->set_z(minZ);
    
    auto* bboxMax = mesh->bbox.mutable_max();
    bboxMax->set_x(maxX); bboxMax->set_y(maxY); bboxMax->set_z(maxZ);
    
    auto* center = mesh->bbox.mutable_center();
    center->set_x((minX + maxX) / 2.0f);
    center->set_y((minY + maxY) / 2.0f);
    center->set_z((minZ + maxZ) / 2.0f);
    
    float dx = maxX - minX;
    float dy = maxY - minY;
    float dz = maxZ - minZ;
    mesh->bbox.set_radius(std::sqrt(dx*dx + dy*dy + dz*dz) / 2.0f);
    
    // Metadata
    const IfcGeom::Element* elem = element->product();
    if (elem) {
        mesh->metadata.set_guid(elem->guid());
        mesh->metadata.set_name(elem->name());
        mesh->metadata.set_ifc_type(elem->type());
    }
    
    return mesh;
}

size_t IFCProcessor::getObjectCount() const {
    if (!ifcFile_) return 0;
    
    // Count all IfcProduct instances
    auto products = ifcFile_->instances_by_type("IfcProduct");
    return products ? products->size() : 0;
}

void IFCProcessor::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

} // namespace geometry
} // namespace bim
