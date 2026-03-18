Mini Tank — Bluetooth Controlled with Turret
Arduino Uno based tracked robot controlled via Bluetooth from an Android phone.
Features differential PWM steering, 4 DC motors and a 2-axis servo turret (pan 360° + tilt 90°).
Chassis 3D printed from open source model (Thingiverse #3004073).

Hardware
ComponentDetailsMicrocontrollerArduino Uno (ATmega328P, 16MHz)Bluetooth moduleHC-05 (UART, 9600 baud)Motor driverL293D dual H-bridge (4 channels)Motors4× DC motor with gearboxServo panContinuous rotation 360° — turret panServo tiltStandard 90° — turret tiltPower7.2V NiMH — 6× AA cells in seriesChassis3D printed PLA — Thingiverse #3004073TracksTPU flexible track links

Wiring
Motors — L293D
L293D IN1       →  D2   (left tracks direction A)
L293D IN2       →  D3   (left tracks direction B)
L293D EN1 (PWM) →  D6   (left tracks speed)
L293D IN3       →  D4   (right tracks direction A)
L293D IN4       →  D5   (right tracks direction B)
L293D EN2 (PWM) →  D9   (right tracks speed)
L293D VS        →  7.2V battery + (motor supply)
L293D VSS       →  Arduino 5V    (logic supply)

Motors are wired in parallel pairs: front-left + rear-left share Channel 1,
front-right + rear-right share Channel 2.

Servos
Servo pan  (360°) →  D7
Servo tilt  (90°) →  D8
Bluetooth — HC-05
HC-05 TX   →  D10 (SoftwareSerial RX)
HC-05 RX   →  D11 via 1kΩ/2kΩ voltage divider (5V → 3.3V)
HC-05 VCC  →  Arduino 5V
HC-05 GND  →  GND

HC-05 RX operates at 3.3V — voltage divider on Arduino D11 is required.


How It Works
Motor layout
Four DC motors are grouped into two independent channels on the L293D:

Channel 1 — front-left + rear-left motors (parallel)
Channel 2 — front-right + rear-right motors (parallel)

Each channel is controlled with direction (IN pins) and speed (EN PWM pin) independently.
Differential PWM steering
Instead of simple on/off turning, each maneuver uses different PWM values per side:
CommandLeft tracksRight tracksForwardbaseSpeed forwardbaseSpeed forwardBackwardbaseSpeed backwardbaseSpeed backwardTurn left80 backward200 forwardTurn right200 forward80 backwardCurve left130 forwardbaseSpeed forwardCurve rightbaseSpeed forward130 forward
This produces smooth arcs instead of jerky pivot turns.
Turret control
App buttonASCIIActionHeadlights (hold)WPan turret — continuous rotation CWHeadlights (release)wPan stopHornUTilt up (+20°)Rear lightsVTilt down (−20°)
Pan servo is a continuous rotation type — write(90) = stop, write(0) = rotate.
Tilt servo is a standard 90° type — position is incremented/decremented per button press and held.
ASCII command map (full)
CharActionFForwardBBackwardLPivot leftRPivot rightGCurve forward-leftICurve forward-rightHCurve backward-leftJCurve backward-rightSStop0–9Speed (PWM 70–255)W / wPan CW / stopUTilt upV / uTilt down

Files
tank-bt/
├── firmware/
│   └── tank_bt.ino          — main Arduino sketch
├── schematic/
│   └── schema_tank_bt.svg   — electrical schematic
└── README.md

Build Notes

Pair HC-05 before powering Arduino — default PIN 1234
TURN_INNER = 80 and TURN_OUTER = 200 — tune these for your surface
Pan servo write(0) = CW, write(180) = CCW — swap if turret rotates wrong way
L293D VS and VSS are separate pins — VS must be connected to battery, VSS to 5V
SoftwareSerial on D10/D11 keeps hardware Serial free for USB debug


Author
Cosmin Leonardo Cozaciuc
Electrical Engineering Student — UPB, Bucharest
leonardoczaciuc@gmail.com
