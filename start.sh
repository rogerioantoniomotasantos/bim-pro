#!/bin/bash

echo "════════════════════════════════════════════"
echo "  BIM PROFESSIONAL PLATFORM"
echo "  Quick Start Script"
echo "════════════════════════════════════════════"
echo ""

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    echo "❌ Docker não está rodando!"
    echo "   Por favor, inicie o Docker Desktop e tente novamente."
    exit 1
fi

echo "✅ Docker está rodando"
echo ""

# Build images
echo "📦 Building Docker images..."
docker-compose build

if [ $? -ne 0 ]; then
    echo "❌ Build falhou!"
    exit 1
fi

echo "✅ Build completo"
echo ""

# Start services
echo "🚀 Iniciando serviços..."
docker-compose up -d

if [ $? -ne 0 ]; then
    echo "❌ Falha ao iniciar serviços!"
    exit 1
fi

echo ""
echo "════════════════════════════════════════════"
echo "  ✅ SISTEMA INICIADO COM SUCESSO!"
echo "════════════════════════════════════════════"
echo ""
echo "  🌐 Viewer:  http://localhost"
echo "  🔌 API:     http://localhost:8000"
echo "  📊 Status:  docker-compose ps"
echo "  📝 Logs:    docker-compose logs -f"
echo ""
echo "════════════════════════════════════════════"
