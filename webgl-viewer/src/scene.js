class Scene {
    constructor() {
        this.objects = [];
        this.bounds = { min: [0,0,0], max: [0,0,0] };
    }
    
    addModel(model) {
        this.objects.push(...model.objects);
        this.bounds = model.bounds;
    }
    
    cull(frustum) {
        return this.objects.filter(obj => this.isVisible(obj, frustum));
    }
    
    isVisible(obj, frustum) {
        // Simple AABB frustum test
        return true; // TODO: implement proper culling
    }
}
window.Scene = Scene;
