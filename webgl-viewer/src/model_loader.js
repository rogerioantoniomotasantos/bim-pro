class ModelLoader {
    static async loadBinary(gl, arrayBuffer) {
        const view = new DataView(arrayBuffer);
        let offset = 0;
        
        // Read header
        const magic = String.fromCharCode(
            view.getUint8(offset++),
            view.getUint8(offset++),
            view.getUint8(offset++),
            view.getUint8(offset++)
        );
        
        if (magic !== 'BIMG') {
            throw new Error('Invalid file format');
        }
        
        const version = view.getUint32(offset, true); offset += 4;
        const meshCount = view.getUint32(offset, true); offset += 4;
        const lodCount = view.getUint32(offset, true); offset += 4;
        
        // Skip LOD levels
        offset += lodCount * 4;
        
        const objects = [];
        let bounds = {
            min: [Infinity, Infinity, Infinity],
            max: [-Infinity, -Infinity, -Infinity]
        };
        
        for (let i = 0; i < meshCount; i++) {
            const obj = this.readMesh(gl, view, offset);
            objects.push(obj.object);
            offset = obj.newOffset;
            
            // Update bounds
            for (let j = 0; j < 3; j++) {
                bounds.min[j] = Math.min(bounds.min[j], obj.bboxMin[j]);
                bounds.max[j] = Math.max(bounds.max[j], obj.bboxMax[j]);
            }
        }
        
        return { objects, bounds, totalVertices: 0, totalTriangles: 0 };
    }
    
    static readMesh(gl, view, offset) {
        const vertexCount = view.getUint32(offset, true); offset += 4;
        const triangleCount = view.getUint32(offset, true); offset += 4;
        
        // Read vertices
        const vertices = new Float32Array(vertexCount * 3);
        for (let i = 0; i < vertexCount * 3; i++) {
            vertices[i] = view.getFloat32(offset, true);
            offset += 4;
        }
        
        // Read normals
        const normals = new Float32Array(vertexCount * 3);
        for (let i = 0; i < vertexCount * 3; i++) {
            normals[i] = view.getFloat32(offset, true);
            offset += 4;
        }
        
        // Read indices
        const indices = new Uint32Array(triangleCount * 3);
        for (let i = 0; i < triangleCount * 3; i++) {
            indices[i] = view.getUint32(offset, true);
            offset += 4;
        }
        
        // Read bounding box
        const bboxMin = [
            view.getFloat32(offset, true), offset += 4,
            view.getFloat32(offset, true), offset += 4,
            view.getFloat32(offset, true)
        ]; offset += 4;
        
        const bboxMax = [
            view.getFloat32(offset, true), offset += 4,
            view.getFloat32(offset, true), offset += 4,
            view.getFloat32(offset, true)
        ]; offset += 4;
        
        // Skip metadata for now
        const guidLen = view.getUint32(offset, true); offset += 4;
        offset += guidLen;
        const nameLen = view.getUint32(offset, true); offset += 4;
        offset += nameLen;
        
        // Create WebGL buffers
        const vao = gl.createVertexArray();
        gl.bindVertexArray(vao);
        
        const vbo = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
        gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);
        
        const nbo = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, nbo);
        gl.bufferData(gl.ARRAY_BUFFER, normals, gl.STATIC_DRAW);
        gl.enableVertexAttribArray(1);
        gl.vertexAttribPointer(1, 3, gl.FLOAT, false, 0, 0);
        
        const ibo = gl.createBuffer();
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);
        
        gl.bindVertexArray(null);
        
        return {
            object: {
                vao,
                indexCount: indices.length,
                modelMatrix: mat4.create()
            },
            bboxMin,
            bboxMax,
            newOffset: offset
        };
    }
}
window.ModelLoader = ModelLoader;
