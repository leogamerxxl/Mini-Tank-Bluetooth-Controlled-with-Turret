/*
 * ============================================================
 *  MINI TANK — BLUETOOTH CONTROLLER
 *  Hardware: Arduino Uno + HC-05 + L298N + 2x DC Motor
 *  Power:    7.2V NiMH (6x AA cells in series)
 *  Control:  "Bluetooth RC Car" Android app (ASCII commands)
 *  Author:   Cosmin Leonardo Cozaciuc
 *  Version:  1.0 — reconstructed & documented 2025
 * ============================================================
 *
 *  ASCII COMMAND MAP (Bluetooth RC Car app):
 *   'F' → Forward
 *   'B' → Backward
 *   'L' → Turn left  (left track reverse, right track forward)
 *   'R' → Turn right (right track reverse, left track forward)
 *   'G' → Forward-left  (forward + slight left)
 *   'I' → Forward-right (forward + slight right)
 *   'H' → Backward-left
 *   'J' → Backward-right
 *   'S' → Stop
 *   '0'–'9' → Speed control (0=min, 9=max)
 *
 *  WIRING SUMMARY:
 *   HC-05 TX  → Arduino D10 (SoftwareSerial RX)
 *   HC-05 RX  → Arduino D11 via voltage divider (SoftwareSerial TX)
 *   HC-05 VCC → Arduino 5V
 *   HC-05 GND → GND
 *
 *   L293D IN1       → D2  (Left track direction A)
 *   L293D IN2       → D3  (Left track direction B)
 *   L293D IN3       → D4  (Right track direction A)
 *   L293D IN4       → D5  (Right track direction B)
 *   L293D EN1 (PWM) → D6  (Left track speed)
 *   L293D EN2 (PWM) → D9  (Right track speed)
 *   L293D VS        → 7.2V NiMH battery + (motor supply)
 *   L293D VSS       → Arduino 5V (logic supply)
 *   GND             → Common ground
 *
 *  NOTE: HC-05 RX pin requires a voltage divider (1kΩ + 2kΩ)
 *        to drop Arduino's 5V TX down to 3.3V for HC-05 logic.
 * ============================================================
 */

#include <SoftwareSerial.h>

// ── BLUETOOTH SERIAL ────────────────────────────────────────
// Using SoftwareSerial to keep D0/D1 free for USB debug
SoftwareSerial BT(10, 11);  // RX=D10, TX=D11

// ── MOTOR PINS ───────────────────────────────────────────────
// Left track
const int L_IN1 = 2;
const int L_IN2 = 3;
const int L_PWM = 6;   // ENA — must be PWM pin

// Right track
const int R_IN3 = 4;
const int R_IN4 = 5;
const int R_PWM = 9;   // ENB — must be PWM pin

// ── SPEED SETTINGS ───────────────────────────────────────────
/*
 *  L293D has separate VS (motor 7.2V) and VSS (logic 5V) pins.
 *  Motor voltage = battery minus ~1.4V L293D drop = ~5.8V.
 *  BASE_SPEED set conservatively for indoor use.
 *  TURN_SPEED reduced to prevent spinning out on hard surfaces.
 */
int baseSpeed = 200;        // normal forward/backward speed (0–255)
const int TURN_SPEED = 180; // speed during turns
const int DIAG_SPEED = 160; // slower track during diagonal moves

// ── STATE ────────────────────────────────────────────────────
char lastCmd = 'S';

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);   // USB debug
  BT.begin(9600);       // HC-05 default baud rate

  // Motor pins — all outputs
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_IN3, OUTPUT);
  pinMode(R_IN4, OUTPUT);
  pinMode(R_PWM, OUTPUT);

  stopMotors();
  Serial.println("=== TANK BT BOOT ===");
  Serial.println("Waiting for Bluetooth connection...");
}

// ════════════════════════════════════════════════════════════
void loop() {
  if (BT.available()) {
    char cmd = (char)BT.read();

    // Speed control — '0' to '9'
    if (cmd >= '0' && cmd <= '9') {
      // Map '0'–'9' to PWM range 70–255
      baseSpeed = map(cmd - '0', 0, 9, 70, 255);
      Serial.print("Speed set to: ");
      Serial.println(baseSpeed);
      return;
    }

    // Only act if command changed (reduces motor chatter)
    if (cmd == lastCmd) return;
    lastCmd = cmd;

    Serial.print("CMD: ");
    Serial.println(cmd);

    switch (cmd) {
      case 'F': moveForward();       break;
      case 'B': moveBackward();      break;
      case 'L': turnLeft();          break;
      case 'R': turnRight();         break;
      case 'G': forwardLeft();       break;
      case 'I': forwardRight();      break;
      case 'H': backwardLeft();      break;
      case 'J': backwardRight();     break;
      case 'S': stopMotors();        break;
      default:  break;
    }
  }
}

// ════════════════════════════════════════════════════════════
//  MOVEMENT FUNCTIONS
// ════════════════════════════════════════════════════════════

void moveForward() {
  // Both tracks forward at base speed
  setLeft(baseSpeed,  true);
  setRight(baseSpeed, true);
}

void moveBackward() {
  // Both tracks backward at base speed
  setLeft(baseSpeed,  false);
  setRight(baseSpeed, false);
}

void turnLeft() {
  // Left track backward, right track forward — pivot turn
  setLeft(TURN_SPEED,  false);
  setRight(TURN_SPEED, true);
}

void turnRight() {
  // Right track backward, left track forward — pivot turn
  setLeft(TURN_SPEED,  true);
  setRight(TURN_SPEED, false);
}

void forwardLeft() {
  // Forward, left track slower — gentle curve left
  setLeft(DIAG_SPEED, true);
  setRight(baseSpeed, true);
}

void forwardRight() {
  // Forward, right track slower — gentle curve right
  setLeft(baseSpeed,  true);
  setRight(DIAG_SPEED, true);
}

void backwardLeft() {
  setLeft(DIAG_SPEED, false);
  setRight(baseSpeed, false);
}

void backwardRight() {
  setLeft(baseSpeed,  false);
  setRight(DIAG_SPEED, false);
}

void stopMotors() {
  // Brake mode — both IN pins LOW
  digitalWrite(L_IN1, LOW);
  digitalWrite(L_IN2, LOW);
  analogWrite(L_PWM,  0);
  digitalWrite(R_IN3, LOW);
  digitalWrite(R_IN4, LOW);
  analogWrite(R_PWM,  0);
}

// ════════════════════════════════════════════════════════════
//  LOW-LEVEL MOTOR HELPERS
// ════════════════════════════════════════════════════════════

/*
 *  setLeft(speed, forward)
 *  Sets left track direction and speed via L293D IN1/IN2 + EN1.
 */
void setLeft(int speed, bool forward) {
  digitalWrite(L_IN1, forward ? HIGH : LOW);
  digitalWrite(L_IN2, forward ? LOW  : HIGH);
  analogWrite(L_PWM, speed);
}

/*
 *  setRight(speed, forward)
 *  Sets right track direction and speed via L293D IN3/IN4 + EN2.
 */
void setRight(int speed, bool forward) {
  digitalWrite(R_IN3, forward ? HIGH : LOW);
  digitalWrite(R_IN4, forward ? LOW  : HIGH);
  analogWrite(R_PWM, speed);
}
