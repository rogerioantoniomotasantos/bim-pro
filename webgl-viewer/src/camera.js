/**
 * Camera Controller
 * Orbit camera with smooth controls
 */

class Camera {
    constructor(canvas) {
        this.canvas = canvas;
        
        // Camera position
        this.position = vec3.fromValues(10, 10, 10);
        this.target = vec3.fromValues(0, 0, 0);
        this.up = vec3.fromValues(0, 1, 0);
        
        // Projection
        this.fov = 45 * Math.PI / 180;
        this.near = 0.1;
        this.far = 10000;
        this.aspect = 1;
        
        // Matrices
        this.viewMatrix = mat4.create();
        this.projectionMatrix = mat4.create();
        this.viewProjectionMatrix = mat4.create();
        
        // Orbit controls
        this.distance = 20;
        this.azimuth = 45 * Math.PI / 180;
        this.elevation = 30 * Math.PI / 180;
        
        this.setupControls();
        this.update();
    }
    
    setupControls() {
        let isDragging = false;
        let lastX = 0, lastY = 0;
        
        this.canvas.addEventListener('mousedown', (e) => {
            isDragging = true;
            lastX = e.clientX;
            lastY = e.clientY;
        });
        
        window.addEventListener('mousemove', (e) => {
            if (!isDragging) return;
            
            const deltaX = e.clientX - lastX;
            const deltaY = e.clientY - lastY;
            
            if (e.shiftKey) {
                // Pan
                this.pan(deltaX * 0.01, -deltaY * 0.01);
            } else {
                // Orbit
                this.azimuth -= deltaX * 0.01;
                this.elevation += deltaY * 0.01;
                this.elevation = Math.max(-Math.PI / 2 + 0.01, 
                                        Math.min(Math.PI / 2 - 0.01, this.elevation));
            }
            
            lastX = e.clientX;
            lastY = e.clientY;
        });
        
        window.addEventListener('mouseup', () => {
            isDragging = false;
        });
        
        this.canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            this.distance *= (1 + e.deltaY * 0.001);
            this.distance = Math.max(1, Math.min(1000, this.distance));
        });
    }
    
    pan(dx, dy) {
        const right = vec3.create();
        const up = vec3.create();
        
        vec3.subtract(right, this.position, this.target);
        vec3.cross(up, right, this.up);
        vec3.normalize(right, right);
        vec3.normalize(up, up);
        
        vec3.scaleAndAdd(this.target, this.target, right, -dx * this.distance * 0.1);
        vec3.scaleAndAdd(this.target, this.target, up, dy * this.distance * 0.1);
    }
    
    update() {
        // Calculate position from spherical coordinates
        const x = this.target[0] + this.distance * Math.cos(this.elevation) * Math.sin(this.azimuth);
        const y = this.target[1] + this.distance * Math.sin(this.elevation);
        const z = this.target[2] + this.distance * Math.cos(this.elevation) * Math.cos(this.azimuth);
        
        vec3.set(this.position, x, y, z);
        
        // Update view matrix
        mat4.lookAt(this.viewMatrix, this.position, this.target, this.up);
        
        // Update combined matrix
        mat4.multiply(this.viewProjectionMatrix, this.projectionMatrix, this.viewMatrix);
    }
    
    updateProjection(aspect) {
        this.aspect = aspect;
        mat4.perspective(this.projectionMatrix, this.fov, this.aspect, this.near, this.far);
    }
    
    fitToBounds(bounds) {
        // Center camera on bounds
        this.target = vec3.fromValues(
            (bounds.min[0] + bounds.max[0]) / 2,
            (bounds.min[1] + bounds.max[1]) / 2,
            (bounds.min[2] + bounds.max[2]) / 2
        );
        
        // Calculate distance to fit all geometry
        const size = vec3.create();
        vec3.subtract(size, bounds.max, bounds.min);
        const maxSize = Math.max(size[0], size[1], size[2]);
        
        this.distance = maxSize / Math.tan(this.fov / 2) * 1.5;
    }
    
    getFrustum() {
        // Extract frustum planes from view-projection matrix
        const vp = this.viewProjectionMatrix;
        const planes = [];
        
        // Left
        planes.push(vec4.fromValues(
            vp[3] + vp[0], vp[7] + vp[4], vp[11] + vp[8], vp[15] + vp[12]
        ));
        
        // Right  
        planes.push(vec4.fromValues(
            vp[3] - vp[0], vp[7] - vp[4], vp[11] - vp[8], vp[15] - vp[12]
        ));
        
        // Bottom
        planes.push(vec4.fromValues(
            vp[3] + vp[1], vp[7] + vp[5], vp[11] + vp[9], vp[15] + vp[13]
        ));
        
        // Top
        planes.push(vec4.fromValues(
            vp[3] - vp[1], vp[7] - vp[5], vp[11] - vp[9], vp[15] - vp[13]
        ));
        
        // Near
        planes.push(vec4.fromValues(
            vp[3] + vp[2], vp[7] + vp[6], vp[11] + vp[10], vp[15] + vp[14]
        ));
        
        // Far
        planes.push(vec4.fromValues(
            vp[3] - vp[2], vp[7] - vp[6], vp[11] - vp[10], vp[15] - vp[14]
        ));
        
        // Normalize planes
        planes.forEach(plane => {
            const len = Math.sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
            vec4.scale(plane, plane, 1 / len);
        });
        
        return planes;
    }
}

window.Camera = Camera;
