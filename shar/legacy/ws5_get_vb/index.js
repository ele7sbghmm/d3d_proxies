const cv = document.getElementById('cv');
const ctx = cv.getContext('2d');

setInterval(() => {
    fetchVertices();
}, 1000);

async function fetchVertices() {
    const res = await fetch('/api/vertices');
    const buffer = await res.arrayBuffer();
    
    const stride = 16; 
    const view = new DataView(buffer);
    const count = buffer.byteLength / stride;

    for (let i = 0; i < count; i++) {
        let offset = i * stride;
        const x = view.getFloat32(offset, true);
        const y = view.getFloat32(offset + 4, true);
        const z = view.getFloat32(offset + 8, true);
        const c = view.getUint32(offset + 12, true);

        drawToCanvas(x, z, c);
    }
}

function drawToCanvas(x, y, c) {
    const cv = document.getElementById('cv');
    const ctx = cv.getContext('2d');

    const b = (c & 0xff);
    const g = (c >> 8) & 0xff;
    const r = (c >> 16) & 0xff;
    const a = ((c >> 24) & 0xff) / 255;
    ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${a})`;

    ctx.fillRect(x + 500, y + 500, 2, 2);
}