#!/bin/bash

echo "════════════════════════════════════════════"
echo "  BIM PROFESSIONAL - Full Build"
echo "  Com processamento IFC REAL"
echo "════════════════════════════════════════════"
echo ""

echo "📦 1. Ativando versão completa..."
if [ -f docker-compose.demo.yml ]; then
    echo "   Versão demo já salva"
else
    mv docker-compose.yml docker-compose.demo.yml
    mv docker-compose.full.yml docker-compose.yml
fi

echo ""
echo "🔨 2. Building containers..."
echo "   ⚠️  Isso vai demorar 5-10 minutos..."
echo ""

docker-compose build

echo ""
echo "🚀 3. Iniciando serviços..."
docker-compose up -d

echo ""
echo "⏳ 4. Aguardando inicialização..."
sleep 10

echo ""
echo "✅ 5. Verificando status..."
docker-compose ps

echo ""
echo "════════════════════════════════════════════"
echo "  ✅ BUILD COMPLETO!"
echo "════════════════════════════════════════════"
echo ""
echo "  🌐 Viewer: http://localhost"
echo "  📡 API: http://localhost:8080"
echo ""
echo "  Agora você pode processar arquivos IFC REAIS!"
echo ""
echo "  Teste:"
echo "  1. Abra http://localhost"
echo "  2. Clique 'Load IFC File'"
echo "  3. Selecione seu arquivo .ifc"
echo "  4. Aguarde processamento (pode demorar)"
echo "  5. Veja seu modelo 3D renderizado!"
echo ""
