class Renderer {
    constructor(gl) {
        this.gl = gl;
        this.drawCalls = 0;
    }
    
    render(gl, objects, camera, shaderManager) {
        this.drawCalls = 0;
        
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        
        const program = shaderManager.getProgram('basic');
        gl.useProgram(program.program);
        
        gl.uniformMatrix4fv(program.uniforms.u_viewMatrix, false, camera.viewMatrix);
        gl.uniformMatrix4fv(program.uniforms.u_projectionMatrix, false, camera.projectionMatrix);
        
        for (const obj of objects) {
            this.renderObject(gl, obj, program);
            this.drawCalls++;
        }
    }
    
    renderObject(gl, obj, program) {
        gl.bindVertexArray(obj.vao);
        gl.uniformMatrix4fv(program.uniforms.u_modelMatrix, false, obj.modelMatrix);
        gl.drawElements(gl.TRIANGLES, obj.indexCount, gl.UNSIGNED_INT, 0);
        gl.bindVertexArray(null);
    }
}
window.Renderer = Renderer;
