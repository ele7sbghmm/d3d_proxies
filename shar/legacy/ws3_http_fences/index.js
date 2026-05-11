const h1 = document.getElementById('h1');
h1.innerText = 'workin\'';

const cv = document.getElementById('cv');
const ctx = cv.getContext('2d');

let fetched_fences = false;

setInterval(() => {
    if (fetched_fences == false) {
        fetch('/fence')
            .then(r => r.json())
            .then(fences => {
                ctx.clearRect(0, 0, cv.width, cv.height);
                ctx.beginPath();
                fences.forEach(fence => {
                    ctx.moveTo(fence.start.x, fence.start.z);
                    ctx.lineTo(fence.end.x, fence.end.z);
                    console.log(fence.start.x, fence.start.z, fence.end.x, fence.end.z);
                });
                ctx.strokeStyle = 'red';
                ctx.lineWidth = 1;
                ctx.stroke();
                
                fetched_fences = true;
            });
    }
}, 1000);
