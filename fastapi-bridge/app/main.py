"""
FastAPI Bridge - Simplified Demo Version
Serves as REST API for file upload/download
"""
from fastapi import FastAPI, UploadFile, File, HTTPException
from fastapi.responses import FileResponse, JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import os
import uuid
from pathlib import Path

app = FastAPI(title="BIM Geometry Bridge - Demo")

# CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Directories
UPLOAD_DIR = Path("/tmp/uploads")
OUTPUT_DIR = Path("/tmp/outputs")

UPLOAD_DIR.mkdir(exist_ok=True, parents=True)
OUTPUT_DIR.mkdir(exist_ok=True, parents=True)


@app.get("/")
async def root():
    return {
        "service": "BIM Geometry Bridge",
        "status": "running",
        "version": "1.0.0-demo",
        "endpoints": {
            "health": "/health",
            "upload": "POST /api/convert",
            "status": "GET /api/status/{task_id}",
            "download": "GET /api/download/{task_id}"
        }
    }


@app.post("/api/convert")
async def convert_ifc(file: UploadFile = File(...)):
    """
    Upload IFC file and convert using geometry server.
    """
    # Save uploaded file
    file_id = str(uuid.uuid4())
    ifc_path = UPLOAD_DIR / f"{file_id}.ifc"
    output_path = OUTPUT_DIR / f"{file_id}.bimg"
    
    content = await file.read()
    with open(ifc_path, "wb") as f:
        f.write(content)
    
    # Call geometry server (if available)
    geometry_server_url = os.getenv("GEOMETRY_SERVER_URL", None)
    
    if geometry_server_url:
        try:
            import requests
            response = requests.post(
                f"http://{geometry_server_url}/process",
                json={
                    "ifc_path": str(ifc_path),
                    "output_path": str(output_path)
                },
                timeout=300  # 5 minutes timeout
            )
            
            if response.status_code == 200:
                result = response.json()
                return {
                    "task_id": file_id,
                    "status": "completed",
                    "message": f"Processed successfully: {result.get('message', '')}",
                    "download_url": f"/api/download/{file_id}",
                    "note": "Processed by C++ geometry server"
                }
        except Exception as e:
            print(f"Geometry server error: {e}")
            # Fall through to demo cube
    
    # DEMO fallback: create test cube
    import struct
    
    with open(output_path, "wb") as f:
        f.write(b'BIMG')
        f.write((1).to_bytes(4, 'little'))
        f.write((1).to_bytes(4, 'little'))
        f.write((3).to_bytes(4, 'little'))
        
        vertices = [-1.0,-1.0,-1.0, 1.0,-1.0,-1.0, 1.0,1.0,-1.0, -1.0,1.0,-1.0,
                    -1.0,-1.0,1.0, 1.0,-1.0,1.0, 1.0,1.0,1.0, -1.0,1.0,1.0]
        normals = [-1.0,-1.0,-1.0, 1.0,-1.0,-1.0, 1.0,1.0,-1.0, -1.0,1.0,-1.0,
                   -1.0,-1.0,1.0, 1.0,-1.0,1.0, 1.0,1.0,1.0, -1.0,1.0,1.0]
        indices = [0,1,2, 0,2,3, 4,6,5, 4,7,6, 0,3,7, 0,7,4,
                   1,5,6, 1,6,2, 3,2,6, 3,6,7, 0,4,5, 0,5,1]
        
        f.write((8).to_bytes(4, 'little'))
        f.write((12).to_bytes(4, 'little'))
        
        for v in vertices:
            f.write(struct.pack('f', v))
        for n in normals:
            f.write(struct.pack('f', n))
        for i in indices:
            f.write(i.to_bytes(4, 'little'))
        for b in [-1.0, -1.0, -1.0, 1.0, 1.0, 1.0]:
            f.write(struct.pack('f', b))
        
        guid = f"demo-{file_id[:8]}\x00".encode('utf-8')
        name = f"Demo: {file.filename}\x00".encode('utf-8')
        
        f.write(len(guid).to_bytes(4, 'little'))
        f.write(guid)
        f.write(len(name).to_bytes(4, 'little'))
        f.write(name)
    
    return {
        "task_id": file_id,
        "status": "completed",
        "message": "Demo: Created test cube (geometry server not available)",
        "download_url": f"/api/download/{file_id}",
        "note": "To process real IFC: enable geometry-server in docker-compose"
    }


@app.get("/api/status/{task_id}")
async def get_status(task_id: str):
    """Get conversion status (demo)."""
    output_path = OUTPUT_DIR / f"{task_id}.bimg"
    
    if output_path.exists():
        return {
            "task_id": task_id,
            "status": "completed",
            "progress": 1.0,
            "download_url": f"/api/download/{task_id}"
        }
    else:
        return {
            "task_id": task_id,
            "status": "not_found",
            "message": "Task not found"
        }


@app.get("/api/download/{task_id}")
async def download_binary(task_id: str):
    """Download converted binary geometry file."""
    file_path = OUTPUT_DIR / f"{task_id}.bimg"
    
    if not file_path.exists():
        raise HTTPException(status_code=404, detail="File not found")
    
    return FileResponse(
        file_path,
        media_type="application/octet-stream",
        filename=f"model_{task_id}.bimg"
    )


@app.get("/health")
async def health():
    """Health check."""
    return {
        "status": "healthy",
        "service": "geometry-bridge-demo",
        "uploads": len(list(UPLOAD_DIR.glob("*.ifc"))),
        "outputs": len(list(OUTPUT_DIR.glob("*.bimg")))
    }


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
