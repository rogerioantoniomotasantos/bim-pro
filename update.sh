#!/bin/bash

echo "🔄 Atualizando BIM Viewer..."
echo ""

# Parar containers
echo "⏸️  Parando containers..."
docker-compose down

# Aguardar
sleep 2

# Reiniciar
echo "🚀 Reiniciando containers..."
docker-compose up -d

# Aguardar
echo "⏳ Aguardando inicialização..."
sleep 5

# Verificar
echo ""
echo "✅ Status dos containers:"
docker-compose ps

echo ""
echo "════════════════════════════════════════════"
echo "  ✅ ATUALIZAÇÃO COMPLETA!"
echo "════════════════════════════════════════════"
echo ""
echo "  🌐 Abrir: http://localhost"
echo "  📝 Pressione Cmd+Shift+R para recarregar"
echo ""
echo "  Agora o viewer aceita arquivos .ifc!"
echo ""
