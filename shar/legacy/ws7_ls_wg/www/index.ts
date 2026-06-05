async function main() {
    console.log('foo')
    console.log('navigator: ', navigator)
    console.log('gpu: ', navigator.gpu)
    const adapter = await navigator.gpu.requestAdapter()
    console.log('adapter: ', adapter)
    if (!adapter) return;
    const device = await adapter.requestDevice()
    const format = navigator.gpu.getPreferredCanvasFormat()

    const canvas = document.querySelector('canvas')
    const context = canvas!.getContext('webgpu') as GPUCanvasContext
    context.configure({ device, format })

    const vert_wgsl = `
        @vertex
        fn vs_main(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
            var pos = array<vec2f, 3>(
                vec2f(.5, .5),
                vec2f(-.5, -.5),
                vec2f(.5, -.5)
            );
            return vec4f(pos[i], 0., 1.);
        }
        @fragment
        fn fs_main() -> @location(0) vec4f {
            return vec4f(0., 1., 0., 1.);
        }
    `

    const shaderModule = device?.createShaderModule({
        code: vert_wgsl
    })

    const pipeline = device.createRenderPipeline({
        layout: 'auto',
        vertex: {
            module: shaderModule,
            entryPoint: 'vs_main',
        }, fragment: {
            module: shaderModule,
            entryPoint: 'fs_main',
            targets: [{ format, }],
        }, primitive: { topology: 'triangle-list' },
    })

    const encoder = device.createCommandEncoder()

    const pass = encoder.beginRenderPass({
        colorAttachments: [{
            view: context.getCurrentTexture().createView(),
            clearValue: { r: .1, g: .1, b: .1, a: 1. },
            loadOp: 'clear',
            storeOp: 'store'
        }]
    })

    pass.setPipeline(pipeline)
    pass.draw(3)
    pass.end()

    device.queue.submit([ encoder.finish() ])
}

main()
