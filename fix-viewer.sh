#!/bin/bash
echo "🔧 Corrigindo viewer..."
docker cp webgl-viewer/index.html bim-viewer:/usr/share/nginx/html/index.html
docker exec bim-viewer nginx -s reload
echo "✅ Viewer atualizado!"
echo "Abra http://localhost e pressione Cmd+Shift+R"
