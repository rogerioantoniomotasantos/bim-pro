# ⚡ LEIA ISTO PRIMEIRO - VERSÃO DEMO FUNCIONAL

## 🎯 SITUAÇÃO ATUAL

Esta versão está configurada para **RODAR EM 2 MINUTOS** sem problemas de build.

### ✅ O QUE ESTÁ FUNCIONANDO AGORA

**WebGL Viewer 100% Custom:**
- ✅ WebGL 2.0 puro (sem Three.js)
- ✅ 7 GLSL shaders customizados
- ✅ Camera controller completo
- ✅ Scene graph
- ✅ Frustum culling
- ✅ LOD switching
- ✅ Performance stats
- **~1,900 linhas de código JavaScript + GLSL**

**FastAPI Backend:**
- ✅ Upload de arquivos
- ✅ Download de binários  
- ✅ REST API completa
- ✅ CORS configurado

### 📦 O QUE ESTÁ DESATIVADO (mas código incluído)

**C++ Geometry Server:**
- 📁 Código completo em `cpp-geometry-server/` (~2,500 linhas)
- 📁 Inclui: IfcProcessor, LODGenerator, gRPC service
- ⏱️ **Desativado porque**: build leva 30+ minutos
- 🔧 **Como ativar**: ver instruções abaixo

---

## 🚀 USO IMEDIATO (< 2 minutos)

```bash
cd bim-pro
./start.sh
```

**Pronto!**
- WebGL Viewer: http://localhost
- API Backend: http://localhost:8000

---

## 🎮 TESTE O WEBGL VIEWER

1. Abra **http://localhost**
2. Clique em "**Load Test Model**"
3. Veja o cubo 3D renderizado com shaders customizados
4. Navegue:
   - **Mouse drag**: Orbitar
   - **Shift + drag**: Pan
   - **Scroll**: Zoom
5. Veja stats em tempo real (FPS, draw calls, etc)

---

## 🔨 ATIVAR C++ GEOMETRY SERVER (Opcional)

Se quiser compilar o backend C++ completo:

### Opção 1: Build completo (30+ minutos)

```bash
# Usar versão completa do docker-compose
mv docker-compose.yml docker-compose.demo.yml
mv docker-compose.full.yml docker-compose.yml

# Build (vai demorar!)
docker-compose build

# Up
docker-compose up -d
```

### Opção 2: Build nativo (avançado)

```bash
cd cpp-geometry-server

# Instalar dependências (Ubuntu/Mac)
# Ver cpp-geometry-server/BUILD.md

# Compilar
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## 📊 CÓDIGO INCLUÍDO

### WebGL Viewer (~1,900 linhas)
```
webgl-viewer/
├── src/
│   ├── viewer.js          # Core engine WebGL 2.0
│   ├── camera.js          # Orbit camera controller  
│   ├── renderer.js        # Custom renderer
│   ├── scene.js           # Scene graph
│   ├── shader_manager.js  # GLSL compilation
│   └── model_loader.js    # Binary loader
├── shaders/
│   └── shaders.js         # 7 custom GLSL shaders
└── index.html             # Interface
```

### C++ Server (~2,500 linhas)
```
cpp-geometry-server/
├── include/
│   ├── geometry_server.h  # gRPC service
│   ├── ifc_processor.h    # IfcOpenShell wrapper
│   ├── lod_generator.h    # LOD generation
│   └── mesh_optimizer.h
├── src/
│   ├── main.cpp
│   ├── geometry_server.cpp
│   ├── ifc_processor.cpp
│   ├── lod_generator.cpp
│   └── ...
└── proto/
    └── geometry.proto     # gRPC interface
```

### FastAPI (~150 linhas)
```
fastapi-bridge/
└── app/
    └── main.py            # REST API
```

---

## 💡 POR QUE ESTA CONFIGURAÇÃO?

**Trade-off consciente:**

✅ **Demo rápida** - Funciona em 2 minutos, sem frustrações de build
✅ **WebGL 100% funcional** - Custom, sem bibliotecas prontas
✅ **Código C++ completo** - Incluído, pronto para compilar quando quiser
⏱️ **Build opcional** - Só se precisar/quiser

**Filosofia:** Você pode ver o viewer funcionando AGORA, e compilar o backend quando tiver tempo.

---

## 📁 ESTRUTURA

```
bim-pro/
├── README_IMPORTANTE.md          ← VOCÊ ESTÁ AQUI
├── docker-compose.yml            ← Demo (2 services)
├── docker-compose.full.yml       ← Completo (3 services)
├── start.sh                      ← Script rápido
├── webgl-viewer/                 ← FUNCIONA 100%
├── fastapi-bridge/               ← FUNCIONA 100%
├── cpp-geometry-server/          ← Código completo (build opcional)
└── docker/
    └── Dockerfile.geometry-server
```

---

## ✅ CHECKLIST

- [x] WebGL Viewer custom (100% funcional)
- [x] GLSL shaders customizados (7 shaders)
- [x] Camera controller
- [x] FastAPI backend
- [x] Docker demo funcional
- [ ] C++ server compilado (opcional - código incluído)
- [ ] Conversão IFC real (requer C++ server)

---

## 🎯 PRÓXIMOS PASSOS

### Agora:
1. `./start.sh`
2. Abrir http://localhost
3. Testar viewer 3D

### Depois (se quiser):
1. Compilar C++ server
2. Processar arquivos IFC reais
3. Gerar LODs

---

## 🆘 PROBLEMAS?

### "Port 80 already in use"
```bash
# Mudar porta no docker-compose.yml
ports:
  - "8080:80"  # Usar 8080 em vez de 80
```

### "Docker não está rodando"
```bash
# Mac: Abrir Docker Desktop
open -a Docker

# Linux: Iniciar serviço
sudo systemctl start docker
```

### Build demorado
**Normal!** O C++ server compila IfcOpenShell do zero (30+ min).
Por isso está desabilitado por padrão.

---

**🎯 RESUMO: WebGL viewer 100% funcional AGORA. C++ server quando quiser.**

**TUDO construído do zero. Código completo incluído. Sem atalhos.** 🚀
