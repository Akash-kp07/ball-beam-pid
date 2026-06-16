#include <Servo.h>

// ── Pins ─────────────────────────────────────────────
const int SERVO_PIN = 8;
const int TRIG_PIN  = 9;
const int ECHO_PIN  = 10;

// ── Beam geometry ────────────────────────────────────
float TARGET_DIST = 15.0;   // default
const float DIST_MIN = 2.5;
const float DIST_MAX = 25.0;

// ── Servo limits ─────────────────────────────────────
const int SERVO_CENTER  = 44;
const int SERVO_MIN_DEG = 20;
const int SERVO_MAX_DEG = 100;

// ── Kalman Filter ────────────────────────────────────
struct KalmanFilter {
  float q, r, p, x, k;

  KalmanFilter(float meas_err, float est_err, float proc_var)
    : q(proc_var), r(meas_err), p(est_err), x(0), k(0) {}

  float update(float z) {
    p = p + q;
    k = p / (p + r);
    x = x + k * (z - x);
    p = (1 - k) * p;
    return x;
  }
};

KalmanFilter kf(0.4, 0.4, 0.05);

// ── PID gains ────────────────────────────────────────
float KP_BASE = 1.9;
float KI_BASE = 0.3;
float KD_BASE = 1.2;

// ── PID variables ────────────────────────────────────
float pid_integral = 0.0;
float pid_prev_err = 0.0;
float filtered_deriv = 0.0;

// ── Globals ──────────────────────────────────────────
Servo servo;
float ball_position = 0;

int prev_servo_angle = SERVO_CENTER;
unsigned long prev_time = 0;

// ── Ultrasonic sensor ────────────────────────────────
float readRawDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1;

  return dur * 0.0343 / 2.0;
}

bool readSensor() {
  float raw = readRawDistance();
  if (raw < DIST_MIN || raw > DIST_MAX) return false;

  ball_position = kf.update(raw);
  return true;
}

// ── PID controller ───────────────────────────────────
float computePID(float dt) {
  float error = ball_position - TARGET_DIST;

  float deadband = 0.07 * TARGET_DIST;
  if (abs(error) < deadband) {
    pid_integral = 0;
    pid_prev_err = error;
    filtered_deriv = 0;
    return 0;
  }

  float scale = TARGET_DIST / 15.0;

  float KP = KP_BASE * scale;
  float KI = KI_BASE * scale;
  float KD = KD_BASE * scale;

  pid_integral += error * dt;
  pid_integral = constrain(pid_integral, -20, 20);

  float derivative = (error - pid_prev_err) / dt;
  filtered_deriv = 0.7 * filtered_deriv + 0.3 * derivative;

  pid_prev_err = error;

  return (KP * error) + (KI * pid_integral) + (KD * filtered_deriv);
}

// ── Slew limiter ─────────────────────────────────────
int slewServo(int target_angle, float error) {
  float abs_err = abs(error);

  int max_step;
  if (abs_err > 5)      max_step = 15;
  else if (abs_err > 3) max_step = 8;
  else if (abs_err > 2) max_step = 5;
  else                  max_step = 3;

  int delta = target_angle - prev_servo_angle;
  delta = constrain(delta, -max_step, max_step);

  return prev_servo_angle + delta;
}

// ── Serial input (UI compatible) ─────────────────────
void checkSerial() {
  if (Serial.available()) {
    float newTarget = Serial.parseFloat();

    if (newTarget >= 5 && newTarget <= 22) {   // ✅ your range
      TARGET_DIST = newTarget;
      Serial.print("New Target: ");
      Serial.println(TARGET_DIST);
    }
  }
}

// ── Setup ────────────────────────────────────────────
void setup() {
  Serial.begin(9600);   // ✅ MUST match Python

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);

  delay(500);

  kf.x = TARGET_DIST;

  Serial.println("System Ready");
  prev_time = micros();
}

// ── Loop ─────────────────────────────────────────────
void loop() {
  checkSerial();

  unsigned long now = micros();
  float dt = (now - prev_time) / 1e6;
  prev_time = now;

  if (dt <= 0 || dt > 0.5) return;

  if (!readSensor()) {
    servo.write(SERVO_CENTER);
    prev_servo_angle = SERVO_CENTER;
    pid_integral = 0;
    pid_prev_err = 0;
    filtered_deriv = 0;
    return;
  }

  float pid_out = computePID(dt);

  int raw_angle = SERVO_CENTER + (int)pid_out;
  raw_angle = constrain(raw_angle, SERVO_MIN_DEG, SERVO_MAX_DEG);

  int servo_angle = slewServo(raw_angle, ball_position - TARGET_DIST);
  servo_angle = constrain(servo_angle, SERVO_MIN_DEG, SERVO_MAX_DEG);

  servo.write(servo_angle);
  prev_servo_angle = servo_angle;

  // Optional debug
  Serial.print("pos:");
  Serial.print(ball_position);
  Serial.print(",target:");
  Serial.print(TARGET_DIST);
  Serial.print(",servo:");
  Serial.println(servo_angle);

  delay(20);
}