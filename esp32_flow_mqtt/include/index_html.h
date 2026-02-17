#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="theme-color" content="#0f172a">
    <title>Tecotrack | Flow Control</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary: #00f2fe;
            --secondary: #4facfe;
            --dark: #0f172a;
            --glass: rgba(255, 255, 255, 0.05);
            --text: #f8fafc;
            --danger: #ef4444;
            --success: #22c55e;
            --warning: #f59e0b;
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
            overflow-x: hidden;
            background: radial-gradient(circle at top right, #1e293b, #0f172a);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
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
            font-size: 1.8rem;
            font-weight: 800;
            background: linear-gradient(to right, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            letter-spacing: -1px;
        }

        .container {
            margin-top: 100px;
            width: 90%;
            max-width: 1000px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 2rem;
            padding: 2rem;
        }

        .card {
            background: var(--glass);
            padding: 2rem;
            border-radius: 24px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: all 0.4s ease;
            backdrop-filter: blur(12px);
            text-align: center;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }

        .card:hover {
            border-color: var(--primary);
            transform: translateY(-5px);
        }

        .progress-container {
            width: 100%;
            height: 10px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 5px;
            margin-top: 1.5rem;
            overflow: hidden;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }

        .progress-bar {
            width: 0%;
            height: 100%;
            background: linear-gradient(to right, var(--primary), var(--secondary));
            border-radius: 5px;
            transition: width 0.4s ease, background 0.4s ease;
            box-shadow: 0 0 10px rgba(0, 242, 254, 0.3);
        }

        .gauge-container {
            position: relative;
            width: 200px;
            height: 200px;
            margin: 0 auto;
        }

        .gauge-svg {
            width: 100%;
            height: 100%;
            transform: rotate(-90deg);
        }

        .gauge-bg {
            fill: none;
            stroke: rgba(255, 255, 255, 0.1);
            stroke-width: 10;
        }

        .gauge-bar {
            fill: none;
            stroke: var(--primary);
            stroke-width: 10;
            stroke-dasharray: 440;
            stroke-dashoffset: 440;
            transition: stroke-dashoffset 0.5s ease;
            stroke-linecap: round;
        }

        .gauge-value {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-size: 2.5rem;
            font-weight: 800;
        }

        .gauge-label {
            display: block;
            margin-top: 1rem;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 2px;
            color: #94a3b8;
        }

        .controls {
            display: flex;
            flex-direction: column;
            gap: 1rem;
        }

        .btn {
            padding: 1rem;
            border-radius: 12px;
            border: none;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .btn-toggle {
            background: var(--glass);
            color: white;
            border: 1px solid rgba(255, 255, 255, 0.2);
        }

        .btn-toggle.active {
            background: var(--success);
            color: black;
            box-shadow: 0 0 20px rgba(34, 197, 94, 0.4);
        }

        .btn-danger {
            background: rgba(239, 68, 68, 0.1);
            color: var(--danger);
            border: 1px solid rgba(239, 68, 68, 0.2);
        }

        .btn-primary {
            background: linear-gradient(to right, var(--primary), var(--secondary));
            color: black;
        }

        .input-group {
            display: flex;
            gap: 0.5rem;
            margin-top: 1rem;
        }

        .btn-group {
            display: flex;
            gap: 0.8rem;
            width: 100%;
        }

        .btn-group .btn {
            flex: 1;
            padding: 0.8rem;
            font-size: 0.85rem;
        }

        input {
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 8px;
            padding: 0.5rem;
            color: white;
            flex: 1;
            text-align: center;
        }

        .status-badge {
            background: rgba(239, 68, 68, 0.1);
            color: var(--danger);
            padding: 0.4rem 0.8rem;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
        }

        .status-badge.connected {
            background: rgba(34, 197, 94, 0.1);
            color: var(--success);
        }

        #bg-canvas {
            position: absolute;
            top: 0;
            left: 0;
            z-index: -1;
        }

        /* Responsive Breakpoints */
        @media (max-width: 768px) {
            .header {
                padding: 1rem;
            }
            .logo {
                font-size: 1.4rem;
            }
            .container {
                margin-top: 80px;
                padding: 1rem;
                gap: 1.5rem;
                grid-template-columns: 1fr;
            }
            .card {
                padding: 1.5rem;
            }
            .gauge-container {
                width: 180px;
                height: 180px;
            }
            .gauge-value {
                font-size: 2rem;
            }
            .btn {
                padding: 0.8rem;
                font-size: 0.9rem;
            }
            .btn-group .btn {
                padding: 0.7rem;
                font-size: 0.8rem;
            }
        }

        @media (max-width: 480px) {
            .logo {
                font-size: 1.2rem;
            }
            .status-badge {
                padding: 0.3rem 0.6rem;
                font-size: 0.7rem;
            }
            .gauge-container {
                width: 160px;
                height: 160px;
            }
            .gauge-value {
                font-size: 1.8rem;
            }
            input {
                font-size: 0.9rem;
                padding: 0.4rem;
            }
        }
    </style>
</head>
<body>
    <canvas id="bg-canvas"></canvas>
    
    <header class="header">
        <div class="logo">TECOTRACK</div>
        <div class="status-badge" id="ws-status">Disconnected</div>
    </header>

    <div class="container">
        <!-- Flow Card -->
        <div class="card">
            <div class="gauge-container">
                <svg class="gauge-svg" viewBox="0 0 160 160">
                    <circle class="gauge-bg" cx="80" cy="80" r="70"></circle>
                    <circle id="flow-bar" class="gauge-bar" cx="80" cy="80" r="70" style="stroke: #f59e0b;"></circle>
                </svg>
                <div class="gauge-value" id="flow-val">0.0</div>
            </div>
            <span class="gauge-label">Flow Rate (L/min)</span>
        </div>

        <!-- Volume Card -->
        <div class="card">
            <h3 style="margin-bottom: 1rem; color: var(--primary);">Batch Control</h3>
            <div style="font-size: 3rem; font-weight: 800; margin: 1rem 0;" id="vol-val">0.00</div>
            <div style="color: #94a3b8; margin-bottom: 1rem;">Liters / <span id="vol-target-display">1000</span> Target</div>
            
            <div class="controls">
                <div class="input-group">
                    <input type="number" id="vol-input" placeholder="Set Target">
                    <button class="btn btn-primary" onclick="setTarget()" style="padding: 0.5rem 1rem;">Set</button>
                </div>
                <div class="btn-group">
                    <button id="main-btn" class="btn btn-toggle" onclick="togglePin(13)">Start Batch</button>
                    <button class="btn btn-danger" onclick="resetBatch()">Reset Volume</button>
                </div>
            </div>
            <div class="progress-container">
                <div id="batch-progress" class="progress-bar"></div>
            </div>
        </div>

        <!-- System Card -->
        <div class="card">
            <h3 style="margin-bottom: 1.5rem; color: var(--primary);">System Status</h3>
            <div style="text-align: left; display: flex; flex-direction: column; gap: 0.8rem;">
                <div>Uptime: <span id="uptime">0s</span></div>
                <div>Session Time: <span id="session-time">00:00:00</span></div>
                <div>Status: <span id="batch-status" style="font-weight: 700; color: #94a3b8;">Stopped</span></div>
                <div style="margin-top: 1rem;">
                    <button id="valve-btn" class="btn btn-toggle" style="width: 100%;" onclick="togglePin(16)">VALVE (GPIO 16)</button>
                </div>
            </div>
        </div>
    </div>

    <script>
        let socket;
        let start = Date.now();

        function connectWS() {
            socket = new WebSocket(`ws://${window.location.hostname}:81`);
            
            socket.onopen = () => {
                const status = document.getElementById('ws-status');
                status.innerText = "Connected";
                status.classList.add('connected');
            };

            socket.onclose = () => {
                const status = document.getElementById('ws-status');
                status.innerText = "Disconnected";
                status.classList.remove('connected');
                setTimeout(connectWS, 2000);
            };

            socket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                
                if (data.type === 'flow') {
                    const val = data.val.toFixed(1);
                    document.getElementById('flow-val').innerText = val;
                    const offset = 440 - (440 * Math.min(val, 100) / 100);
                    document.getElementById('flow-bar').style.strokeDashoffset = offset;
                } 
                else if (data.type === 'volumeUpdate') {
                    document.getElementById('vol-val').innerText = data.vol.toFixed(2);
                    document.getElementById('vol-target-display').innerText = Math.round(data.target);
                    
                    const s = data.elapsed;
                    const h = Math.floor(s / 3600).toString().padStart(2, '0');
                    const m = Math.floor((s % 3600) / 60).toString().padStart(2, '0');
                    const sec = (s % 60).toString().padStart(2, '0');
                    document.getElementById('session-time').innerText = `${h}:${m}:${sec}`;
                    
                    const statusEl = document.getElementById('batch-status');
                    const mainBtn = document.getElementById('main-btn');
                    const volInput = document.getElementById('vol-input');
                    const resetBtn = document.querySelector('.btn-danger');

                    // Update Relay UI state
                    if (data.relayActive) {
                        mainBtn.classList.add('active');
                        mainBtn.innerText = "Pause Batch";
                        volInput.disabled = true;
                        volInput.style.opacity = "0.5";
                        resetBtn.disabled = true;
                        resetBtn.style.opacity = "0.5";
                    } else {
                        mainBtn.classList.remove('active');
                        mainBtn.innerText = "Start Batch";
                        volInput.disabled = false;
                        volInput.style.opacity = "1";
                        resetBtn.disabled = false;
                        resetBtn.style.opacity = "1";
                    }

                    // Update Valve UI state
                    const valveBtn = document.getElementById('valve-btn');
                    if (data.valveActive) valveBtn.classList.add('active');
                    else valveBtn.classList.remove('active');

                    if (data.targetReached) {
                        statusEl.innerText = "COMPLETED";
                        statusEl.style.color = "var(--danger)";
                        document.getElementById('batch-progress').style.background = "var(--danger)";
                    } else if (data.relayActive) {
                        statusEl.innerText = "RUNNING";
                        statusEl.style.color = "var(--success)";
                        document.getElementById('batch-progress').style.background = "linear-gradient(to right, var(--primary), var(--secondary))";
                    } else {
                        statusEl.innerText = "Stopped";
                        statusEl.style.color = "var(--warning)";
                        document.getElementById('batch-progress').style.background = "var(--warning)";
                    }

                    const pct = Math.min((data.vol / data.target) * 100, 100);
                    document.getElementById('batch-progress').style.width = `${pct}%`;

                    // Update Device Uptime
                    const upSec = data.uptime;
                    const upH = Math.floor(upSec / 3600);
                    const upM = Math.floor((upSec % 3600) / 60);
                    const upS = upSec % 60;
                    let upStr = "";
                    if (upH > 0) upStr += upH + "h ";
                    if (upM > 0 || upH > 0) upStr += upM + "m ";
                    upStr += upS + "s";
                    document.getElementById('uptime').innerText = upStr;
                }
            };
        }

        function togglePin(pin) {
            if (socket.readyState === WebSocket.OPEN) socket.send(`toggle:${pin}`);
        }

        function setTarget() {
            const val = parseFloat(document.getElementById('vol-input').value);
            const current = parseFloat(document.getElementById('vol-val').innerText);
            
            if (!val || val <= current) {
                alert(`New target must be greater than current volume (${current.toFixed(2)} L)`);
                return;
            }

            if (socket.readyState === WebSocket.OPEN) socket.send(`setTarget:${val}`);
        }

        function resetBatch() {
            if (confirm("Reset current batch data?") && socket.readyState === WebSocket.OPEN) {
                socket.send("resetBatch");
            }
        }

        // Background particles
        const canvas = document.getElementById('bg-canvas');
        const ctx = canvas.getContext('2d');
        let w, h, particles = [];

        function resize() {
            w = canvas.width = window.innerWidth;
            h = canvas.height = window.innerHeight;
        }

        window.addEventListener('resize', resize);
        resize();

        for(let i=0; i<50; i++) {
            particles.push({
                x: Math.random()*w, y: Math.random()*h,
                vx: (Math.random()-0.5)*0.5, vy: (Math.random()-0.5)*0.5,
                s: Math.random()*2
            });
        }

        function draw() {
            ctx.clearRect(0,0,w,h);
            ctx.fillStyle = 'rgba(0, 242, 254, 0.15)';
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

        connectWS();
    </script>
</body>
</html>
)=====";

#endif
