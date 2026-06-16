from flask import Flask, render_template_string, request
import serial
import time
import threading

# 🔧 CHANGE THIS PORT
SERIAL_PORT = 'COM7'   # Windows → COM3, Linux → /dev/ttyUSB0

ser = serial.Serial(SERIAL_PORT, 9600, timeout=1)
time.sleep(2)

app = Flask(__name__)

# ── Global variable ──────────────────────────────────
current_position = 0

# ── Serial Reader Thread ─────────────────────────────
def read_serial():
    global current_position

    while True:
        try:
            line = ser.readline().decode().strip()

            if "pos:" in line:
                parts = line.split(",")

                for p in parts:
                    if "pos:" in p:
                        value = float(p.replace("pos:", ""))
                        current_position = round(value)   # ✅ rounding

        except:
            pass

# ── HTML UI ──────────────────────────────────────────
HTML = """
<!DOCTYPE html>
<html>
<head>
    <title>Distance Control</title>
    <style>
        body {
            background: #0f172a;
            color: #e2e8f0;
            text-align: center;
            font-family: Arial;
        }

        h1 {
            margin-top: 50px;
        }

        .slider {
            width: 300px;
        }

        .box {
            margin-top: 40px;
        }
    </style>
</head>
<body>

<h1>Target Distance Controller</h1>

<div class="box">
    <h2>Target: <span id="target">15</span> cm</h2>
    <h2>Current: <span id="current">0</span> cm</h2>

    <input type="range" min="5" max="22" value="15" class="slider" id="slider">
</div>

<script>
let slider = document.getElementById("slider");
let target = document.getElementById("target");
let current = document.getElementById("current");

// Send target value
slider.oninput = function() {
    target.innerHTML = this.value;
    fetch("/set?value=" + this.value);
}

// Get current position every 200ms
setInterval(() => {
    fetch("/position")
    .then(res => res.text())
    .then(data => {
        current.innerHTML = data;
    });
}, 200);

</script>

</body>
</html>
"""

# ── Routes ───────────────────────────────────────────
@app.route('/')
def index():
    return render_template_string(HTML)

@app.route('/set')
def set_value():
    value = request.args.get('value')

    if value:
        ser.write((value + '\\n').encode())
        print("Sent:", value)

    return "OK"

@app.route('/position')
def get_position():
    return str(current_position)

# ── Start everything ─────────────────────────────────
if __name__ == '__main__':
    threading.Thread(target=read_serial, daemon=True).start()
    app.run(host='0.0.0.0', port=5000)