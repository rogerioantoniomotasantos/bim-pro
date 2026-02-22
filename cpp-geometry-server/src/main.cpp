#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "geometry_server.h"

void RunServer(const std::string& serverAddress) {
    bim::geometry::GeometryServiceImpl service;
    
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    
    grpc::ServerBuilder builder;
    
    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    
    // Register service
    builder.RegisterService(&service);
    
    // Set max message size (500 MB for large IFC files)
    builder.SetMaxReceiveMessageSize(500 * 1024 * 1024);
    builder.SetMaxSendMessageSize(500 * 1024 * 1024);
    
    // Assemble and start the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    std::cout << "🚀 BIM Geometry Server listening on " << serverAddress << std::endl;
    std::cout << "⚡ Using " << std::thread::hardware_concurrency() << " threads" << std::endl;
    std::cout << "📦 Max message size: 500 MB" << std::endl;
    std::cout << "✅ gRPC Health Check enabled" << std::endl;
    std::cout << "🔍 gRPC Reflection enabled" << std::endl;
    
    // Wait for the server to shutdown
    server->Wait();
}

int main(int argc, char** argv) {
    std::string serverAddress = "0.0.0.0:50051";
    
    if (argc > 1) {
        serverAddress = argv[1];
    }
    
    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "   BIM GEOMETRY SERVER v1.0.0" << std::endl;
    std::cout << "   High-Performance IFC Processing" << std::endl;
    std::cout << "   C++17 | IfcOpenShell | gRPC" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    try {
        RunServer(serverAddress);
    } catch (const std::exception& e) {
        std::cerr << "❌ Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
