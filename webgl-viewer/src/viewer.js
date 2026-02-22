/**
 * BIM WebGL Viewer - Custom Engine
 * Pure WebGL 2.0 - No Dependencies
 * 
 * High-performance BIM model rendering
 * - Frustum culling (GPU)
 * - LOD switching
 * - Instanced rendering
 * - Custom shaders
 */

class BIMViewer {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.gl = this.canvas.getContext('webgl2', {
            alpha: false,
            antialias: true,
            depth: true,
            preserveDrawingBuffer: false,
            powerPreference: 'high-performance'
        });
        
        if (!this.gl) {
            throw new Error('WebGL 2.0 not supported');
        }
        
        // Rendering state
        this.camera = new Camera(this.canvas);
        this.scene = new Scene();
        this.renderer = new Renderer(this.gl);
        this.shaderManager = new ShaderManager(this.gl);
        
        // Performance tracking
        this.stats = {
            fps: 0,
            frameTime: 0,
            drawCalls: 0,
            triangles: 0,
            visibleObjects: 0
        };
        
        this.init();
    }
    
    init() {
        // Initialize shaders
        this.shaderManager.createProgram('basic', {
            vertex: BASIC_VERTEX_SHADER,
            fragment: BASIC_FRAGMENT_SHADER
        });
        
        this.shaderManager.createProgram('instanced', {
            vertex: INSTANCED_VERTEX_SHADER,
            fragment: INSTANCED_FRAGMENT_SHADER
        });
        
        // Setup WebGL state
        const gl = this.gl;
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.enable(gl.CULL_FACE);
        gl.cullFace(gl.BACK);
        gl.frontFace(gl.CCW);
        
        // Clear color
        gl.clearColor(0.95, 0.95, 0.95, 1.0);
        
        // Resize handler
        window.addEventListener('resize', () => this.handleResize());
        this.handleResize();
        
        console.log('✅ BIM Viewer initialized');
        console.log('   WebGL Version:', gl.getParameter(gl.VERSION));
        console.log('   Max Texture Size:', gl.getParameter(gl.MAX_TEXTURE_SIZE));
        console.log('   Max Vertex Attribs:', gl.getParameter(gl.MAX_VERTEX_ATTRIBS));
    }
    
    handleResize() {
        const dpr = window.devicePixelRatio || 1;
        this.canvas.width = this.canvas.clientWidth * dpr;
        this.canvas.height = this.canvas.clientHeight * dpr;
        
        this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
        this.camera.updateProjection(this.canvas.width / this.canvas.height);
    }
    
    async loadModel(binaryData) {
        console.log('📦 Loading BIM model...');
        const startTime = performance.now();
        
        const model = await ModelLoader.loadBinary(this.gl, binaryData);
        this.scene.addModel(model);
        
        // Fit camera to model
        this.camera.fitToBounds(model.bounds);
        
        const loadTime = performance.now() - startTime;
        console.log(`✅ Model loaded in ${loadTime.toFixed(2)}ms`);
        console.log(`   Objects: ${model.objects.length}`);
        console.log(`   Vertices: ${model.totalVertices.toLocaleString()}`);
        console.log(`   Triangles: ${model.totalTriangles.toLocaleString()}`);
        
        return model;
    }
    
    render() {
        const startTime = performance.now();
        
        // Update camera
        this.camera.update();
        
        // Frustum culling
        const frustum = this.camera.getFrustum();
        const visibleObjects = this.scene.cull(frustum);
        
        // Render
        this.renderer.render(
            this.gl,
            visibleObjects,
            this.camera,
            this.shaderManager
        );
        
        // Update stats
        const frameTime = performance.now() - startTime;
        this.updateStats(frameTime, visibleObjects.length);
        
        requestAnimationFrame(() => this.render());
    }
    
    updateStats(frameTime, visibleObjects) {
        this.stats.frameTime = frameTime;
        this.stats.fps = 1000 / frameTime;
        this.stats.visibleObjects = visibleObjects;
    }
    
    start() {
        console.log('🎬 Starting render loop...');
        this.render();
    }
    
    selectObject(x, y) {
        // TODO: GPU picking
        return null;
    }
    
    getObjectAt(x, y) {
        // TODO: Ray casting
        return null;
    }
}

// Export
window.BIMViewer = BIMViewer;
