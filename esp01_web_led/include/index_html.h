#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP Control | GPIO2</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary: #00f2fe;
            --secondary: #4facfe;
            --dark: #0f172a;
            --glass: rgba(255, 255, 255, 0.05);
            --text: #f8fafc;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Outfit', sans-serif;
        }

        body {
            background: var(--dark);
            color: var(--text);
            overflow: hidden;
            background: radial-gradient(circle at top right, #1e293b, #0f172a);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .header {
            padding: 1.5rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            backdrop-filter: blur(10px);
            position: fixed;
            width: 100%;
            top: 0;
            z-index: 100;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }

        .logo {
            font-size: 1.5rem;
            font-weight: 800;
            background: linear-gradient(to right, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            letter-spacing: -1px;
        }

        .card {
            background: var(--glass);
            padding: 3rem;
            border-radius: 32px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: all 0.4s ease;
            backdrop-filter: blur(12px);
            text-align: center;
            width: 90%;
            max-width: 400px;
            box-shadow: 0 20px 50px rgba(0,0,0,0.5);
        }

        .card:hover {
            border-color: var(--primary);
            transform: translateY(-5px);
        }

        h1 {
            font-size: 2.5rem;
            margin-bottom: 2rem;
            font-weight: 800;
        }

        .btn-toggle {
            width: 120px;
            height: 120px;
            border-radius: 50%;
            border: 4px solid var(--glass);
            background: var(--glass);
            cursor: pointer;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0 auto;
            transition: all 0.5s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            position: relative;
        }

        .btn-toggle.active {
            background: var(--primary);
            box-shadow: 0 0 50px rgba(0, 242, 254, 0.4);
            border-color: white;
            color: #000;
        }

        .btn-toggle i {
            font-size: 3rem;
        }

        .status-label {
            margin-top: 1.5rem;
            font-size: 1.2rem;
            font-weight: 600;
            color: #94a3b8;
        }

        .active-text {
            color: var(--primary);
        }

        #bg-canvas {
            position: absolute;
            top: 0;
            left: 0;
            z-index: -1;
        }

        .status-badge {
            background: rgba(34, 197, 94, 0.1);
            color: #22c55e;
            padding: 0.5rem 1rem;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
        }
    </style>
</head>
<body>
    <canvas id="bg-canvas"></canvas>
    
    <header class="header">
        <div class="logo">TECOTRACK</div>
        <div class="status-badge" id="uptime">Uptime: 0s</div>
    </header>

    <div class="card">
        <h1>LED Control</h1>
        <button id="ledBtn" class="btn-toggle" onclick="toggleLED()">
            <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M18.36 6.64a9 9 0 1 1-12.73 0"></path>
                <line x1="12" y1="2" x2="12" y2="12"></line>
            </svg>
        </button>
        <div class="status-label">GPIO2: <span id="statusTxt">OFF</span></div>
    </div>

    <script>
        let ledState = false;

        async function updateStatus() {
            try {
                const res = await fetch('/status');
                const data = await res.json();
                ledState = data.state === 1;
                refreshUI();
            } catch (e) { console.error(e); }
        }

        async function toggleLED() {
            const newState = ledState ? 0 : 1;
            try {
                const res = await fetch(`/toggle?state=${newState}`, { method: 'POST' });
                const data = await res.json();
                ledState = data.state === 1;
                refreshUI();
            } catch (e) { console.error(e); }
        }

        function refreshUI() {
            const btn = document.getElementById('ledBtn');
            const txt = document.getElementById('statusTxt');
            if (ledState) {
                btn.classList.add('active');
                txt.innerText = 'ON';
                txt.classList.add('active-text');
            } else {
                btn.classList.remove('active');
                txt.innerText = 'OFF';
                txt.classList.remove('active-text');
            }
        }

        // Uptime counter
        let startTime = Date.now();
        setInterval(() => {
            const secs = Math.floor((Date.now() - startTime) / 1000);
            document.getElementById('uptime').innerText = `Uptime: ${secs}s`;
        }, 1000);

        // Animated particles
        const canvas = document.getElementById('bg-canvas');
        const ctx = canvas.getContext('2d');
        let w, h, particles = [];

        function resize() {
            w = canvas.width = window.innerWidth;
            h = canvas.height = window.innerHeight;
        }

        window.addEventListener('resize', resize);
        resize();

        for(let i=0; i<40; i++) {
            particles.push({
                x: Math.random()*w, y: Math.random()*h,
                vx: (Math.random()-0.5)*0.5, vy: (Math.random()-0.5)*0.5,
                s: Math.random()*2
            });
        }

        function draw() {
            ctx.clearRect(0,0,w,h);
            ctx.fillStyle = 'rgba(0, 242, 254, 0.2)';
            particles.forEach(p => {
                p.x += p.vx; p.y += p.vy;
                if(p.x<0) p.x=w; if(p.x>w) p.x=0;
                if(p.y<0) p.y=h; if(p.y>h) p.y=0;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.s, 0, Math.PI*2);
                ctx.fill();
            });
            requestAnimationFrame(draw);
        }
        draw();

        // Initial checks
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
)=====";

#endif
