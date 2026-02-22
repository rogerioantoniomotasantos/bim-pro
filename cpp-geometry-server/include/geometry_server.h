#pragma once

#include <grpcpp/grpcpp.h>
#include "geometry.grpc.pb.h"
#include "ifc_processor.h"
#include "lod_generator.h"
#include "tile_generator.h"
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace bim {
namespace geometry {

class GeometryServiceImpl final : public GeometryService::Service {
public:
    GeometryServiceImpl();
    ~GeometryServiceImpl();
    
    // gRPC service methods
    grpc::Status ProcessIFC(
        grpc::ServerContext* context,
        const ProcessRequest* request,
        grpc::ServerWriter<ProcessResponse>* writer
    ) override;
    
    grpc::Status GetObjectLOD(
        grpc::ServerContext* context,
        const ObjectLODRequest* request,
        ObjectLODResponse* response
    ) override;
    
    grpc::Status GenerateTiles(
        grpc::ServerContext* context,
        const TileRequest* request,
        grpc::ServerWriter<TileResponse>* writer
    ) override;
    
    grpc::Status HealthCheck(
        grpc::ServerContext* context,
        const HealthRequest* request,
        HealthResponse* response
    ) override;
    
private:
    // Thread pool for parallel processing
    class ThreadPool {
    public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();
        
        template<typename F>
        void enqueue(F&& task);
        
    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex queueMutex_;
        std::condition_variable condition_;
        bool stop_;
    };
    
    std::unique_ptr<ThreadPool> threadPool_;
    
    // Processors
    std::unique_ptr<IFCProcessor> ifcProcessor_;
    std::unique_ptr<LODGenerator> lodGenerator_;
    std::unique_ptr<TileGenerator> tileGenerator_;
    
    // Process single IFC file
    ProcessResult processIFCFile(
        const std::string& inputPath,
        const std::string& outputPath,
        const std::vector<int>& lodLevels,
        grpc::ServerWriter<ProcessResponse>* writer
    );
    
    // Write binary output format
    void writeBinaryOutput(
        const std::string& outputPath,
        const std::vector<std::shared_ptr<Mesh>>& meshes,
        const std::vector<int>& lodLevels
    );
    
    // Send progress update
    void sendProgress(
        grpc::ServerWriter<ProcessResponse>* writer,
        ProcessResponse::Status status,
        float progress,
        const std::string& message,
        int objectsProcessed = 0,
        int totalObjects = 0
    );
    
    // Cache for processed objects
    std::mutex cacheMutex_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Mesh>>> objectCache_;
};

} // namespace geometry
} // namespace bim
