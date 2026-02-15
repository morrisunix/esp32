#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tecotrack | Industrial Automation Solutions</title>
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
            overflow-x: hidden;
            background: radial-gradient(circle at top right, #1e293b, #0f172a);
            min-height: 100vh;
        }

        .header {
            padding: 2rem;
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

        .hero {
            height: 100vh;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            text-align: center;
            padding: 0 1rem;
            position: relative;
        }

        .hero h1 {
            font-size: 4rem;
            line-height: 1.1;
            margin-bottom: 1.5rem;
            animation: fadeInUp 1s ease-out;
        }

        .hero p {
            font-size: 1.25rem;
            color: #94a3b8;
            max-width: 600px;
            margin-bottom: 2.5rem;
            animation: fadeInUp 1s ease-out 0.2s backwards;
        }

        .btn {
            padding: 1rem 2.5rem;
            border-radius: 50px;
            text-decoration: none;
            font-weight: 600;
            transition: all 0.3s ease;
            cursor: pointer;
            border: none;
        }

        .btn-primary {
            background: linear-gradient(to right, var(--primary), var(--secondary));
            color: #000;
            box-shadow: 0 10px 20px rgba(0, 242, 254, 0.2);
        }

        .btn-primary:hover {
            transform: translateY(-3px);
            box-shadow: 0 15px 30px rgba(0, 242, 254, 0.4);
        }

        .features {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 2rem;
            padding: 4rem 2rem;
            max-width: 1200px;
            margin: 0 auto;
        }

        .card {
            background: var(--glass);
            padding: 2.5rem;
            border-radius: 24px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: all 0.4s ease;
            backdrop-filter: blur(5px);
        }

        .card:hover {
            transform: translateY(-10px);
            background: rgba(255, 255, 255, 0.08);
            border-color: var(--primary);
        }

        .card h3 {
            font-size: 1.5rem;
            margin-bottom: 1rem;
            color: var(--primary);
        }

        .card p {
            color: #94a3b8;
            line-height: 1.6;
        }

        .status-badge {
            background: rgba(34, 197, 94, 0.1);
            color: #22c55e;
            padding: 0.5rem 1rem;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
            margin-bottom: 1rem;
            display: inline-block;
        }

        @keyframes fadeInUp {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }

        #bg-canvas {
            position: absolute;
            top: 0;
            left: 0;
            z-index: -1;
        }

        .footer {
            text-align: center;
            padding: 4rem 2rem;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            color: #475569;
        }

        .gauges-container {
            display: flex;
            justify-content: center;
            gap: 2rem;
            flex-wrap: wrap;
            padding: 2rem;
            margin-top: 2rem;
        }

        .gauge-card {
            background: var(--glass);
            padding: 2rem;
            border-radius: 24px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            text-align: center;
            width: 200px;
            backdrop-filter: blur(10px);
        }

        .gauge-svg {
            width: 150px;
            height: 150px;
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
            font-size: 2rem;
            font-weight: 800;
            margin-top: -95px;
            margin-bottom: 60px;
            display: block;
        }

        .gauge-label {
            color: #94a3b8;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .btn-toggle {
            background: rgba(255, 255, 255, 0.1);
            color: white;
            border: 1px solid rgba(255, 255, 255, 0.2);
            margin-top: 1rem;
        }

        .btn-toggle.active {
            background: #22c55e;
            color: black;
            box-shadow: 0 0 20px rgba(34, 197, 94, 0.4);
        }
    </style>
</head>
<body>
    <canvas id="bg-canvas"></canvas>
    
    <header class="header">
        <div class="logo">TECOTRACK</div>
        <div style="display: flex; gap: 0.5rem;">
            <div class="status-badge" id="ws-status" style="background: rgba(239, 68, 68, 0.1); color: #ef4444;">Connecting...</div>
            <div class="status-badge" id="uptime">System Online: 0s</div>
        </div>
    </header>

    <section class="hero">
        <h1>Flow<br><span id="dynamic-text">Control</span></h1>
        <p>Precision engineering, PLC integration, and real-time dashboard monitoring for the next generation of industry.</p>
        <div style="display: flex; flex-direction: column; align-items: center; gap: 1rem; width: 100%; max-width: 400px;">
            <div style="display: grid; grid-template-columns: 1fr; gap: 0.5rem; width: 100%;">
                <button id="btn-13" class="btn btn-toggle" style="margin-top: 0;" onclick="togglePin(13)">RELAY (GPIO 13)</button>
                <button id="btn-16" class="btn btn-toggle" style="margin-top: 0;" onclick="togglePin(16)">VALVE (GPIO 16)</button>
            </div>
        </div>
    </section>

    <div class="features">
        <div class="card">
            <h3>Real-time Sensors</h3>
            <div class="gauges-container">
                <div class="gauge-card">
                    <svg class="gauge-svg" viewBox="0 0 160 160">
                        <circle class="gauge-bg" cx="80" cy="80" r="70"></circle>
                        <circle id="temp-bar" class="gauge-bar" cx="80" cy="80" r="70"></circle>
                    </svg>
                    <span id="temp-val" class="gauge-value">--°C</span>
                    <span class="gauge-label">Temperature</span>
                </div>
                <div class="gauge-card">
                    <svg class="gauge-svg" viewBox="0 0 160 160">
                        <circle class="gauge-bg" cx="80" cy="80" r="70"></circle>
                        <circle id="hum-bar" class="gauge-bar" cx="80" cy="80" r="70" style="stroke: #a78bfa;"></circle>
                    </svg>
                    <span id="hum-val" class="gauge-value">--%</span>
                    <span class="gauge-label">Humidity</span>
                </div>
                <!-- Distance Display -->
                <div class="gauge-card" style="width: 250px;">
                    <div style="font-size: 3rem; font-weight: 800; color: var(--primary);" id="dist-val">--</div>
                    <div style="font-size: 1.2rem; font-weight: 600; margin-bottom: 0.5rem;">CM</div>
                    <span class="gauge-label">Obstacle Distance</span>
                    <div style="width: 100%; height: 8px; background: rgba(255,255,255,0.1); border-radius: 4px; margin-top: 1rem; overflow: hidden;">
                        <div id="dist-bar" style="width: 0%; height: 100%; background: var(--primary); transition: width 0.3s ease;"></div>
                    </div>
                </div>
                <!-- Flow Rate Display -->
                <div class="gauge-card" style="width: 250px; border-color: #f59e0b;">
                    <div style="font-size: 3rem; font-weight: 800; color: #f59e0b;" id="flow-val">--</div>
                    <div style="font-size: 1.2rem; font-weight: 600; margin-bottom: 0.5rem;">L/MIN</div>
                    <span class="gauge-label">Active Flow Rate</span>
                    <div style="width: 100%; height: 8px; background: rgba(255,255,255,0.1); border-radius: 4px; margin-top: 1rem; overflow: hidden;">
                        <div id="flow-bar" style="width: 0%; height: 100%; background: #f59e0b; transition: width 0.3s ease;"></div>
                    </div>
                </div>
                <!-- Volume Accumulator -->
                <div class="gauge-card" style="width: 280px; border-color: #22c55e;">
                    <div style="font-size: 2.5rem; font-weight: 800; color: #22c55e;" id="vol-val">0.00</div>
                    <div style="font-size: 1rem; font-weight: 600; margin-bottom: 0.2rem;">/ <span id="vol-target-display">1000</span> LITERS</div>
                    <div style="font-size: 1.2rem; font-weight: 800; font-family: monospace; letter-spacing: 2px; color: var(--primary); margin: 0.5rem 0;" id="elapsed-time">00:00:00</div>
                    <div id="batch-status" class="status-badge" style="background: rgba(148, 163, 184, 0.1); color: #94a3b8; font-size: 0.7rem;">IDLE</div>
                    
                    <div style="margin: 1rem 0; display: flex; flex-direction: column; gap: 0.5rem;">
                        <div style="display: flex; gap: 0.5rem; justify-content: center;">
                            <input type="number" id="volume-input" min="1" max="1000000" placeholder="Set Target" 
                                   style="width: 100px; padding: 5px; border-radius: 4px; border: 1px solid rgba(255,255,255,0.2); background: rgba(0,0,0,0.3); color: white; text-align: center;">
                            <button onclick="setVolumeTarget()" 
                                    style="padding: 5px 10px; border-radius: 4px; border: none; background: var(--secondary); color: white; font-weight: 600; cursor: pointer;">SET</button>
                        </div>
                        <div style="display: flex; gap: 0.5rem; justify-content: center;">
                            <button id="pause-btn" onclick="togglePin(13)" 
                                    style="flex: 1; padding: 10px; border-radius: 8px; border: none; background: var(--primary); color: black; font-weight: 800; cursor: pointer;">START / PAUSE</button>
                            <button onclick="resetBatch()" 
                                    style="padding: 10px; border-radius: 8px; border: 1px solid rgba(239, 68, 68, 0.5); background: transparent; color: #ef4444; font-weight: 600; cursor: pointer;">CLEAR</button>
                        </div>
                    </div>

                    <span class="gauge-label">Batch Accumulator</span>
                    <div style="width: 100%; height: 8px; background: rgba(255,255,255,0.1); border-radius: 4px; margin-top: 0.8rem; overflow: hidden;">
                        <div id="vol-bar" style="width: 0%; height: 100%; background: #22c55e; transition: width 0.5s ease;"></div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <footer class="footer">
        &copy; 2026 Tecotrack Automation. Powered by ESP8266 & Antigravity.
    </footer>

    <script>
        // Dynamic uptime counter
        let start = Date.now();
        setInterval(() => {
            let seconds = Math.floor((Date.now() - start) / 1000);
            document.getElementById('uptime').innerText = `System Online: ${seconds}s`;
        }, 1000);

        // Simple Background Animation
        const canvas = document.getElementById('bg-canvas');
        const ctx = canvas.getContext('2d');
        let width, height;

        function resize() {
            width = canvas.width = window.innerWidth;
            height = canvas.height = window.innerHeight;
        }

        window.addEventListener('resize', resize);
        resize();

        const particles = [];
        for(let i = 0; i < 50; i++) {
            particles.push({
                x: Math.random() * width,
                y: Math.random() * height,
                vx: (Math.random() - 0.5) * 0.5,
                vy: (Math.random() - 0.5) * 0.5,
                size: Math.random() * 2
            });
        }

        function animate() {
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = 'rgba(0, 242, 254, 0.15)';
            
            particles.forEach(p => {
                p.x += p.vx;
                p.y += p.vy;

                if(p.x < 0) p.x = width;
                if(p.x > width) p.x = 0;
                if(p.y < 0) p.y = height;
                if(p.y > height) p.y = 0;

                ctx.beginPath();
                ctx.arc(p.x, p.y, p.size, 0, Math.PI * 2);
                ctx.fill();
            });
            requestAnimationFrame(animate);
        }
        animate();

        // Text animation
        const words = ["Automation", "Integration", "Efficiency", "Monitoring"];
        let wordIdx = 0;
        setInterval(() => {
            wordIdx = (wordIdx + 1) % words.length;
            const el = document.getElementById('dynamic-text');
            el.style.opacity = 0;
            setTimeout(() => {
                el.innerText = words[wordIdx];
                el.style.opacity = 1;
            }, 500);
        }, 3000);
        // WebSocket Logic
        let socket;
        function connectWS() {
            // Use the same hostname as the web server, but port 81 for WebSocket
            const wsPort = 81;
            socket = new WebSocket(`ws://${window.location.hostname}:${wsPort}`);
            
            socket.onopen = () => {
                const status = document.getElementById('ws-status');
                status.innerText = "Live Connected";
                status.style.background = "rgba(34, 197, 94, 0.1)";
                status.style.color = "#22c55e";
                console.log("WS Connected");
            };

            socket.onclose = () => {
                const status = document.getElementById('ws-status');
                status.innerText = "Disconnected";
                status.style.background = "rgba(239, 68, 68, 0.1)";
                status.style.color = "#ef4444";
                setTimeout(connectWS, 2000);
            };

            socket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                
                if (data.type === 'sensor') {
                    // Update Temperature
                    const tempVal = Math.round(data.temp);
                    document.getElementById('temp-val').innerText = `${tempVal}°C`;
                    const tempOffset = 440 - (440 * tempVal / 100);
                    document.getElementById('temp-bar').style.strokeDashoffset = tempOffset;

                    // Update Humidity
                    const humVal = Math.round(data.hum);
                    document.getElementById('hum-val').innerText = `${humVal}%`;
                    const humOffset = 440 - (440 * humVal / 100);
                    document.getElementById('hum-bar').style.strokeDashoffset = humOffset;
                } else if (data.type === 'distance') {
                    // Update Distance
                    const dist = data.val.toFixed(1);
                    document.getElementById('dist-val').innerText = dist;
                    // Map 0-100cm to 0-100%
                    let pct = Math.min(100, Math.max(0, data.val));
                    document.getElementById('dist-bar').style.width = `${pct}%`;
                } else if (data.type === 'flow') {
                    // Update Flow
                    const flow = data.val.toFixed(1);
                    document.getElementById('flow-val').innerText = flow;
                    // Map 0-100 L/min to 0-100%
                    let pct = Math.min(100, Math.max(0, data.val));
                    document.getElementById('flow-bar').style.width = `${pct}%`;
                } else if (data.type === 'volumeUpdate') {
                    // Update Volume and Time
                    document.getElementById('vol-val').innerText = data.vol.toFixed(2);
                    document.getElementById('vol-target-display').innerText = Math.round(data.target);
                    
                    const s = data.elapsed;
                    const h = Math.floor(s / 3600).toString().padStart(2, '0');
                    const m = Math.floor((s % 3600) / 60).toString().padStart(2, '0');
                    const sec = (s % 60).toString().padStart(2, '0');
                    document.getElementById('elapsed-time').innerText = `${h}:${m}:${sec}`;
                    
                    const pct = (data.vol / data.target) * 100;
                    document.getElementById('vol-bar').style.width = `${pct}%`;
                    
                    if (data.targetReached) {
                       document.getElementById('vol-val').style.color = "#ef4444";
                       document.getElementById('vol-bar').style.background = "#ef4444";
                       document.getElementById('batch-status').innerText = "COMPLETED";
                       document.getElementById('batch-status').style.color = "#ef4444";
                       document.getElementById('batch-status').style.background = "rgba(239, 68, 68, 0.1)";
                    } else {
                       document.getElementById('vol-val').style.color = "#22c55e";
                       document.getElementById('vol-bar').style.background = "#22c55e";
                    }
                } else if (data.type === 'pinState') {
                    const btn = document.getElementById(`btn-${data.pin}`);
                    if (btn) {
                        if (data.state) {
                            btn.classList.add('active');
                        } else {
                            btn.classList.remove('active');
                        }
                    }
                    if (data.pin === 13) {
                        const pauseBtn = document.getElementById('pause-btn');
                        const status = document.getElementById('batch-status');
                        if (data.state) {
                            pauseBtn.innerText = "PAUSE";
                            pauseBtn.style.background = "#f59e0b"; // Orange for Pause
                            status.innerText = "RUNNING";
                            status.style.color = "#22c55e";
                            status.style.background = "rgba(34, 197, 94, 0.1)";
                        } else {
                            pauseBtn.innerText = "RUN / RESUME";
                            pauseBtn.style.background = "var(--primary)";
                            if (status.innerText !== "COMPLETED") {
                                status.innerText = "PAUSED";
                                status.style.color = "#f59e0b";
                                status.style.background = "rgba(245, 158, 11, 0.1)";
                            }
                        }
                    }
                }
            };
        }

        function togglePin(pin) {
            if (socket.readyState === WebSocket.OPEN) {
                socket.send(`toggle:${pin}`);
            }
        }

        function setVolumeTarget() {
            const input = document.getElementById('volume-input');
            const val = parseFloat(input.value);
            if (val >= 1 && val <= 1000000) {
                if (socket.readyState === WebSocket.OPEN) {
                    socket.send(`setTarget:${val}`);
                }
            } else {
                alert("Please enter a value between 1 and 1,000,000");
            }
        }

        function resetBatch() {
            if (confirm("Reset current batch volume and timer?")) {
                if (socket.readyState === WebSocket.OPEN) {
                    socket.send("resetBatch");
                }
            }
        }

        connectWS();
    </script>
</body>
</html>
)=====";

#endif
