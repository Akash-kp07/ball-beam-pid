# Ball and Beam PID Control System

A closed-loop ball balancing system using an ultrasonic sensor, servo motor, and a PID controller with Kalman filtering — built on Arduino.

## Demo


https://github.com/user-attachments/assets/a7be61a5-dee0-4388-ae0d-bb8cbe6352cf


📁 See `/Media/Photos` and `/Media/Videos` for system footage.

---

## How It Works

An ultrasonic sensor measures the ball's position on the beam. A PID controller computes the correction needed and drives a servo motor to tilt the beam, keeping the ball at a target position.

```
[Ultrasonic Sensor] → [Kalman Filter] → [PID Controller] → [Servo Motor] → [Beam]
         ↑_______________________________________________________|
```

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | Arduino (Uno/Nano) |
| Sensor | HC-SR04 Ultrasonic |
| Actuator | Servo Motor |
| Beam range | 2.5 cm – 25 cm |

### Pin Connections

| Pin | Function |
|-----|----------|
| D8  | Servo signal |
| D9  | Ultrasonic TRIG |
| D10 | Ultrasonic ECHO |

---

## Software Features

- **PID Control** — Proportional, Integral, Derivative with dynamic gain scaling based on target distance
- **Kalman Filter** — Smooths noisy ultrasonic readings
- **Deadband** — Eliminates servo jitter near setpoint (7% of target)
- **Derivative filter** — Low-pass filter on derivative term (α = 0.3) to reduce noise amplification
- **Slew limiter** — Limits servo angular velocity based on error magnitude (prevents overshoot)
- **Integral windup clamp** — Integral term clamped to ±20
- **Serial setpoint control** — Target position adjustable live via Serial (5–22 cm range)

---

## PID Gains

| Gain | Base Value |
|------|-----------|
| Kp   | 1.9 |
| Ki   | 0.3 |
| Kd   | 1.2 |

Gains are scaled dynamically: `K_scaled = K_base × (target / 15.0)`

---

## Serial Interface

Send a float value (5.0 – 22.0) over Serial at **9600 baud** to update the target position:

```
15.0   → sets target to 15 cm
10.0   → sets target to 10 cm
```

Serial output format (for real-time plotting):
```
pos:14.87,target:15.00,servo:46
```

---

## Code Structure

```
Code/
└── sketch_apr16a.ino   # Main Arduino sketch
Media/
├── Photos/             # System photos
└── Videos/             # Demo videos
```

---

## Getting Started

1. Wire hardware per pin table above
2. Open `Code/sketch_apr16a.ino` in Arduino IDE
3. Upload to Arduino
4. Open Serial Monitor at **9600 baud**
5. Send target distance (e.g. `15.0`) to begin

---

## Author

**Akash KP** — Embedded Systems & Robotics Developer  
[GitHub](https://github.com/Akash-kp07)
