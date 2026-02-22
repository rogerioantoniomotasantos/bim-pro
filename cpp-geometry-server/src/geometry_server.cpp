#include "geometry_server.h"
#include <fstream>
#include <iostream>
#include <chrono>

namespace bim {
namespace geometry {

GeometryServiceImpl::GeometryServiceImpl()
    : threadPool_(std::make_unique<ThreadPool>(std::thread::hardware_concurrency()))
    , ifcProcessor_(std::make_unique<IFCProcessor>())
    , lodGenerator_(std::make_unique<LODGenerator>())
    , tileGenerator_(nullptr) {
}

GeometryServiceImpl::~GeometryServiceImpl() = default;

grpc::Status GeometryServiceImpl::ProcessIFC(
    grpc::ServerContext* context,
    const ProcessRequest* request,
    grpc::ServerWriter<ProcessResponse>* writer) {
    
    std::vector<int> lodLevels(request->lod_levels().begin(), request->lod_levels().end());
    if (lodLevels.empty()) {
        lodLevels = {0, 1, 2};  // Default: all LODs
    }
    
    try {
        ProcessResult result = processIFCFile(
            request->file_path(),
            request->output_path(),
            lodLevels,
            writer
        );
        
        // Send completion message
        ProcessResponse response;
        response.set_status(ProcessResponse::COMPLETED);
        response.set_progress(1.0f);
        response.set_message("Processing completed successfully");
        *response.mutable_result() = result;
        
        writer->Write(response);
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        ProcessResponse response;
        response.set_status(ProcessResponse::FAILED);
        response.set_message(std::string("Error: ") + e.what());
        writer->Write(response);
        
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

ProcessResult GeometryServiceImpl::processIFCFile(
    const std::string& inputPath,
    const std::string& outputPath,
    const std::vector<int>& lodLevels,
    grpc::ServerWriter<ProcessResponse>* writer) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 1. Load IFC file
    sendProgress(writer, ProcessResponse::READING_IFC, 0.05f, 
                 "Loading IFC file: " + inputPath);
    
    ifcProcessor_->setProgressCallback([&](float prog, const std::string& msg) {
        sendProgress(writer, ProcessResponse::READING_IFC, 0.05f + prog * 0.1f, msg);
    });
    
    if (!ifcProcessor_->loadFile(inputPath)) {
        throw std::runtime_error("Failed to load IFC file");
    }
    
    // 2. Extract geometry
    sendProgress(writer, ProcessResponse::EXTRACTING_GEOMETRY, 0.15f, 
                 "Extracting geometry from IFC");
    
    ifcProcessor_->setProgressCallback([&](float prog, const std::string& msg) {
        sendProgress(writer, ProcessResponse::EXTRACTING_GEOMETRY, 0.15f + prog * 0.4f, msg);
    });
    
    auto meshes = ifcProcessor_->extractGeometry();
    
    sendProgress(writer, ProcessResponse::EXTRACTING_GEOMETRY, 0.55f, 
                 "Extracted " + std::to_string(meshes.size()) + " objects");
    
    // 3. Generate LODs for each mesh
    sendProgress(writer, ProcessResponse::GENERATING_LOD, 0.55f, 
                 "Generating LOD levels");
    
    std::vector<std::vector<std::shared_ptr<Mesh>>> allLODs;
    allLODs.reserve(meshes.size());
    
    for (size_t i = 0; i < meshes.size(); i++) {
        auto lods = lodGenerator_->generateLODs(meshes[i], lodLevels);
        allLODs.push_back(lods);
        
        if (i % 10 == 0) {
            float progress = 0.55f + (static_cast<float>(i) / meshes.size()) * 0.25f;
            sendProgress(writer, ProcessResponse::GENERATING_LOD, progress,
                        "Generated LODs for " + std::to_string(i) + " / " + 
                        std::to_string(meshes.size()) + " objects");
        }
    }
    
    // 4. Optimize meshes
    sendProgress(writer, ProcessResponse::OPTIMIZING_MESH, 0.80f, 
                 "Optimizing mesh data");
    
    // TODO: Add mesh optimization (vertex cache optimization, etc.)
    
    // 5. Write binary output
    sendProgress(writer, ProcessResponse::WRITING_OUTPUT, 0.90f, 
                 "Writing binary output");
    
    writeBinaryOutput(outputPath, meshes, lodLevels);
    
    // Calculate statistics
    ProcessResult result;
    result.set_output_file(outputPath);
    result.set_total_objects(meshes.size());
    
    int64_t totalVertices = 0;
    int64_t totalTriangles = 0;
    
    for (const auto& mesh : meshes) {
        totalVertices += mesh->vertexCount();
        totalTriangles += mesh->triangleCount();
    }
    
    result.set_total_vertices(totalVertices);
    result.set_total_triangles(totalTriangles);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    result.set_processing_time_seconds(duration.count() / 1000.0f);
    
    return result;
}

void GeometryServiceImpl::writeBinaryOutput(
    const std::string& outputPath,
    const std::vector<std::shared_ptr<Mesh>>& meshes,
    const std::vector<int>& lodLevels) {
    
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }
    
    // Write header
    const char magic[4] = {'B', 'I', 'M', 'G'};  // BIM Geometry
    file.write(magic, 4);
    
    uint32_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    uint32_t meshCount = meshes.size();
    file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
    
    uint32_t lodCount = lodLevels.size();
    file.write(reinterpret_cast<const char*>(&lodCount), sizeof(lodCount));
    
    // Write LOD levels
    for (int lod : lodLevels) {
        uint32_t level = lod;
        file.write(reinterpret_cast<const char*>(&level), sizeof(level));
    }
    
    // Write each mesh
    for (const auto& mesh : meshes) {
        // Vertex count
        uint32_t vertexCount = mesh->vertexCount();
        file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        
        // Triangle count
        uint32_t triangleCount = mesh->triangleCount();
        file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));
        
        // Vertices
        file.write(reinterpret_cast<const char*>(mesh->vertices.data()), 
                  mesh->vertices.size() * sizeof(float));
        
        // Normals
        file.write(reinterpret_cast<const char*>(mesh->normals.data()), 
                  mesh->normals.size() * sizeof(float));
        
        // Indices
        file.write(reinterpret_cast<const char*>(mesh->indices.data()), 
                  mesh->indices.size() * sizeof(uint32_t));
        
        // Bounding box
        file.write(reinterpret_cast<const char*>(&mesh->bbox.min().x()), sizeof(float) * 3);
        file.write(reinterpret_cast<const char*>(&mesh->bbox.max().x()), sizeof(float) * 3);
        
        // Metadata (simplified)
        uint32_t guidLen = mesh->metadata.guid().size();
        file.write(reinterpret_cast<const char*>(&guidLen), sizeof(guidLen));
        file.write(mesh->metadata.guid().data(), guidLen);
        
        uint32_t nameLen = mesh->metadata.name().size();
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        file.write(mesh->metadata.name().data(), nameLen);
    }
    
    file.close();
}

void GeometryServiceImpl::sendProgress(
    grpc::ServerWriter<ProcessResponse>* writer,
    ProcessResponse::Status status,
    float progress,
    const std::string& message,
    int objectsProcessed,
    int totalObjects) {
    
    ProcessResponse response;
    response.set_status(status);
    response.set_progress(progress);
    response.set_message(message);
    response.set_objects_processed(objectsProcessed);
    response.set_total_objects(totalObjects);
    
    writer->Write(response);
}

grpc::Status GeometryServiceImpl::HealthCheck(
    grpc::ServerContext* context,
    const HealthRequest* request,
    HealthResponse* response) {
    
    response->set_healthy(true);
    response->set_version("1.0.0");
    response->set_active_jobs(0);  // TODO: track active jobs
    
    return grpc::Status::OK;
}

// ThreadPool implementation
GeometryServiceImpl::ThreadPool::ThreadPool(size_t numThreads) 
    : stop_(false) {
    
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    condition_.wait(lock, [this] { 
                        return stop_ || !tasks_.empty(); 
                    });
                    
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                
                task();
            }
        });
    }
}

GeometryServiceImpl::ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

} // namespace geometry
} // namespace bim
