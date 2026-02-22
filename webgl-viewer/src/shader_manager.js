class ShaderManager {
    constructor(gl) {
        this.gl = gl;
        this.programs = new Map();
    }
    
    createProgram(name, sources) {
        const gl = this.gl;
        const vertShader = this.compileShader(gl.VERTEX_SHADER, sources.vertex);
        const fragShader = this.compileShader(gl.FRAGMENT_SHADER, sources.fragment);
        
        const program = gl.createProgram();
        gl.attachShader(program, vertShader);
        gl.attachShader(program, fragShader);
        gl.linkProgram(program);
        
        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
            console.error('Shader link error:', gl.getProgramInfoLog(program));
            return null;
        }
        
        const uniforms = this.getUniforms(program);
        this.programs.set(name, { program, uniforms });
        
        return this.programs.get(name);
    }
    
    compileShader(type, source) {
        const gl = this.gl;
        const shader = gl.createShader(type);
        gl.shaderSource(shader, source);
        gl.compileShader(shader);
        
        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            console.error('Shader compile error:', gl.getShaderInfoLog(shader));
            gl.deleteShader(shader);
            return null;
        }
        
        return shader;
    }
    
    getUniforms(program) {
        const gl = this.gl;
        const uniforms = {};
        const count = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
        
        for (let i = 0; i < count; i++) {
            const info = gl.getActiveUniform(program, i);
            uniforms[info.name] = gl.getUniformLocation(program, info.name);
        }
        
        return uniforms;
    }
    
    getProgram(name) {
        return this.programs.get(name);
    }
}
window.ShaderManager = ShaderManager;
